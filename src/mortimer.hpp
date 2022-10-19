#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <chrono>

using indentation = int;
constexpr int SLEEP_DURATION_MS = 8;
constexpr int PROGRESS_BAR_SIZE = 100;

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
      int actual_seconds;
      int recommended_seconds;
      float weight;
      bool plus;
      bool minus;
    };

    void handleOutput() const;
    void handleKeys();
    void handleTime();

    void pause();
    void resume();
    void previous();
    void next();
    void end();

    float progress() const;

    std::string _title;
    std::vector<Section> _sections;
    std::vector<Section>::iterator _current_section;

    std::chrono::time_point<std::chrono::steady_clock> _section_start;
    std::chrono::time_point<std::chrono::steady_clock> _pause_start;
    std::chrono::duration<int, std::milli>  _pause_duration;
    bool _paused;
    bool _running;

  public:
    friend std::ostream& operator<<(std::ostream&, const Session&);
    friend std::ostream& operator<<(std::ostream&, const Section&);
};

void rtrim(std::string& s);
void printUsage();
std::string handleInput(int argc, char* argv[]);
