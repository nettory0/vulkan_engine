#include "co_logging.h"
#include <memory>
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h" 


namespace common
{

namespace
{
std::shared_ptr<spdlog::logger> gConsoleLogger;
std::shared_ptr<spdlog::logger> gFileLogger;
}

void initLogging()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S] [%l] %v%$");

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/app.log", 1024 * 1024 * 5, 3);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

        gConsoleLogger = std::make_shared<spdlog::logger>("console", console_sink);
        gFileLogger = std::make_shared<spdlog::logger>("file", file_sink);

        gConsoleLogger->set_level(spdlog::level::debug);
        gFileLogger->set_level(spdlog::level::debug);
    });
}


namespace
{
void dispatch(LogTarget target, const std::function<void(spdlog::logger&)>& fn)
{
    initLogging();
    if(target == LogTarget::Console || target == LogTarget::Both)
        fn(*gConsoleLogger);
    if(target == LogTarget::File || target == LogTarget::Both)
        fn(*gFileLogger);
}
}

void logError(const std::string& comment, LogTarget target)
{
    dispatch(target, [&](spdlog::logger& l) { l.error("{}", comment); });
}

void logWarning(const std::string& comment, LogTarget target)
{
    dispatch(target, [&](spdlog::logger& l) { l.warn("{}", comment); });
}

void logInfo(const std::string& comment, LogTarget target)
{
    dispatch(target, [&](spdlog::logger& l) { l.info("{}", comment); });
}


}