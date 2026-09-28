#pragma once

#include "Logger.h"

class ScopedTimer {
 public:
  ScopedTimer(const std::string& name) : m_Name(name), m_StartTimePoint(std::chrono::high_resolution_clock::now()) {}

  ~ScopedTimer() {
    auto endTimePoint = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

    auto duration = end - start;
    double ms = duration * 0.001;

    Logger::log(m_Name, " took ", ms, " ms (", duration, " us)");
  }

 private:
  std::string m_Name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
};