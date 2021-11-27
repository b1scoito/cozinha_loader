#pragma once

#include <TlHelp32.h>

namespace memory
{
	// Open process with CreateProcess
	bool open_process( std::wstring_view path, const std::vector<std::wstring> arguments, PROCESS_INFORMATION& pi );

	// Checks if process is opened
	bool is_process_open( const std::vector<std::pair<std::uint32_t, std::wstring>>& vec_processes, std::wstring_view str_proc );

	// Kills target process
	bool kill_process( const std::vector<std::pair<std::uint32_t, std::wstring>>& vec_processes, std::wstring_view str_proc );

	// Returns the process id from process name
	std::uint32_t get_process_id_by_name( const std::vector<std::pair<std::uint32_t, std::wstring>>& vec_processes, std::wstring_view str_proc );

	// Gets the process list
	std::vector<std::pair<std::uint32_t, std::wstring>> get_process_list();
}