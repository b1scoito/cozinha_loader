#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
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
#include "logging.hpp"
#include "util.hpp"
#include "memory.hpp"

#include "injection.h"