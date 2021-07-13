#pragma once

namespace string
{
	// Format string
	template<typename ... args>
	std::string format( const std::string& format, args ... arg )
	{
		const size_t size = std::snprintf( nullptr, 0, format.c_str(), arg ... ) + 1;
		std::unique_ptr<char[]> buf( new char[size] );
		std::snprintf( buf.get(), size, format.c_str(), arg ... );
		return std::string( buf.get(), buf.get() + size - 1 );
	}

	// Transforms string to lowercase
	std::string to_lower( std::string str );

	// Converts wstring to string
	std::string to_utf8( std::wstring_view wstr );

	// Converts string to wstring
	std::wstring to_unicode( std::string_view str );
}

namespace util
{
	// Reads file and writes to an unsigned char array
	bool read_file_to_memory( const std::filesystem::path& path, std::vector<std::uint8_t>* out_buffer );

	// Reads an unsigned char array and writes into a binary file
	bool write_file_from_memory( std::string_view name, std::vector<std::uint8_t> buffer );

	// Returns the steam path from regedit
	std::wstring get_steam_path();
}