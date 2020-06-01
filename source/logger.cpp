#include <cstdarg>
#include "logger.hpp"

namespace arm {

    LogLevel Logger::getLogLevel() {
        return Logger::s_logLevel;
    }

    void Logger::setLogLevel(LogLevel logLevel) {
        Logger::s_logLevel = logLevel;
    }

    void log(LogLevel logLevel, const char* prefix, const char* color, const char* format, va_list ap)
    {
        if (Logger::getLogLevel() > logLevel)
            return;

        printf("\033%s[%s]\033[0m ", color, prefix);
        vprintf(format, ap);
        printf("\n");
    }

    void Logger::debug(const std::string fmt, ...) {
        va_list args;
        va_start(args, fmt);

        log(LogLevel::Debug, "DEBUG", "[0;32m", fmt.c_str(), args);

        va_end(args);
    }

    void Logger::info(const std::string fmt, ...) {
        va_list args;
        va_start(args, fmt);

        log(LogLevel::Info, "INFO ", "[0;34m", fmt.c_str(), args);

        va_end(args);
    }

    void Logger::warn(const std::string fmt, ...) {
        va_list args;
        va_start(args, fmt);

        log(LogLevel::Warn, "WARN ", "[0;33m", fmt.c_str(), args);

        va_end(args);
    }

    void Logger::error(const std::string fmt, ...) {
        va_list args;
        va_start(args, fmt);

        log(LogLevel::Error, "ERROR", "[0;31m", fmt.c_str(), args);

        va_end(args);
    }

    [[noreturn]] void Logger::fatal(const std::string fmt, ...) {
        va_list args;
        va_start(args, fmt);

        log(LogLevel::Fatal, "FATAL", "[1;31m", fmt.c_str(), args);

        va_end(args);

        std::abort();
    }

}