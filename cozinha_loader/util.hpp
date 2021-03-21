#pragma once

namespace utils
{
	inline std::string to_lower( std::string string )
	{
		std::transform( string.begin(), string.end(), string.begin(), static_cast<int(*)(int)>(::tolower) );
		return string;
	}

	inline std::string to_upper( std::string string )
	{
		std::transform( string.begin(), string.end(), string.begin(), static_cast<int(*)(int)>(::toupper) );
		return string;
	}

	inline bool read_file_to_memory( const std::string &file_path, std::vector<std::uint8_t> *out_buffer )
	{
		std::ifstream file_ifstream( file_path, std::ios::binary );
		if (!file_ifstream)
			return false;

		out_buffer->assign( (std::istreambuf_iterator<char>( file_ifstream )), std::istreambuf_iterator<char>() );
		file_ifstream.close();

		return true;
	}

	inline std::string wstring_to_string( std::wstring wstr )
	{
		if (wstr.empty())
			return std::string();

		const auto size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), nullptr, 0, nullptr, nullptr );
		auto ret = std::string( size, 0 );
		WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), &ret[0], size, nullptr, nullptr );

		return ret;
	}

	inline std::string get_steam_path()
	{
		HKEY h_key {};
		if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS)
		{
			_loge( "Failed to find steam registry." );
			RegCloseKey( h_key );
			return "";
		}

		char steam_path_reg[MAX_PATH] {}; steam_path_reg[0] = '"';
		DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

		if (RegQueryValueEx( h_key, "SteamExe", nullptr, nullptr, (LPBYTE) (steam_path_reg + 1), &steam_path_size ) != ERROR_SUCCESS)
		{
			_loge( "Failed to query SteamExe." );
			RegCloseKey( h_key );
			return "";
		}

		RegCloseKey( h_key );

		return std::string( steam_path_reg ) + "\"";
	}
}