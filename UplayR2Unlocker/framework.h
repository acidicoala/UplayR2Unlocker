#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <fstream>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // Windows Header Files

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <nlohmann/json.hpp>

