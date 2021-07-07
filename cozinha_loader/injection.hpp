#pragma once

class c_injector
{
private:
	bool map( std::string proc, std::wstring mod_name, std::vector<std::uint8_t> buf, bool wait_for_mod = true );

	void close_processes( std::vector<std::string> processes );

	std::vector<std::pair<int, std::string>> app_ids =
	{
		{ 730, "csgo.exe" } // Counter-Strike: Global Offensive
	};

	std::uint8_t retries = 0;

public:
	c_injector() = default;
	~c_injector() = default;

	bool init( std::string proc_name, std::filesystem::path cheat_name );
};

inline auto g_injector = std::make_unique<c_injector>();