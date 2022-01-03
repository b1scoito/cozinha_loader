#include "pch.hpp"
#include "utils.hpp"

namespace string
{
	std::wstring to_lower( std::wstring str )
	{
		std::transform( str.begin(), str.end(), str.begin(), static_cast<int( * )( int )>( ::tolower ) );
		return str;
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

namespace util
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

	bool write_file_from_memory( std::wstring name, std::vector<std::uint8_t> buffer )
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
		winreg::RegKey  steam_path_key { HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam" };

		std::wstring steam_path = steam_path_key.GetStringValue(L"SteamExe");

		return std::wstring(steam_path);
	}

	std::wstring get_system_directory()
	{
		wchar_t buf[MAX_PATH];
		GetSystemDirectory( buf, MAX_PATH );

		return std::wstring( buf );
	}

	std::wstring get_executable_path()
	{
		wchar_t buf[MAX_PATH];
		GetModuleFileName( NULL, buf, MAX_PATH );

		return std::wstring( buf );
	}
}