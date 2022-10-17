#pragma once

#include <iostream>
#include <deque>
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
      std::deque<Section> sections;
    };

    void handleKeys();
    void handleTime();
    void handleOutput();

    Content _content;
    std::deque<Section> _finished_sections;
    std::chrono::time_point<std::chrono::steady_clock> _section_start;
    bool _paused;
    bool _running;

  public:
    friend std::ostream& operator<<(std::ostream&, const Session&);
    friend std::ostream& operator<<(std::ostream&, const Section&);
};

void rtrim(std::string& s);
void printUsage();
std::string handleInput(int argc, char* argv[]);