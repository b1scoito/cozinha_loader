#pragma once

namespace utils
{
	namespace string
	{
		inline std::string to_lower( std::string string )
		{
			std::transform( string.begin(), string.end(), string.begin(), static_cast<int(*)(int)>(::tolower) );
			return string;
		}

		inline std::string wstring_to_string( std::wstring wstring )
		{
			if (wstring.empty())
				return {};

			const auto size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstring[0], wstring.size(), nullptr, 0, nullptr, nullptr );
			auto ret = std::string( size, 0 );
			WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstring[0], wstring.size(), &ret[0], size, nullptr, nullptr );

			return ret;
		}
	}

	namespace other
	{
		inline bool read_file_to_memory( const std::string &path, std::vector<std::uint8_t> *out_buffer )
		{
			std::ifstream file( path, std::ios::binary );
			if (!file)
				return {};

			out_buffer->assign( (std::istreambuf_iterator<char>( file )), std::istreambuf_iterator<char>() );
			file.close();

			return true;
		}

		inline std::string get_steam_path()
		{
			HKEY h_key {};
			if (RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS)
			{
				RegCloseKey( h_key );
				return {};
			}

			char steam_path_reg[MAX_PATH] {}; steam_path_reg[0] = '"';
			DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

			if (RegQueryValueEx( h_key, "SteamExe", nullptr, nullptr, (LPBYTE) (steam_path_reg + 1), &steam_path_size ) != ERROR_SUCCESS)
			{
				RegCloseKey( h_key );
				return {};
			}

			RegCloseKey( h_key );

			return std::string( steam_path_reg ) + "\"";
		}

		inline bool safe_close_handle( HANDLE h_handle )
		{
			return h_handle && h_handle != INVALID_HANDLE_VALUE ? CloseHandle( h_handle ) : true;
		}
	}
}