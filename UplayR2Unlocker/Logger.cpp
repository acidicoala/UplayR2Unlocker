#include "pch.h"
#include "Logger.h"
#include "Config.h"

constexpr auto LOG_PATH = "UplayR2Unlocker.log";

shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt("default", LOG_PATH, true);

void Logger::init()
{
	logger->set_pattern("[%H:%M:%S.%e] [%l]\t%v");
	logger->set_level(spdlog::level::from_str(config.log_level));
	logger->flush_on(spdlog::level::debug);
}
