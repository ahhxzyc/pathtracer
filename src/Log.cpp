#include "Log.h"

void Log::Init()
{
    spdlog::set_pattern("%^[%T] %n: %v%$");
    s_Logger = spdlog::stdout_color_mt("Renderer");
    s_Logger->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger> Log::s_Logger;