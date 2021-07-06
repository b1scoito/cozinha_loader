#pragma once

namespace string
{
	std::string to_lower( std::string str );
	std::string to_upper( std::string str );

	std::string to_utf8( std::wstring wstr );
	std::wstring to_unicode( std::string str );
}

namespace other
{
	bool read_file_to_memory( const std::string& path, std::vector<std::uint8_t>* out_buffer );

	std::wstring get_steam_path();
}