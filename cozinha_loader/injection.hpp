#pragma once

class c_injector
{
private:
	bool map( std::string proc, std::wstring mod_name, std::vector<std::uint8_t> buf );

	void close_processes( std::vector<std::string> processes );

	std::vector<std::pair<int, std::string>> app_ids =
	{
		{ 730, "csgo.exe" } // Counter-Strike: Global Offensive
	};

public:
	c_injector() = default;
	~c_injector() = default;

	bool init( std::string process_name, std::string cheat_filename );
};

inline auto g_injector = std::make_unique<c_injector>();