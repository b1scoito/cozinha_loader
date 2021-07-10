#include "pch.hpp"
#include "utils.hpp"

namespace string
{
	std::string to_lower( std::string str )
	{
		std::transform( str.begin(), str.end(), str.begin(), static_cast<int( * )( int )>( ::tolower ) );
		return str;
	}

	std::string to_utf8( std::wstring wstr )
	{
		if ( wstr.empty() )
			return {};

		const auto size = WideCharToMultiByte( CP_UTF8, 0, wstr.data(), (int) wstr.size(), 0, 0, 0, 0 );
		auto ret = std::string( size, 0 );

		WideCharToMultiByte( CP_UTF8, 0, wstr.data(), (int) wstr.size(), ret.data(), size, 0, 0 );

		return ret;
	}

	std::wstring to_unicode( std::string str )
	{
		if ( str.empty() )
			return {};

		const auto size = MultiByteToWideChar( CP_UTF8, 0, str.data(), (int) str.size(), 0, 0 );
		auto ret = std::wstring( size, 0 );

		MultiByteToWideChar( CP_UTF8, 0, str.data(), (int) str.size(), ret.data(), size );

		return ret;
	}
}

namespace ext
{
	bool read_file_to_memory( const std::filesystem::path& path, std::vector<std::uint8_t>* out_buffer )
	{
		std::ifstream file( path, std::ios::binary );
		if ( file.fail() )
			return false;

		out_buffer->assign( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );

		file.close();

		return true;
	}

	bool write_file_from_memory( std::string_view name, std::vector<std::uint8_t> buffer )
	{
		std::ofstream file( name );
		if ( file.fail() )
			return false;

		std::ostream_iterator<std::uint8_t> iterator( file );
		std::copy( buffer.begin(), buffer.end(), iterator );

		file.close();

		return true;
	}

	std::wstring get_steam_path()
	{
		HKEY h_key {};
		if ( RegOpenKeyEx( HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS )
		{
			RegCloseKey( h_key );
			return {};
		}

		wchar_t steam_path_reg[MAX_PATH] {}; steam_path_reg[0] = '"';
		DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

		if ( RegQueryValueEx( h_key, L"SteamExe", nullptr, nullptr, (LPBYTE) ( steam_path_reg + 1 ), &steam_path_size ) != ERROR_SUCCESS )
		{
			RegCloseKey( h_key );
			return {};
		}

		RegCloseKey( h_key );

		return std::wstring( steam_path_reg ) + L"\"";
	}
}