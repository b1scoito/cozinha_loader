#pragma once

class injector
{
private:
	bool map( std::string process, std::wstring module_name, std::vector<std::uint8_t> binary_bytes );

	void close_processes( std::vector<std::string> processes );

public:
	injector() = default;
	~injector() = default;

	bool call( std::string process_name, std::string cheat_filename );
};

inline auto g_injector = injector();