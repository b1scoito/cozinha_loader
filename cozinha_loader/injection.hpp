#pragma once

class injector
{
private:
	bool inject( std::string process, std::wstring module_name, std::filesystem::path path_to_dll );
	void close_processes( std::vector<std::string> processes );
public:
	injector( ) = default;
	~injector( ) = default;
	bool run( );
};

inline auto g_inj = std::make_unique<injector>( );