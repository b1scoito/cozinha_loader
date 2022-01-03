#include "pch.hpp"
#include "injection.hpp"

bool c_injector::initalize( const std::filesystem::path dll_path )
{
	if ( vars::b_inject_vac_bypass )
	{
		log_debug( L"Closing processes..." );

		// Closing processes
		this->close_processes( { string::to_unicode( vars::str_process_name ), L"steam.exe" } );

		const auto steam_path = util::get_steam_path();

		if ( steam_path.empty() )
			return failure( L"Failed to get Steam path!" );

		std::wstring launch_append = {};
		if ( vars::b_open_game_automatically )
		{
			for ( const auto& it : this->vec_app_ids )
			{
				if ( it.second.find( string::to_unicode( vars::str_process_name ) ) != std::wstring::npos )
					launch_append = string::format( L"-applaunch %d", it.first );
			}
		}

#ifdef _DEBUG
		launch_append += L" -windowed -w 1280 -h 720";
#endif

		PROCESS_INFORMATION pi; // Could use the current handle instead of closing it for steam, might do it in the future...
		if ( !memory::open_process( steam_path, { L"-console", launch_append }, pi ) )
			return failure( L"Failed to open Steam!", { pi.hProcess, pi.hThread } );

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		// This won't take long
		std::vector<std::uint8_t> vac_buffer( std::begin( vac3_data ), std::end( vac3_data ) );

		// Inject vac bypass to steam
		if ( !this->map( L"steam.exe", string::to_unicode( vars::str_steam_mod_name ), vac_buffer ) )
			return false;

		// Start Steam heartbeat thread
		std::thread(&c_injector::check_for_steam_thread, this).detach();
	}

	std::vector<std::uint8_t> dll_buffer = {};
	if ( !util::read_file_to_memory( std::filesystem::absolute( dll_path ), &dll_buffer ) )
		return failure( L"Failed to write DLL to memory!" );

	// Inject DLL to process
	if ( !this->map( string::to_unicode( vars::str_process_name ), string::to_unicode( vars::str_process_mod_name ), dll_buffer ) )
		return false;

	return true;
}

bool c_injector::map( std::wstring_view str_proc, std::wstring_view wstr_mod_name, std::vector<std::uint8_t> vec_buffer, blackbone::eLoadFlags flags )
{
	log_debug( L"Waiting for %s to open...", str_proc.data() );

	// Update process list while process is not opened
	auto proc_list = memory::get_process_list();
	do
	{
		proc_list = memory::get_process_list();
		std::this_thread::sleep_for( 500ms );
	}
	while ( !memory::is_process_open( proc_list, str_proc ) );

	blackbone::Process bb_proc;
	bb_proc.Attach( memory::get_process_id_by_name( proc_list, str_proc ), PROCESS_ALL_ACCESS ); // PROCESS_ALL_ACCESS not needed perhaps?

	// Wait for a process module so we can continue with injection
	log_debug( L"Waiting for module %s in %s...", wstr_mod_name.data(), str_proc.data() );

	auto mod_ready = false;
	while ( !mod_ready )
	{
		for ( const auto& mod : bb_proc.modules().GetAllModules() )
		{
			if ( mod.first.first == wstr_mod_name )
			{
				mod_ready = true;
				break;
			}
		}

		if ( mod_ready )
			break;

		std::this_thread::sleep_for( 500ms ); // 1s? fixes 0xC34... (i think i was calling the patch too early now)
	}

	// Bypassing injection block by csgo (-allow_third_party_software)
	if ( str_proc.find( L"csgo" ) != std::wstring::npos )
	{
		const auto patch_nt_open_file = [&]()
		{
			const auto ntdll_path = string::format( L"%s\\ntdll.dll", util::get_system_directory().data() );
			const auto ntdll = LoadLibrary( ntdll_path.data() );

			if ( !ntdll )
				return failure( L"Failed to load ntdll?" );

			void* ntopenfile_ptr = GetProcAddress( ntdll, "NtOpenFile" );

			if ( !ntopenfile_ptr )
				return failure( L"Failed to get NtOpenFile proc address?" );

			std::uint8_t restore[5];
			std::memcpy( restore, ntopenfile_ptr, sizeof( restore ) );

			const auto result = bb_proc.memory().Write( (std::uintptr_t) ntopenfile_ptr, restore );

			if ( !NT_SUCCESS( result ) )
				return failure( L"Failed to write patch memory!" );

			return true;
		};

		if ( !patch_nt_open_file() )
			return failure( L"Failed to patch NtOpenFile!" );
	}

	// Resolve PE imports
	const auto mod_callback = []( blackbone::CallbackType type, void*, blackbone::Process&, const blackbone::ModuleData& modInfo )
	{
		if ( type == blackbone::PreCallback )
		{
			if ( modInfo.name == L"user32.dll" )
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}

		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	// Mapping dll to the process
	const auto call_result = bb_proc.mmap().MapImage( vec_buffer.size(), vec_buffer.data(), false, flags, mod_callback );

	if ( !call_result.success() )
	{
		// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
		log_err( L"Failed to inject into %s. NT Status: 0x%.8X", str_proc.data(), call_result.status );
		bb_proc.Detach();

		return false;
	}

	// Free memory and detach from process
	bb_proc.Detach();
	log_ok( L"Injected into %s successfully!", str_proc.data() );

	return true;
}

void c_injector::close_processes( std::vector<std::wstring> vec_processes )
{
	auto proc_list = memory::get_process_list();
	for ( const auto& proc : vec_processes )
	{
		do
		{
			memory::kill_process( proc_list, proc );

			proc_list = memory::get_process_list();
			std::this_thread::sleep_for( 25ms );
		}
		while ( memory::is_process_open( proc_list, proc ) );
	}
}

void c_injector::check_for_steam_thread()
{
	auto proc_list = memory::get_process_list();

	while ( true )
	{
		proc_list = memory::get_process_list();
		if (!memory::is_process_open(proc_list, L"steam.exe"))
			break;

		std::this_thread::sleep_for(150ms);
	}

	log_warn(L"Steam was closed, exiting...");

	// Exit if steam was closed (because they logged off or other reasons...)
	std::exit(0);
}