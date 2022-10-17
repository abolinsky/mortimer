#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <cstdlib>
#include <stack>
#include <tuple>

using indentation = int;

void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

struct Session {
  struct Section {
    Section() : seconds(0), plus(true), minus(true) {}

    std::string qualifiedName() const {
      return prefix.empty() ? title : prefix + " >> " + title;
    }

    std::string prefix;
    std::string title;
    int seconds;
    bool plus;
    bool minus;
  };

  struct Content {
    std::string title;
    std::vector<Section> sections;
  };

  Session(const std::string& filename) : total_elapsed_time(0), section_elapsed_time(0), paused(false) {
    std::ifstream in(filename);

    if (!in) {
      std::cerr << "There is no file at " << filename << std::endl;
      std::exit(EXIT_FAILURE);
    }

    std::regex pattern(R"((^\s*)(\d+\.)\s*([^(]*)\s*\(?(\d*h)?(\d*m)?(\d*s)?(\+?)(\-?)\)?$)");
    std::smatch matches;
    
    std::string line;
    std::getline(in, line);
    content.title = line;

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
        content.sections.push_back(section);
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
        content.sections.push_back(section);
      }
    }
  }

  void tick() {
  }

  Content content;
  std::vector<Section> finished_sections;
  int total_elapsed_time;
  int section_elapsed_time;
  bool paused;
};

std::ostream& operator<<(std::ostream& os, const Session::Section& section) {
  if (section.prefix.empty()) {
    os << section.title << " ";
  } else {
    os << section.qualifiedName() << " ";
  }

  os << "(" << section.seconds << "s)" << std::endl;

  return os;
}

std::ostream& operator<<(std::ostream& os, const Session& session) {
  os << session.content.title << std::endl;
  for (auto& section : session.content.sections) {
    os << section; 
  }
  return os;
}

void printUsage() {
  std::cerr << "You forgot something!" << std::endl;
  std::cerr << "usage: mortimer <file>" << std::endl;
}

std::string handleInput(int argc, char* argv[]) {
  if (argc != 2) {
    printUsage();
    std::exit(EXIT_FAILURE);
  }

  return std::string(argv[1]);
}

int main(int argc, char* argv[]) {
  const auto filename = handleInput(argc, argv);
  Session session(filename);

  std::cout << session << std::endl;
  
  for (;;) {
    /* Handle timer */
    session.tick();
    /* Handle key inputs */
    /* Handle terminal output */
    break;
  }
}
