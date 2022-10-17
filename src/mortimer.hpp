#pragma once

#include <iostream>
#include <vector>
#include <string>

using indentation = int;
constexpr int SLEEP_DURATION_MS = 8;

class Session {
  public:
    Session(const std::string& filename);

    void run();
    bool running() const;

  private:
    struct Section {
      Section();

      std::string qualifiedName() const;

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

    void handleTime();
    void handleKeys();
    void handleOutput();

    Content _content;
    std::vector<Section> _finished_sections;
    int _total_elapsed_time;
    int _section_elapsed_time;
    bool _paused;
    bool _running;

  public:
    friend std::ostream& operator<<(std::ostream&, const Session&);
    friend std::ostream& operator<<(std::ostream&, const Section&);
};

void rtrim(std::string& s);
void printUsage();
std::string handleInput(int argc, char* argv[]);
