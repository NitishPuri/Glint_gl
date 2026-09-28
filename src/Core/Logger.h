#pragma once

// supress warning due to localtime being unsafe
#pragma warning(disable : 4996)

#include <chrono>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

//
#include <glm/glm.hpp>

#include "imgui/imgui.h"

#define CONSOLE_LOG 1

// Formatter specialization for ImVec2
template <>
struct std::formatter<ImVec2> : std::formatter<std::string> {
  auto format(const ImVec2& v, format_context& ctx) const {
    return std::formatter<std::string>::format(std::format("({}, {})", v.x, v.y), ctx);
  }
};
class Logger {
 public:
  static void init(const std::string& log_file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
      error("Logger already initialized");
      return;
    }
    log_file_.open(log_file_path, std::ios_base::app);  // Open in append mode
    if (!log_file_.is_open()) {
      std::cerr << "Unable to open log file: " << log_file_path << std::endl;
    }
  }

  static void shutdown() {
    // Close the log file
    if (log_file_.is_open()) {
      log_file_.close();
    }
  }

  template <typename... Args>
  static void error(Args... args) {
    log_message("ERROR", std::cerr, args...);
  }

  template <typename... Args>
  static void log(Args... args) {
    log_message("LOG", std::clog, args...);
  }

  template <typename... Args>
  static void logf(std::string_view format_str, Args... args) {
    std::string formatted_message = std::vformat(format_str, std::make_format_args(args...));
    log_message("LOG", std::clog, formatted_message);
  }

  template <typename... Args>
  static void errorf(std::string_view format_str, Args... args) {
    std::string formatted_message = std::vformat(format_str, std::make_format_args(args...));
    log_message("ERROR", std::cerr, formatted_message);
  }

 private:
  //   static std::string log_file_path_;
  static std::ofstream log_file_;
  static std::mutex mutex_;

  static std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S") << '-' << std::setfill('0') << std::setw(3)
       << ms.count();
    return ss.str();
  }

  template <typename... Args>
  static void log_message(const std::string& level, std::ostream& console_stream, Args... args) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << get_current_timestamp() << " [" << level << "]: ";
    (oss << ... << args);  // Fold expression to handle variadic arguments

    std::string timestamped_message = oss.str();
#if CONSOLE_LOG
    // Log to console
    console_stream << timestamped_message << std::endl;
#endif

    // Log to file
    if (log_file_.is_open()) {
      log_file_ << timestamped_message << std::endl;
    }
  }
};

// std::ofstream Logger::log_file_;
// std::mutex Logger::mutex_;

// Overload the << operator for glm::vec3
inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
  os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
  return os;
}

// Overload the << operator for glm::vec4
inline std::ostream& operator<<(std::ostream& os, const glm::vec4& vec) {
  os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
  return os;
}

// Overload the << operator for glm::mat4
inline std::ostream& operator<<(std::ostream& os, const glm::mat4& mat) {
  os << "\n";
  for (int i = 0; i < 4; ++i) {
    os << "| ";
    for (int j = 0; j < 4; ++j) {
      os << mat[i][j] << " ";
    }
    os << "|\n";
  }
  return os;
}
