#pragma once

namespace utils
{
	namespace string
	{
		inline std::string to_lower( std::string str )
		{
			std::transform( str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(::tolower) );
			return str;
		}

		inline std::string to_upper( std::string str )
		{
			std::transform( str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(::toupper) );
			return str;
		}

		inline std::wstring string_to_wstring( std::string str )
		{
			if ( str.empty() )
				return std::wstring();

			const auto len = str.length() + 1;
			auto ret = std::wstring( len, 0 );
			const auto size = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, &str[0], str.size(), &ret[0], len );
			ret.resize( size );

			return ret;
		}

		inline std::string wstring_to_string( std::wstring wstr )
		{
			if ( wstr.empty() )
				return std::string();

			const auto size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), nullptr, 0, nullptr, nullptr );
			auto ret = std::string( size, 0 );
			WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), &ret[0], size, nullptr, nullptr );

			return ret;
		}
	}

	namespace other
	{
		inline bool read_file_to_memory( const std::string& path, std::vector<std::uint8_t>* out_buffer )
		{
			std::ifstream file( path, std::ios::binary );
			if ( !file )
				return {};

			out_buffer->assign( (std::istreambuf_iterator<char>( file )), std::istreambuf_iterator<char>() );
			file.close();

			return true;
		}

		inline std::string get_steam_path()
		{
			HKEY h_key{};
			if ( RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS )
			{
				RegCloseKey( h_key );
				return {};
			}

			char steam_path_reg[MAX_PATH]{}; steam_path_reg[0] = '"';
			DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

			if ( RegQueryValueEx( h_key, "SteamExe", nullptr, nullptr, (LPBYTE) (steam_path_reg + 1), &steam_path_size ) != ERROR_SUCCESS )
			{
				RegCloseKey( h_key );
				return {};
			}

			RegCloseKey( h_key );

			return std::string( steam_path_reg ) + "\"";
		}
	}

	namespace vars
	{
		inline std::string cheat_filename = "cheat.dll";
	}
}