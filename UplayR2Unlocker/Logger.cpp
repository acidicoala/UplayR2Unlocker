#include "pch.h"
#include "Logger.h"
#include "Config.h"

constexpr auto LOG_PATH = "UplayR2Unlocker.log";

shared_ptr<spdlog::logger> logger = spdlog::null_logger_mt("null");

void Logger::init(HMODULE hModule)
{
	if(config->log_level != "off")
	{
		auto path = getDllDir(hModule) / LOG_PATH;
		logger = spdlog::basic_logger_mt("default", path.string(), true);
		logger->set_pattern("[%H:%M:%S.%e] [%l]\t%v");
		logger->set_level(spdlog::level::from_str(config->log_level));
		logger->flush_on(spdlog::level::debug);
	}
}
