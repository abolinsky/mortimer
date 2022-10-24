#include "mortimer.hpp"

#include <curses.h>

#include <fstream>
#include <regex>
#include <cstdlib>
#include <stack>
#include <tuple>
#include <thread>

int main(int argc, char* argv[]) {
  Session session(handleInput(argc, argv));
  session.run(); 
}

std::string handleInput(int argc, char* argv[]) {
  if (argc != 2) {
    printUsage();
    std::exit(EXIT_FAILURE);
  }

  return std::string(argv[1]);
}

void printUsage() {
  std::cerr << "You forgot something!" << std::endl;
  std::cerr << "usage: mortimer <file>" << std::endl;
}

Session::Session(const std::string& filename) : _pause_duration(0), _paused(false), _running(false) {
  std::ifstream in(filename);

  if (!in) {
    std::cerr << "There is no file at " << filename << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::regex pattern(R"((^\s*)?(\w+\.)?\s*([^(]*)\s*\(?(\d*h)?(\d*m)?(\d*s)?(\+?)(\-?)\)?$)");
  std::smatch matches;
  
  std::string line;
  std::getline(in, line);
  _title = line;

  std::stack<std::tuple<Section, indentation>> section_stack;

  while (std::getline(in, line)) {
    if (!std::regex_match(line, matches, pattern)) {
      std::cerr << '"' << line << '"' << " does not follow the expected structure" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    Section section;

    section.title = matches[3];
    rtrim(section.title);

    const std::string h(matches[4]);
    const std::string m(matches[5]);
    const std::string s(matches[6]);

    try {
      section.seconds += h.size() ? std::stoi(h.substr(0, h.size() - 1)) * 360 : 0;
      section.seconds += m.size() ? std::stoi(m.substr(0, m.size() - 1)) * 60 : 0;
      section.seconds += s.size() ? std::stoi(s.substr(0, s.size() - 1)) : 0;
    } catch (const std::invalid_argument e) {
      std::cerr << "At least one of your times doesn't look right" << std::endl;
      std::exit(EXIT_FAILURE);
    }

    section.plus = matches[7].length();
    section.minus = matches[8].length();

    const int indentation = matches[1].length();

    if (section_stack.empty()) {
      section_stack.emplace(section, indentation);
      _sections.push_back(section);
      continue;
    }

    auto [top_section, top_indentation] = section_stack.top();
    if (indentation == top_indentation) {
      section.prefix = top_section.prefix;
      section_stack.pop();
      section_stack.emplace(section, indentation);
    } else if (indentation > top_indentation) {
      section.prefix = top_section.qualifiedName();
      section_stack.emplace(section, indentation);
    } else {
      while (indentation < top_indentation) {
        section_stack.pop();
        std::tie(top_section, top_indentation) = section_stack.top();
      }
      section.prefix = top_section.prefix;
      section_stack.emplace(section, indentation);
    }

    if (section.seconds) {
      _sections.push_back(section);
    }
  }

  _current_section = _sections.begin();
}

void Session::run() {
  /* Start curses mode */
  initscr();
  start_color();
  raw();
  noecho();
  keypad(stdscr, TRUE);
  init_pair(1, COLOR_GREEN, COLOR_BLACK);

  _running = true;

  printw(_title.c_str());
  while (_running) {
    handleKeys();
    handleTime();
    handleOutput();
  }

  /* End curses mode */
	endwin();
}

void Session::handleKeys() {
  /* Wait for user input */
	int ch = getch();

  switch (ch) {
    case 100: {
      next();
      break;
    }
    case 97: {
      previous();
      break;
    }
    case 113: {
      end();
      break;
    }
    case 119: {
      pause();
    }
    case 101: {
      resume();
    }
    default: {}
  }
}

void Session::handleTime() {
  std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));

  if (_current_section == _sections.end()) {
    _running = false;
    return;
  }

  if (_paused) {
    return;
  }

  if (_section_start.time_since_epoch().count() == 0) {
    _section_start = std::chrono::steady_clock::now();
  }

  if (progress() >= 1.0) {
    next();
  }

  if (_current_section == _sections.end()) {
    _running = false;
    return;
  }
}

void Session::handleOutput() {
  if (!_running) {
    return;
  }

  getmaxyx(stdscr, _row, _col);

  clear();
  move(0, 0);

  const std::string& current_title = _current_section->qualifiedName();
  printw(_title.c_str());
  printw("\n");
  printw(current_title.c_str());
  printw("\n");

  static int previous_progress;
  const int PROGRESS_BAR_SIZE = _row - 17;
  const int current_progress = progress() * PROGRESS_BAR_SIZE;
  const int spaces = PROGRESS_BAR_SIZE - current_progress;
  const int remaining_seconds = (1.0f - progress()) * _current_section->seconds;

  printw("[");
  attron(COLOR_PAIR(1));
  if (current_progress != previous_progress) {
    for (auto i = 0; i < current_progress; ++i) {
      if (i < PROGRESS_BAR_SIZE * 0.75) {
        //printw("\x1B[32m|\033[0m");
      } else if (i < PROGRESS_BAR_SIZE * 0.90) {
        //printw("\x1B[33m|\033[0m");
      } else {
        //printw("\x1B[31m|\033[0m");
      }
      printw("|");
    }
    for (auto i = 0; i < spaces - 1; ++i) {
      printw(" ");
    }
  } else {
    for (auto i = 0; i < PROGRESS_BAR_SIZE - 1; ++i) {
      printw(" ");
    }
  }
  attroff(COLOR_PAIR(1));
  printw("] ");

  printw("%im%is ", remaining_seconds / 60, remaining_seconds % 60);

  /* Print it on to the real screen */
	refresh();
}

void Session::pause() {
  if (!_paused) {
    _pause_start = std::chrono::steady_clock::now();
    _paused = true;
  }
}

void Session::resume() {
  if (!_paused) {
    _pause_duration += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _pause_start);
    _paused = false;
  }
}

void Session::previous() {
  if (_current_section > _sections.begin()) {
    --_current_section;
  }
  _section_start = std::chrono::steady_clock::now();
  resume();
  _pause_duration = _pause_duration.zero();
}

void Session::next() {
  if (_current_section < _sections.end()) {
    ++_current_section;
  }

  _section_start = std::chrono::steady_clock::now();
  resume();
  _pause_duration = _pause_duration.zero();
}

void Session::end() {
  _running = false;
}

float Session::progress() const {
  const auto milliseconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _section_start - _pause_duration).count();
  return milliseconds_elapsed / static_cast<float>(_current_section->seconds * 1000);
}

Session::Section::Section() : seconds(0), plus(true), minus(true) {}

std::string Session::Section::qualifiedName() const {
  return prefix.empty() ? title : prefix + " >> " + title;
}

std::ostream& operator<<(std::ostream& os, const Session& session) {
  os << session._title << std::endl;
  for (auto& section : session._sections) {
    os << section; 
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Session::Section& section) {
  if (section.prefix.empty()) {
    os << section.title << " ";
  } else {
    os << section.qualifiedName() << " ";
  }

  os << "(" << section.seconds << "s)" << std::endl;

  return os;
}

void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}
