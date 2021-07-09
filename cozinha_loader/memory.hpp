#pragma once

namespace memory
{
	bool open_process( std::wstring_view path, const std::vector<std::wstring> arguments, PROCESS_INFORMATION& pi );
	bool is_process_open( const std::vector<std::pair<std::uint32_t, std::string>>& vec_processes, std::string_view str_proc );
	bool kill_process( const std::vector<std::pair<std::uint32_t, std::string>>& vec_processes, std::string_view str_proc );

	std::uint32_t get_process_id_by_name( const std::vector<std::pair<std::uint32_t, std::string>>& vec_processes, std::string_view str_proc );

	std::vector<std::pair<std::uint32_t, std::string>> get_process_list();
}