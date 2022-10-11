#pragma once

class Logger {
    public:
        Logger() {};
        virtual ~Logger() {};
        virtual void log(const char* msg) = 0;
};