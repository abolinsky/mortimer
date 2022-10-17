#include "mortimer.hpp"

#include <fstream>
#include <regex>
#include <cstdlib>
#include <stack>
#include <tuple>
#include <chrono>
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

Session::Session(const std::string& filename) : _running(false), _paused(false) {
  std::ifstream in(filename);

  if (!in) {
    std::cerr << "There is no file at " << filename << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::regex pattern(R"((^\s*)?(\w+\.)?\s*([^(]*)\s*\(?(\d*h)?(\d*m)?(\d*s)?(\+?)(\-?)\)?$)");
  std::smatch matches;
  
  std::string line;
  std::getline(in, line);
  _content.title = line;

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
      _content.sections.push_back(section);
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
      _content.sections.push_back(section);
    }
  }
}

void Session::run() {
  _section_start = std::chrono::steady_clock::now();
  _running = true;
  _paused = false;
  std::cout << _content.title << std::endl;
  while (_running) {
    handleKeys();
    handleTime();
    handleOutput();
  }
}

void Session::handleKeys() {
}

void Session::handleTime() {
  if (_paused) {
    return;
  }

  if (_content.sections.empty()) {
    _running = false;
    return;
  }

  const auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _section_start).count();
  if (time >= _content.sections.front().seconds) {
    _finished_sections.push_back(_content.sections.front());
    _content.sections.pop_front();
    _section_start = std::chrono::steady_clock::now();
  }
}

void Session::handleOutput() {
  if (!_running) {
    std::cout << "end" << std::endl;  
    return;
  }
  std::string out = _content.sections.front().qualifiedName();
  std::cout << out << std::flush;
  std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
  for (auto i = 0; i < out.length(); ++i) {
    std::cout << " \b\b" << std::flush;
  }
}

bool Session::running() const {
  return _running;
}

Session::Section::Section() : seconds(0), plus(true), minus(true) {}

std::string Session::Section::qualifiedName() const {
  return prefix.empty() ? title : prefix + " >> " + title;
}

std::ostream& operator<<(std::ostream& os, const Session& session) {
  os << session._content.title << std::endl;
  for (auto& section : session._content.sections) {
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
