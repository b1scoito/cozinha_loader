#pragma once

// includes
#include <iostream>
#include <fstream>
#include <filesystem>

#include <windows.h>
#include <tlhelp32.h>

#include <BlackBone/Process/Process.h>

using namespace std::chrono_literals;

// header files
#include "logger.hpp"
#include "utils.hpp"
#include "memory.hpp"
#include "vac3_bypass_data.hpp"
#include "injection.hpp"