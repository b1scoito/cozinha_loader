#pragma once

#include "winreg/winreg.hpp"

namespace string
{
	// Format string
	template<typename ... arg>
	static std::wstring format( std::wstring_view fmt, arg ... args )
	{
		const int size = std::swprintf( nullptr, NULL, fmt.data(), args ... ) + 1;
		const auto buf = std::make_unique<wchar_t[]>( size );
		std::swprintf( buf.get(), size, fmt.data(), args ... );

		return std::wstring( buf.get(), ( buf.get() + size ) - 1 );
	}

	// Transforms wstring to lowercase
	std::wstring to_lower( std::wstring str );

	// Converts string to wstring
	std::wstring to_unicode( std::string str );
}

namespace util
{
	// Reads file and writes to an unsigned char array
	bool read_file_to_memory( const std::filesystem::path& path, std::vector<std::uint8_t>* out_buffer );

	// Reads an unsigned char array and writes into a binary file
	bool write_file_from_memory( std::wstring name, std::vector<std::uint8_t> buffer );

	// Returns the steam path from regedit
	std::wstring get_steam_path();

	// Returns the system directory
	std::wstring get_system_directory();

	// Returns the current executable path
	std::wstring get_executable_path();
}