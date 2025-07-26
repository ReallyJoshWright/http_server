#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>
#include <format>
#include <print>
#include <ctime>

#include "logger.h"

namespace chrono = std::chrono;

using std::println;
using std::string;
using std::format;

///////////////////////////////////////////////////////////////////////////////
// constructors
///////////////////////////////////////////////////////////////////////////////
Logger::Logger() {
    this->to_console = false;
}

///////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////
void Logger::info(string message) {
    handleMessage(message, "INF");
}

void Logger::error(string message) {
    handleMessage(message, "ERR");
}

void Logger::setLogger(bool to_console) {
    this->to_console = to_console;
}

///////////////////////////////////////////////////////////////////////////////
// private methods
///////////////////////////////////////////////////////////////////////////////
void Logger::handleMessage(string message, string type) {
    string now = getCurrentDateTime();

    string msg = format("[{} {}] {}", now, type, message);

    if (to_console) {
        println("{}", msg);
    }
}

string Logger::getCurrentDateTime() {
    string now_str;

    auto now_time_point = chrono::system_clock::now();
    std::time_t now_c_time = chrono::system_clock::to_time_t(now_time_point);
    std::tm *local_tm = std::localtime(&now_c_time);

    if (local_tm) {
        std::stringstream ss;
        ss << std::put_time(local_tm, "%m/%d/%Y %H:%M:%S");
        now_str = ss.str();
    } else {
        now_str = "TIME_ERROR";
    }

    return now_str;
}
