#pragma once
#include"co_export.h"
#include <string>

namespace common
{

enum class LogTarget
{
    Console,
    File,
    Both
};

COMMON_STRUCT void logError(const std::string& comment, LogTarget target = LogTarget::Both);
COMMON_STRUCT void logWarning(const std::string& comment, LogTarget target = LogTarget::Both);
COMMON_STRUCT void logInfo(const std::string& comment, LogTarget target = LogTarget::Both);
}