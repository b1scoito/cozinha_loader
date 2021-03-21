#include "pch.hpp"
#include "injection.hpp"

bool injector::map( std::string process, std::wstring module_name, std::filesystem::path path_to_dll )
{
	std::vector<std::uint8_t> buffer {};

	// Reading file and writing it to a variable
	if (!utils::read_file_to_memory( path_to_dll.string(), &buffer ))
	{
		_loge( "Failed to write %ls to buffer.", path_to_dll.filename().c_str() );
		return EXIT_FAILURE;
	}

	// Wait for process to be opened
	auto proc_list = memory::get_process_list();
	while (true)
	{
		proc_list = memory::get_process_list();
		if (memory::is_process_open( proc_list, process ))
			break;

		// this is a magic number, and for some reason, if it's not 1 second, won't load csgo.
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}

	if (process.find( "csgo" ) != std::string::npos)
	{
		// Bypassing injection block by csgo (-allow_third_party_software) the easiest way.
		const auto bypass_nt_open_file = []( DWORD pid )
		{
			// Get the handle for our process
			const auto h_process = OpenProcess( PROCESS_ALL_ACCESS, false, pid );

			// Get the procedure address of NtOpenFile.
			LPVOID nt_open_file_address {};
			if (nt_open_file_address)
			{
				nt_open_file_address = GetProcAddress( LoadLibrary( "ntdll" ), "NtOpenFile" );
				char bytes[5];
				// Copy 5 bytes to NtOpenFile procedure address
				std::memcpy( bytes, nt_open_file_address, 5 );
				// Write it to memory.
				WriteProcessMemory( h_process, nt_open_file_address, bytes, 5, nullptr );
			}

			// Close handle
			CloseHandle( h_process );
		};

		bypass_nt_open_file( memory::get_process_id_by_name( proc_list, process ) );
		_logi( "NtOpenFile bypass applied." );
	}

	// Spawning blackbone process variable
	blackbone::Process bb_process {};

	// Attaching blackbone to the process
	bb_process.Attach( memory::get_process_id_by_name( proc_list, process ), PROCESS_ALL_ACCESS );

	_logi( "Injection with %ls is waiting for module %ls in %s.", path_to_dll.filename().c_str(), module_name.c_str(), process.c_str() );

	// Wait for a process module so we can continue with injection.
	auto mod_ready = false;
	while (!mod_ready)
	{
		for (const auto &mod : bb_process.modules().GetAllModules())
		{
			if (mod.first.first == module_name)
			{
				mod_ready = true;
				break;
			}
		}

		if (mod_ready)
			break;

		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	}

	// Resolve PE imports
	const auto mod_callback = []( blackbone::CallbackType type, void *, blackbone::Process &, const blackbone::ModuleData &modInfo )
	{
		std::string user32 = "user32.dll";
		if (type == blackbone::PreCallback)
		{
			if (modInfo.name == std::wstring( user32.begin(), user32.end() ))
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}

		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	// Mapping dll buffer to the process
	if (!bb_process.mmap().MapImage( buffer.size(), buffer.data(), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success())
	{
		_loge( "Failed to inject into %s.", process.c_str() );
		bb_process.Detach();

		return EXIT_FAILURE;
	}

	_logs( "Injected into %s successfully.", process.c_str() );

	// Detach blackbone from the target process.
	bb_process.Detach();

	return EXIT_SUCCESS;
}

bool injector::run()
{
	if (!std::filesystem::exists( vac3_filename ))
	{
		_loge( "%s not found.", vac3_filename.c_str() );
		return EXIT_FAILURE;
	}

	const auto vac_dll_path = std::filesystem::absolute( vac3_filename );

	if (!std::filesystem::exists( cheat_filename ))
	{
		_loge( "%s not found.", cheat_filename.c_str() );
		return EXIT_FAILURE;
	}

	const auto cheat_dll_path = std::filesystem::absolute( cheat_filename );

	close_processes( {
			"csgo",
			"steam",
			"gameoverlay"
		} );

	const auto steam_path = utils::get_steam_path();
	if (steam_path.empty())
		return EXIT_FAILURE;

	_logi( "Steam path: %s.", steam_path.c_str() );

	// Open steam with console opened.
	PROCESS_INFORMATION pi {};
	if (!memory::open_process( steam_path, { "-console", "-applaunch 730" }, pi ))
	{
		_loge( "Failed to open steam." );

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return EXIT_FAILURE;
	}

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	// Inject vac bypass to steam
	map( "steam", L"tier0_s.dll", vac_dll_path );

	// Inject cheat to csgo
	map( "csgo", L"serverbrowser.dll", cheat_dll_path );

	_logi( "All done!" );
	return EXIT_SUCCESS;
}

void injector::close_processes( std::vector<std::string> processes )
{
	auto proc_list = memory::get_process_list();
	for (const auto &process : processes)
	{
		while (true)
		{
			memory::kill_process( proc_list, process );

			proc_list = memory::get_process_list();
			if (!memory::is_process_open( proc_list, process ))
				break;

			std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
		}
	}
}