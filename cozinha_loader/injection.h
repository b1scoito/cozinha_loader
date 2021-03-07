#pragma once

class injection
{
public:
	injection( ) = default;
	~injection( ) = default;

	bool setup( );
	void close_processes( std::vector<std::string> processes );

private:
	bool inject( std::string process, std::wstring module_name, std::filesystem::path path_to_dll );
};

inline auto g_injection = std::make_unique<injection>( );