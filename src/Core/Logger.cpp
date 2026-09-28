#include "Logger.h"

#include "imgui/imgui.h"

std::ofstream Logger::log_file_;
std::mutex Logger::mutex_;
