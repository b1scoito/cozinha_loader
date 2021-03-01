#pragma once

class injection
{
public:
	injection( ) = default;
	~injection( ) = default;

	bool setup( );
	void close_processes( std::vector<std::string> processes );

private:
	std::vector<std::uint8_t> vac_buffer {};
	std::vector<std::uint8_t> cheat_buffer {};

	bool inject( std::string process, bool csgo, std::vector<std::uint8_t> buffer );
	void wait_for_process( std::string process );
};

inline auto g_injection = std::make_unique<injection>( );