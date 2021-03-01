#include "pch.h"

// WinMain definition as-is.
//
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	_log( LINFO, "Starting application." );

	// sleep for 5 seconds and exit before exiting. if that makes sense
	//
	std::atexit( [ ]( ) { std::this_thread::sleep_for( std::chrono::seconds( 5 ) ); std::exit( 0 ); } );

	// check if vac3 bypass dll exists
	//
	if ( !std::filesystem::exists( "vac3_b.dll" ) )
	{
		_log( LERROR, "[filesystem_exists]: vac bypass dll not found." );
		return EXIT_FAILURE;
	}

	// getting vac3 bypass dll path
	//
	auto vac_dll_path = std::filesystem::absolute( "vac3_b.dll" );

	// definition of our dll "buffer"
	//
	std::vector<std::uint8_t> vac_buffer {};

	// read file to our variable in memory
	//
	if ( !util::read_file_to_memory( vac_dll_path.string( ).c_str( ), &vac_buffer ) )
	{
		_log( LERROR, "[read_file_to_memory]: Failed to write vac dll to buffer." );
		return EXIT_FAILURE;
	}

	// check if cheat dll exists
	//
	if ( !std::filesystem::exists( "cheat.dll" ) )
	{
		_log( LERROR, "[filesystem_exists]: cheat dll not found." );
		return EXIT_FAILURE;
	}

	// getting cheat dll path
	//
	auto cheat_dll_path = std::filesystem::absolute( "cheat.dll" );

	// definition of our dll "buffer"
	//
	std::vector<std::uint8_t> cheat_buffer {};

	// read file to our variable in memory
	//
	if ( !util::read_file_to_memory( cheat_dll_path.string( ).c_str( ), &cheat_buffer ) )
	{
		_log( LERROR, "[read_file_to_memory]: Failed to write cheat dll to buffer." );
		return EXIT_FAILURE;
	}

	// open steam regedit key
	//
	HKEY h_key {};
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS )
	{
		_log( LERROR, "[reg_open_key_ex]: Failed to find steam registry." );
		RegCloseKey( h_key );

		return EXIT_FAILURE;
	}

	char steam_path_reg[ MAX_PATH ] {}; steam_path_reg[ 0 ] = '"';
	DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

	// getting our steamexe path from regedit
	//
	if ( RegQueryValueEx( h_key, "SteamExe", nullptr, nullptr, ( LPBYTE ) ( steam_path_reg + 1 ), &steam_path_size ) != ERROR_SUCCESS )
	{
		_log( LERROR, "[reg_query_value_key]: Failed to query SteamExe." );
		RegCloseKey( h_key );

		return EXIT_FAILURE;
	}

	// close handle to regkey
	//
	RegCloseKey( h_key );

	// our steam path string
	//
	auto steam_path = std::string( steam_path_reg ) + "\"";
	_log( LINFO, "[main]: Got steam path: %s.", steam_path.c_str( ) );

	// close all process related to steam and csgo for the injection to begin
	//
	mem::kill_process( "csgo.exe" );

	while ( true )
	{
		mem::kill_process( "steam.exe" );
		mem::kill_process( "steamservice.exe" );
		mem::kill_process( "steamwebhelper.exe" );

		if ( !mem::is_process_open( "steam.exe" )
			&& !mem::is_process_open( "steamservice.exe" )
			&& !mem::is_process_open( "steamwebhelper.exe" ) )
			break;

		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
	}

	// open steam with console opened
	//
	PROCESS_INFORMATION pi {};
	if ( !mem::open_process( steam_path.c_str( ), { "-console" }, pi ) )
	{
		_log( LERROR, "[open_process]: Failed to open steam." );

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return EXIT_FAILURE;
	}

	// close our handles to open process
	//
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	// check if steamservice.exe is opened.
	//
	if ( mem::is_process_open( "steamservice.exe" ) )
	{
		_log( LWARN, "[process_open]: Steam service is running, steam was not opened as admin." );
		return EXIT_FAILURE;
	}

	_log( LINFO, "[blackbone]: Starting injection process." );

	auto mod_callback = [ ]( blackbone::CallbackType type, void *, blackbone::Process &, const blackbone::ModuleData &modInfo )
	{
		std::string user32 = "user32.dll";
		if ( type == blackbone::PreCallback )
		{
			if ( modInfo.name == std::wstring( user32.begin( ), user32.end( ) ) )
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}

		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	// bypassing injection block by csgo the easiest way.
	//
	auto bypass_nt_open_file = [ ]( DWORD pid )
	{
		// get the handle for our process
		//
		auto h_process = OpenProcess( PROCESS_ALL_ACCESS, false, pid );

		// get the procedure address of NtOpenFile
		//
		void *nt_open_file_address = GetProcAddress( LoadLibraryW( L"ntdll" ), "NtOpenFile" );

		// if procedure address found
		//
		if ( nt_open_file_address )
		{
			char bytes[ 5 ];

			// copy 5 bytes to NtOpenFile procedure address
			//
			std::memcpy( bytes, nt_open_file_address, 5 );

			// write memory to process
			//
			WriteProcessMemory( h_process, nt_open_file_address, bytes, 5, nullptr );
		}

		// close handle
		//
		CloseHandle( h_process );
	};

	_log( LINFO, "[vac_injection]: Injecting vac bypass into steam." );

	// spawning blackbone process variable
	//
	blackbone::Process steam_process {};

	// attaching blackbone to the process
	//
	steam_process.Attach( mem::get_process_id_by_name( "steam.exe" ), PROCESS_ALL_ACCESS );

	// suspending process for injection
	//
	steam_process.Suspend( );

	// mapping dll buffer to the process
	//
	if ( !steam_process.mmap( ).MapImage( vac_buffer.size( ), vac_buffer.data( ), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success( ) )
	{
		_log( LERROR, "[vac_injection]: Failed to inject vac bypass into steam." );

		steam_process.Resume( );
		steam_process.Detach( );

		mem::kill_process( "steam.exe" );

		return EXIT_FAILURE;
	}

	// resume process and detach blackbone from it
	//
	steam_process.Resume( );
	steam_process.Detach( );

	_log( LINFO, "[vac_injection]: vac bypass injected." );

	// csgo injection
	//

	// didn't think a better way of doing this
	//
	std::system( "start steam://rungameid/730" );

	_log( LINFO, "[csgo_injection]: opening csgo." );

	// wait for csgo to be opened
	//
	while ( true )
	{
		if ( mem::is_process_open( "csgo.exe" ) )
			break;

		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
	}

	_log( LINFO, "[csgo_injection]: Injecting cheat into csgo." );

	_log( LINFO, "[csgo_injection]: Applying NtOpenFile bypass." );

	// apply the NtOpenFile bypass
	//
	bypass_nt_open_file( mem::get_process_id_by_name( "csgo.exe" ) );

	_log( LINFO, "[csgo_injection]: Bypass applied." );

	blackbone::Process csgo_process {};

	// attach to csgo
	//
	csgo_process.Attach( mem::get_process_id_by_name( "csgo.exe" ), PROCESS_ALL_ACCESS );

	auto &bb_game_modules = csgo_process.modules( );
	auto modules = bb_game_modules.GetAllModules( );

	_log( LINFO, "[csgo_injection]: Waiting for serverbrowser.dll." );

	if ( !modules.empty( ) )
	{
		bool found = false;
		while ( !found )
		{
			for ( auto &mod : modules )
			{
				// check for serverbrowser.dll
				//
				if ( mod.second->name[ 0 ] == L's' && mod.second->name[ 6 ] == L'b' && mod.second->name[ 9 ] == L'w' )
				{
					found = true;
					break;
				}
			}

			if ( found )
				break;

			std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
			modules = bb_game_modules.GetAllModules( );
		}
	}

	_log( LINFO, "[csgo_injection]: serverbrowser.dll found." );

	// manualmap to csgo
	//
	if ( !csgo_process.mmap( ).MapImage( cheat_buffer.size( ), cheat_buffer.data( ), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success( ) )
	{
		_log( LERROR, "[csgo_injection]: Failed to inject cheat into csgo." );

		steam_process.Resume( );
		steam_process.Detach( );

		mem::kill_process( "csgo.exe" );

		return EXIT_FAILURE;
	}

	// resume and detach from csgo you're done mate.
	csgo_process.Resume( );
	csgo_process.Detach( );

	_log( LSUCCESS, "[main]: Everything ready to go." );

	return EXIT_SUCCESS;
}