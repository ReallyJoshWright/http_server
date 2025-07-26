#pragma once

#include <string>

class Logger {
    public:
        Logger();
        void info(std::string message);
        void error(std::string message);
        void setLogger(bool to_console);

    private:
        bool to_console;

        void handleMessage(std::string message, std::string type);
        std::string getCurrentDateTime();
};
