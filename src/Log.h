#pragma once


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

class Log
{
public:
    static void Init();
    static auto& getLogger() {return s_Logger;}
private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

#define LOG_INFO(...)       Log::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)       Log::getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)      Log::getLogger()->error(__VA_ARGS__)

#define Z_ENABLE_ASSERTIONS

#ifdef Z_ENABLE_ASSERTIONS
#define ASSERT(x) \
    if (!(x)) {LOG_ERROR("Assertion failed: "  #x);}
#else
#define ASSERT(x)
#endif