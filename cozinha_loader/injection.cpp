#include "pch.h"
#include "injection.h"

bool injection::inject( std::string process, std::wstring module_name, std::filesystem::path path_to_dll )
{
	std::vector<std::uint8_t> buffer;

	// Reading file and writing it to a variable
	if ( !u::read_file_to_memory( path_to_dll.string( ), &buffer ) )
	{
		_log( LERROR, "Failed to write vac bypass dll to buffer." );
		return EXIT_FAILURE;
	}

	// Wait for process to be opened
	while ( !mem::is_process_open( process.c_str( ) ) )
		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

	// Resolve PE imports
	const auto mod_callback = [ ]( blackbone::CallbackType type, void *, blackbone::Process &, const blackbone::ModuleData &modInfo )
	{
		std::string user32 = "user32.dll";
		if ( type == blackbone::PreCallback )
		{
			if ( modInfo.name == std::wstring( user32.begin( ), user32.end( ) ) )
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}

		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	if ( process.find( "csgo" ) != std::string::npos )
	{
		// Bypassing injection block by csgo (-allow_third_party_software) the easiest way.
		const auto bypass_nt_open_file = [ ]( DWORD pid )
		{
			// Get the handle for our process
			const auto h_process = OpenProcess( PROCESS_ALL_ACCESS, false, pid );

			LPVOID nt_open_file_address {};

			// Get the procedure address of NtOpenFile.
			if ( nt_open_file_address )
				nt_open_file_address = GetProcAddress( LoadLibraryW( L"ntdll" ), "NtOpenFile" );

			if ( nt_open_file_address )
			{
				char bytes[ 5 ];
				// Copy 5 bytes to NtOpenFile procedure address
				std::memcpy( bytes, nt_open_file_address, 5 );
				// Write it to memory.
				WriteProcessMemory( h_process, nt_open_file_address, bytes, 5, nullptr );
			}

			// Close handle
			CloseHandle( h_process );
		};

		bypass_nt_open_file( mem::get_process_id_by_name( process.c_str( ) ) );
		_log( LINFO, "NtOpenFile bypass applied." );
	}

	// Spawning blackbone process variable
	blackbone::Process bb_process {};

	// Attaching blackbone to the process
	bb_process.Attach( mem::get_process_id_by_name( process.c_str( ) ), PROCESS_ALL_ACCESS );

	_log( LINFO, "Injection is waiting for module %s in %s.", u::wstring_to_string( module_name ).c_str( ), process.c_str( ) );

	// Wait for a process module so we can continue with injection.
	auto mod_ready = false;
	while ( !mod_ready )
	{
		for ( const auto &mod : bb_process.modules( ).GetAllModules( ) )
		{
			if ( mod.first.first == module_name.c_str( ) )
			{
				mod_ready = true;
				break;
			}
		}

		if ( mod_ready )
			break;

		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	}

	// Mapping dll buffer to the process
	if ( !( bb_process.mmap( ).MapImage( buffer.size( ), buffer.data( ), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success( ) ) )
	{
		_log( LERROR, "Failed to inject into %s.", process.c_str( ) );
		bb_process.Detach( );
		return EXIT_FAILURE;
	}

	_log( LSUCCESS, "Injected to %s.", process.c_str( ) );

	// Detach blackbone from the target process.
	bb_process.Detach( );

	return EXIT_SUCCESS;
}

bool injection::setup( )
{
	if ( !std::filesystem::exists( "vac3_b.dll" ) )
	{
		_log( LERROR, "vac3_b.dll not found. (vac bypass)" );
		return EXIT_FAILURE;
	}

	const auto vac_dll_path = std::filesystem::absolute( "vac3_b.dll" );

	if ( !std::filesystem::exists( "cheat.dll" ) )
	{
		_log( LERROR, "cheat.dll not found. (cheat)" );
		return EXIT_FAILURE;
	}

	const auto cheat_dll_path = std::filesystem::absolute( "cheat.dll" );

	const auto steam_path = u::get_steam_path( );
	_log( LINFO, "Found SteamExe path: %s.", steam_path.c_str( ) );

	// Open steam with console opened.
	PROCESS_INFORMATION pi {};
	if ( !mem::open_process( steam_path.c_str( ), { "-console" }, pi ) )
	{
		_log( LERROR, "Failed to open steam." );

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return EXIT_FAILURE;
	}

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	// Inject vac bypass to steam
	g_injection->inject( "steam.exe", L"tier0_s.dll", vac_dll_path );

	// Open csgo
	std::system( "start steam://rungameid/730" );

	// Inject cheat to csgo
	g_injection->inject( "csgo.exe", L"serverbrowser.dll", cheat_dll_path );

	_log( LSUCCESS, "All done." );
	return EXIT_SUCCESS;
}

void injection::close_processes( std::vector<std::string> processes )
{
	_log( LINFO, "Closing processes." );

	for ( const auto &process : processes )
	{
		while ( mem::is_process_open( process ) )
			mem::kill_process( process );
	}
}