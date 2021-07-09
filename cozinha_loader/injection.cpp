#include "pch.hpp"
#include "injection.hpp"

const auto failure = []( std::string_view str_err, std::pair<HANDLE, HANDLE> handles = {} ) -> bool
{
	if ( handles.first ) // hProcess
		CloseHandle( handles.first );

	if ( handles.second ) // hThread
		CloseHandle( handles.second );

	log_err( "%s", str_err );
	return false;
};

bool c_injector::init( std::string_view str_proc_name, const std::filesystem::path dll_path )
{
	// closing processes
	close_processes( { str_proc_name, "steam.exe" } );

	const auto steam_path = ext::get_steam_path();

	if ( steam_path.empty() )
		return failure( "Failed to retrieve steam path" );

	std::string launch_append {};
	for ( const auto& it : vec_app_ids )
	{
		if ( it.second.find( str_proc_name ) != std::string::npos )
			launch_append = string::format( "-applaunch %d", it.first );
	}

	log_debug( "Opening steam [ %ls ]...", steam_path.c_str() );

	PROCESS_INFORMATION pi;
	if ( !memory::open_process( steam_path, { L"-console", string::to_unicode( launch_append ) }, pi ) )
		return failure( "Failed to open steam", { pi.hProcess, pi.hThread } );

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	std::vector<std::uint8_t> dll_buffer;
	if ( !ext::read_file_to_memory( std::filesystem::absolute( dll_path ), &dll_buffer ) )
		return failure( "Failed to write DLL to memory!" );

	// inject vac bypass to steam
	if ( !map( "steam.exe", L"tier0_s.dll", vac3_data ) )
		return false;

	// inject dll to process
	if ( !map( str_proc_name, L"serverbrowser.dll", dll_buffer ) )
		return false;

	return true;
}

bool c_injector::map( std::string_view str_proc, std::wstring_view wstr_mod_name, std::vector<std::uint8_t> vec_buffer )
{
	auto proc_list = memory::get_process_list();

	log_debug( "Waiting for process - [ %s ]", str_proc );

	// update process list while process is not opened
	do
	{
		proc_list = memory::get_process_list();
		std::this_thread::sleep_for( 500ms );
	}
	while ( !memory::is_process_open( proc_list, str_proc ) );

	blackbone::Process bb_proc;
	bb_proc.Attach( memory::get_process_id_by_name( proc_list, str_proc ), PROCESS_ALL_ACCESS ); // PROCESS_ALL_ACCESS not needed perhaps? placed it back in

	// wait for a process module so we can continue with injection
	log_debug( "Waiting for - [ %ls ] in %s", wstr_mod_name.data(), str_proc );

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

	// bypassing injection block by csgo (-allow_third_party_software)
	if ( str_proc.find( "csgo" ) != std::string::npos )
	{
		const auto patch_nt_open_file = [&]()
		{
			const auto ntdll = LoadLibrary( L"ntdll" );

			if ( !ntdll )
				return failure( "Failed to load ntdll?" );

			const void* ntopenfile_ptr = GetProcAddress( ntdll, "NtOpenFile" );

			if ( !ntopenfile_ptr )
				return failure( "Failed to get NtOpenFile proc address?" );

			char restore[5];
			std::memcpy( restore, ntopenfile_ptr, sizeof( restore ) );

			const auto result = bb_proc.memory().Write( (uintptr_t) ntopenfile_ptr, restore );

			if ( !NT_SUCCESS( result ) )
				return failure( "Failed to write patch memory" );

			return true;
		};

		if ( !patch_nt_open_file() )
			return false;
	}

	// resolve PE imports
	const auto mod_callback = []( blackbone::CallbackType type, void*, blackbone::Process&, const blackbone::ModuleData& modInfo )
	{
		if ( type == blackbone::PreCallback )
		{
			if ( modInfo.name == L"user32.dll" )
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}

		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	// mapping dll to the process
	const auto call_result = bb_proc.mmap().MapImage( vec_buffer.size(), vec_buffer.data(), false, blackbone::WipeHeader | blackbone::NoThreads, mod_callback );

	// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
	log_debug( "Map Result - [ 0x%.8X ]", call_result.status );

	if ( !call_result.success() )
	{
		log_err( "Failed to inject into - [ %s ]", str_proc );
		bb_proc.Detach();

		return false;
	}

	// free memory and detach from process
	bb_proc.Detach();

	log_ok( "Injection into - [ %s ] done.", str_proc );

	return true;
}

void c_injector::close_processes( const std::vector<std::string_view> vec_processes )
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