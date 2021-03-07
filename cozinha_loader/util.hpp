#pragma once

namespace u
{
	inline std::string wstring_to_string( std::wstring wstring )
	{
		if ( wstring.empty( ) )
			return std::string( );

		const auto size = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstring[ 0 ], wstring.size( ), nullptr, 0, nullptr, nullptr );
		auto ret = std::string( size, 0 );
		WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, &wstring[ 0 ], wstring.size( ), &ret[ 0 ], size, nullptr, nullptr );

		return ret;
	}

	inline std::wstring string_to_wstring( std::string string )
	{
		if ( string.empty( ) )
			return std::wstring( );

		const auto len = string.length( ) + 1;
		auto ret = std::wstring( len, 0 );
		const auto size = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, &string[ 0 ], string.size( ), &ret[ 0 ], len );
		ret.resize( size );

		return ret;
	}

	inline std::string to_lower( std::string string )
	{
		std::transform( string.begin( ), string.end( ), string.begin( ), static_cast< int( * )( int ) >( ::tolower ) );
		return string;
	}

	inline std::string to_upper( std::string string )
	{
		std::transform( string.begin( ), string.end( ), string.begin( ), static_cast< int( * )( int ) >( ::toupper ) );
		return string;
	}

	inline bool read_file_to_memory( const std::string &file_path, std::vector<std::uint8_t> *out_buffer )
	{
		std::ifstream file_ifstream( file_path, std::ios::binary );
		if ( !file_ifstream )
			return false;

		out_buffer->assign( ( std::istreambuf_iterator<char>( file_ifstream ) ), std::istreambuf_iterator<char>( ) );
		file_ifstream.close( );

		return true;
	}

	inline std::string get_steam_path( )
	{
		HKEY h_key {};
		if ( RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS )
		{
			_log( LERROR, "Failed to find steam registry." );
			RegCloseKey( h_key );
			std::exit( 0 );
		}

		char steam_path_reg[ MAX_PATH ] {}; steam_path_reg[ 0 ] = '"';
		DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

		// Query SteamExe path from regedit.
		if ( RegQueryValueEx( h_key, "SteamExe", nullptr, nullptr, ( LPBYTE ) ( steam_path_reg + 1 ), &steam_path_size ) != ERROR_SUCCESS )
		{
			_log( LERROR, "Failed to query SteamExe." );
			RegCloseKey( h_key );
			std::exit( 0 );
		}

		RegCloseKey( h_key );

		return std::string( steam_path_reg ) + "\"";
	}
}