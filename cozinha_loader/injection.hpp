#pragma once

class c_injector
{
private:
	bool map( std::string_view str_proc, std::wstring_view wstr_mod_name, std::vector<std::uint8_t> vec_buffer );

	void close_processes( const std::vector<std::string_view> vec_processes );

	const std::vector<std::pair<int, std::string_view>> vec_app_ids = {
		{ 730, "csgo.exe" } // Counter-Strike: Global Offensive
	};

public:
	c_injector() = default;
	~c_injector() = default;

	bool init( std::string_view str_proc_name, const std::filesystem::path dll_path );
};

inline auto g_injector = std::make_unique<c_injector>();