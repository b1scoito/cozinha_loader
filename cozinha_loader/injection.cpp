#include "pch.hpp"
#include "injection.hpp"

bool c_injector::init( std::string proc_name, std::string cheat_name )
{
	// ~ closing processes
	close_processes( { proc_name, "steam.exe" } );

	const auto steam_path = other::get_steam_path();
	if ( steam_path.empty() )
	{
		log_err( "Failed to retrieve steam path!" );
		return false;
	}

	log_debug( "Opening steam [ %ls ]...", steam_path.c_str() );

	std::string append_option {};

	for ( const auto& id : app_ids )
	{
		if ( id.second.find( proc_name ) != std::string::npos )
			append_option = string::format( "-applaunch %d", id.first );
	}

	PROCESS_INFORMATION pi {};
	if ( !memory::open_process( steam_path, { L"-console", string::to_unicode( append_option ) }, pi ) )
	{
		log_err( "Failed to open steam!" );

		if ( pi.hProcess )
			CloseHandle( pi.hProcess );

		if ( pi.hThread )
			CloseHandle( pi.hThread );

		return false;
	}

	if ( pi.hProcess )
		CloseHandle( pi.hProcess );

	if ( pi.hThread )
		CloseHandle( pi.hThread );

	std::vector<std::uint8_t> cheat_buf {};

	// ~ reading file and writing it to a variable
	if ( !other::read_file_to_memory( std::filesystem::absolute( cheat_name ).string(), &cheat_buf ) )
	{
		log_err( "Failed to write DLL to memory!" );
		return false;
	}

	// ~ inject vac bypass to steam
	if ( !map( "steam.exe", L"tier0_s.dll", vac3_data ) )
	{
		log_err( "Steam memory mapping failure!" );
		return false;
	}

	// ~ inject cheat to process
	if ( !map( proc_name, L"serverbrowser.dll", cheat_buf ) )
	{
		log_err( "Cheat memory mapping failure!" );
		return false;
	}

	log_ok( "All done." );

	return true;
}

bool c_injector::map( std::string proc, std::wstring mod_name, std::vector<std::uint8_t> buf )
{
	// ~ wait for process to be opened
	log_debug( "Waiting for - [ %s ] to be opened...", proc.c_str() );

	auto proc_list = memory::get_process_list();
	do
	{
		proc_list = memory::get_process_list();

		std::this_thread::sleep_for( 500ms );

	} while ( !memory::is_process_open( proc_list, proc ) );

	// ~ bypassing injection block by csgo (-allow_third_party_software) the easiest way
	if ( proc.find( "csgo" ) != std::string::npos )
	{
		const auto bypass_nt_open_file = []( uint32_t pid )
		{
			const auto h_process = OpenProcess( PROCESS_ALL_ACCESS, false, pid );
			const auto nt_dll = LoadLibrary( L"ntdll" );

			if ( !nt_dll )
				return false;

			LPVOID nt_open_file = GetProcAddress( nt_dll, "NtOpenFile" );

			if ( !nt_open_file )
				return false;

			char original_bytes[5];

			// ~ copy 5 bytes to NtOpenFile procedure address
			std::memcpy( original_bytes, nt_open_file, 5 );

			// ~ write it to memory
			WriteProcessMemory( h_process, nt_open_file, original_bytes, 5, nullptr );

			CloseHandle( h_process );

			return true;
		};

		if ( !bypass_nt_open_file( memory::get_process_id_by_name( proc_list, proc ) ) )
			return false;
	}

	blackbone::Process bb_proc;

	bb_proc.Attach( memory::get_process_id_by_name( proc_list, proc ), PROCESS_ALL_ACCESS );

	log_debug( "Waiting for - [ %ls ] in %s...", mod_name.c_str(), proc.c_str() );

	// ~ wait for a process module so we can continue with injection
	auto mod_ready = false;
	while ( !mod_ready )
	{
		for ( const auto& mod : bb_proc.modules().GetAllModules() )
		{
			if ( mod.first.first == mod_name )
			{
				mod_ready = true;
				break;
			}
		}

		if ( mod_ready )
			break;

		std::this_thread::sleep_for( 500ms );
	}

	// ~ resolve PE imports
	const auto mod_callback = []( blackbone::CallbackType type, void*, blackbone::Process&, const blackbone::ModuleData& modInfo )
	{
		if ( type == blackbone::PreCallback )
		{
			if ( modInfo.name == L"user32.dll" )
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}

		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	const auto call_result = bb_proc.mmap().MapImage( buf.size(), buf.data(), false, blackbone::WipeHeader | blackbone::NoThreads, mod_callback );

	// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
	log_debug( "Map Result - [ 0x%.8X ]", call_result.status );

	// ~ mapping dll bytes to the process
	if ( !call_result.success() )
	{
		log_err( "Failed to inject into [ %s ]!", proc.c_str() );

		bb_proc.Terminate();
		bb_proc.Detach();

		return false;
	}

	// ~ free memory and detach from process
	bb_proc.Detach();

	log_ok( "Injected into %s.", proc.c_str() );

	return true;
}

void c_injector::close_processes( std::vector<std::string> processes )
{
	auto proc_list = memory::get_process_list();
	for ( const auto& proc : processes )
	{
		do
		{
			memory::kill_process( proc_list, proc );
			proc_list = memory::get_process_list();

			std::this_thread::sleep_for( 500ms );

		} while ( memory::is_process_open( proc_list, proc ) );
	}
}