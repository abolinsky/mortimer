#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <cstdlib>
#include <stack>

struct Session {
  struct Content {
    struct Section {
      std::string prefix = "";
      std::string title = "";
      int seconds = 0;
      bool plus = true;
      bool minus = true;
    };

    std::string title = "";
    std::vector<Section> sections;
  };

  Session(const std::string& filename) {
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

    std::stack<std::string> section_stack;
    while (std::getline(in, line)) {
      if (std::regex_match(line, matches, pattern)) {
        Content::Section section;

        section.title = matches[3];

        const std::string h(matches[4].str());
        const std::string m(matches[5].str());
        const std::string s(matches[6].str());

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

        content.sections.push_back(section);
      }
    }
  }

  void tick() {
  }

  Content content;
  std::vector<int> finished_sections;
  int total_elapsed_time = 0;
  int section_elapsed_time = 0;
  bool paused = false;
};

std::ostream& operator<<(std::ostream& os, const Session::Content::Section& section) {
  if (section.prefix.empty()) {
    os << section.title << " ";
  } else {
    os << section.prefix << "::" << section.title << " ";
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
