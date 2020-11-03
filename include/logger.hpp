#pragma once

namespace arm::log {

    enum class LogLevel {
        None  = 0,
        Fatal = 1,
        Error = 2,
        Warn  = 3,
        Info  = 4,
        Debug = 5
    };

    constexpr LogLevel logLevel = LogLevel::Info;

    namespace {

        template<typename ... Args>
        void print(const char *prefix, const char *format, Args... args) {
            printf("[%s] ", prefix);
            printf(format, args...);
            puts("\n");
        }

    }

    template<typename ... Args>
    void debug(const char *format, Args... args) {
        if constexpr (logLevel < LogLevel::Debug) return;

        print("DEBUG", format, args...);
    }

    template<typename ... Args>
    void info(const char *format, Args... args) {
        if constexpr (logLevel < LogLevel::Info) return;

        print("INFO ", format, args...);
    }

    template<typename ... Args>
    void warn(const char *format, Args... args) {
        if constexpr (logLevel < LogLevel::Warn) return;

        print("WARN ", format, args...);
    }

    template<typename ... Args>
    void error(const char *format, Args... args) {
        if constexpr (logLevel < LogLevel::Error) return;

        print("ERROR", format, args...);
    }

    template<typename ... Args>
    void fatal(const char *format, Args... args) {
        if constexpr (logLevel < LogLevel::Fatal) return;

        print("FATAL", format, args...);
    }

}