#include "pch.h"
#include "injection.h"

bool injection::inject( std::string process, bool csgo, std::vector<std::uint8_t> buffer )
{
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

	if ( csgo )
	{
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

		// apply the NtOpenFile bypass
		//
		bypass_nt_open_file( mem::get_process_id_by_name( process.c_str( ) ) );

		_log( LINFO, "NtOpenFile bypass applied." );
	}

	// spawning blackbone process variable
	//
	blackbone::Process bb_process {};

	// attaching blackbone to the process
	//
	bb_process.Attach( mem::get_process_id_by_name( process.c_str( ) ), PROCESS_ALL_ACCESS );

	if ( csgo )
	{
		_log( LINFO, "Waiting for serverbrowser.dll." );

		bool c_ready = false;
		while ( !c_ready )
		{
			for ( auto i : bb_process.modules( ).GetAllModules( ) )
			{
				if ( i.first.first == L"serverbrowser.dll" )
				{
					c_ready = true;
					break;
				}
			}

			if ( c_ready )
				break;
		}

		_log( LINFO, "serverbrowser.dll found." );
	}

	_log( LWARN, "Injecting into %s.", process.c_str( ) );

	// wait for tier0 to be loaded so we can inject
	//
	bool s_ready = false;
	while ( !s_ready )
	{
		for ( auto i : bb_process.modules( ).GetAllModules( ) )
		{
			if ( i.first.first == L"tier0_s.dll" )
			{
				s_ready = true;
				break;
			}
		}

		if ( s_ready )
			break;
	}

	// mapping dll buffer to the process
	//
	if ( !bb_process.mmap( ).MapImage( buffer.size( ), buffer.data( ), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success( ) )
	{
		_log( LERROR, "Failed to inject into %s.", process.c_str( ) );

		bb_process.Detach( );

		mem::kill_process( process.c_str( ) );

		return EXIT_FAILURE;
	}

	// detach blackbone from the target process
	//
	bb_process.Detach( );

	_log( LINFO, "Injection to %s process done.", process.c_str( ) );

	return EXIT_SUCCESS;
}

bool injection::setup( )
{
	// check if vac3 bypass dll exists
	//
	if ( !std::filesystem::exists( "vac3_b.dll" ) )
	{
		_log( LERROR, "VAC bypass (vac3_b.dll) not found." );
		return EXIT_FAILURE;
	}

	// getting vac3 bypass dll path
	//
	auto vac_dll_path = std::filesystem::absolute( "vac3_b.dll" );

	// read file to our variable in memory
	//
	if ( !util::read_file_to_memory( vac_dll_path.string( ).c_str( ), &vac_buffer ) )
	{
		_log( LERROR, "Failed to write vac bypass dll to buffer." );
		return EXIT_FAILURE;
	}

	// check if cheat dll exists
	//
	if ( !std::filesystem::exists( "cheat.dll" ) )
	{
		_log( LERROR, "Cheat (cheat.dll) not found." );
		return EXIT_FAILURE;
	}

	// getting cheat dll path
	//
	auto cheat_dll_path = std::filesystem::absolute( "cheat.dll" );

	// read file to our variable in memory
	//
	if ( !util::read_file_to_memory( cheat_dll_path.string( ).c_str( ), &cheat_buffer ) )
	{
		_log( LERROR, "Failed to write cheat dll to buffer." );
		return EXIT_FAILURE;
	}

	// open steam regedit key
	//
	HKEY h_key {};
	if ( RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &h_key ) != ERROR_SUCCESS )
	{
		_log( LERROR, "Failed to find steam registry." );
		RegCloseKey( h_key );

		return EXIT_FAILURE;
	}

	char steam_path_reg[ MAX_PATH ] {}; steam_path_reg[ 0 ] = '"';
	DWORD steam_path_size = sizeof( steam_path_reg ) - sizeof( char );

	// getting our steamexe path from regedit
	//
	if ( RegQueryValueEx( h_key, "SteamExe", nullptr, nullptr, ( LPBYTE ) ( steam_path_reg + 1 ), &steam_path_size ) != ERROR_SUCCESS )
	{
		_log( LERROR, "Failed to query SteamExe." );
		RegCloseKey( h_key );

		return EXIT_FAILURE;
	}

	// close handle to regkey
	//
	RegCloseKey( h_key );

	// check if steamservice.exe is opened.
	//
	if ( mem::is_process_open( "steamservice.exe" ) )
	{
		_log( LERROR, "Steam service is running, steam was not opened as admin." );
		return EXIT_FAILURE;
	}

	// our steam path string
	//
	auto steam_path = std::string( steam_path_reg ) + "\"";
	_log( LINFO, "Got steam path." );

	// open steam with console opened
	//
	PROCESS_INFORMATION pi {};
	if ( !mem::open_process( steam_path.c_str( ), { "-console" }, pi ) )
	{
		_log( LERROR, "Failed to open steam." );

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return EXIT_FAILURE;
	}

	// close our handles to open process
	//
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	// wait for steam to be opened
	//
	wait_for_process( "steam.exe" );

	// inject vac bypass to steam
	//
	inject( "steam.exe", false, vac_buffer );

	// open csgo
	//
	_log( LINFO, "Waiting for csgo to be opened." );
	std::system( "start steam://rungameid/730" );

	// wait for csgo to be opened
	//
	wait_for_process( "csgo.exe" );

	// inject cheat to csgo
	// 
	inject( "csgo.exe", true, cheat_buffer );

	_log( LSUCCESS, "All done." );
	return EXIT_SUCCESS;
}

void injection::wait_for_process( std::string process )
{
	// while process is not opened loop
	//
	while ( true )
	{
		// if process opened
		//
		if ( mem::is_process_open( process ) )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

			// break!!
			//
			break;
		}
	}
}

void injection::close_processes( std::vector<std::string> processes )
{
	// loop through processes
	//
	for ( auto &process : processes )
	{
		// check if it exists
		//
		if ( mem::is_process_open( process ) )
		{
			// kill it
			//
			mem::kill_process( process );
		}
	}
}