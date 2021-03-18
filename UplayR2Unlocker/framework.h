#pragma once
#include <codeanalysis\warnings.h>

#include <string>
#include <filesystem>
#include <vector>
#include <fstream>

#pragma warning(push) // Disable 3rd party library warnings
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // Windows Header Files

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

#include <nlohmann/json.hpp>

#pragma warning(pop)

#ifdef _WIN64
#include "LinkerExports64.h"
#else
#include "LinkerExports.h"
#endif

