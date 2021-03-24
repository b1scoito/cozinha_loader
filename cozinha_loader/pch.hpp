#pragma once

#define NOMINMAX // Use std::min and std::max instead of min() and max()

// Windows includes
#include <windows.h>
#include <iostream>
#include <cstdint>
#include <ostream>
#include <fstream>
#include <shared_mutex>
#include <tlhelp32.h>
#include <algorithm>
#include <vector>
#include <filesystem>

// Blackbone includes
#include <BlackBone/Process/Process.h>

// Header files
#include "logger.hpp"
#include "util.hpp"
#include "memory.hpp"
#include "data.hpp"
#include "injection.hpp"