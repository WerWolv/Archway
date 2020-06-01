#pragma once

#include <string>

namespace arm {

    enum class LogLevel {
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    class Logger {
    public:
        static void setLogLevel(LogLevel logLevel);
        static LogLevel getLogLevel();

        static void debug(const std::string fmt, ...);
        static void info(const std::string fmt, ...);
        static void warn(const std::string fmt, ...);
        static void error(const std::string fmt, ...);
        [[noreturn]] static void fatal(const std::string fmt, ...);
    private:
        static inline LogLevel s_logLevel = LogLevel::Debug;
    };

}