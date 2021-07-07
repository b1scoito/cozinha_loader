#pragma once

namespace string
{
	template<typename ... args>
	std::string format( const std::string& format, args ... arg )
	{
		const size_t size = std::snprintf( nullptr, 0, format.c_str(), arg ... ) + 1;
		std::unique_ptr<char[]> buf( new char[size] );
		std::snprintf( buf.get(), size, format.c_str(), arg ... );
		return std::string( buf.get(), buf.get() + size - 1 );
	}

	std::string to_lower( std::string str );
	std::string to_upper( std::string str );

	std::string to_utf8( std::wstring wstr );
	std::wstring to_unicode( std::string str );
}

namespace other
{
	bool read_file_to_memory( const std::filesystem::path& path, std::vector<std::uint8_t>* out_buffer );

	std::wstring get_steam_path();
}