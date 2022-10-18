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

    void handleOutput() const;
    void handleKeys();
    void handleTime();

    void pause();
    void resume();
    void previous();
    void next();
    void end();

    Content _content;
    std::deque<Section> _finished_sections;

    std::chrono::time_point<std::chrono::steady_clock> _section_start;
    std::chrono::time_point<std::chrono::steady_clock> _pause_start;
    int _pause_duration;
    bool _paused;
    bool _running;

  public:
    friend std::ostream& operator<<(std::ostream&, const Session&);
    friend std::ostream& operator<<(std::ostream&, const Section&);
};

void rtrim(std::string& s);
void printUsage();
std::string handleInput(int argc, char* argv[]);
