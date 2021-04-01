#include "pch.hpp"
#include "injection.hpp"

bool injector::map( std::string process, std::wstring module_name, std::vector<std::uint8_t> binary_bytes )
{
	log_debug( "Waiting for [ %s ] to be opened...", process.c_str() );

	// Wait for process to be opened
	auto process_list = memory::get_process_list();
	while (true)
	{
		std::this_thread::sleep_for( 500ms );

		process_list = memory::get_process_list();
		if (memory::is_process_open( process_list, process ))
			break;
	}

	if (process.find( "csgo" ) != std::string::npos)
	{
		// Bypassing injection block by csgo (-allow_third_party_software) the easiest way.
		const auto bypass_nt_open_file = []( DWORD pid )
		{
			const auto h_process = OpenProcess( PROCESS_ALL_ACCESS, false, pid );
			LPVOID nt_open_file = GetProcAddress( LoadLibrary( "ntdll" ), "NtOpenFile" );

			if (nt_open_file)
			{
				char original_bytes[5];
				// Copy 5 bytes to NtOpenFile procedure address
				std::memcpy( original_bytes, nt_open_file, 5 );
				// Write it to memory.
				WriteProcessMemory( h_process, nt_open_file, original_bytes, 5, nullptr );
			}

			CloseHandle( h_process );
		};

		bypass_nt_open_file( memory::get_process_id_by_name( process_list, process ) );
	}

	blackbone::Process bb_process {};

	bb_process.Attach( memory::get_process_id_by_name( process_list, process ), PROCESS_ALL_ACCESS );

	log_debug( "Injecting into [ %s ] waiting for [ %ls ]...", process.c_str(), module_name.c_str() );

	// Wait for a process module so we can continue with injection.
	auto mod_ready = false;
	while (!mod_ready)
	{
		std::this_thread::sleep_for( 500ms );

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

	// Mapping dll bytes to the process
	if (!bb_process.mmap().MapImage( binary_bytes.size(), binary_bytes.data(), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success())
	{
		log_err( "Failed to inject into [ %s ]! [ blackbone_mapping_failed ]", process.c_str() );
		bb_process.Detach();

		return false;
	}

	// Free memory and detach from process
	bb_process.Detach();

	log_ok( "Injected into [ %s ] successfully!", process.c_str() );
	return true;
}

bool injector::call()
{
	if (!std::filesystem::exists( cheat_filename ))
	{
		log_err( "[ %s ] not found! Try dragging and dropping the dll into the loader or putting a cheat dll called cheat.dll in the same folder as the loader.", cheat_filename.c_str() );
		return false;
	}

	// Closing processes
	close_processes( { "csgo.exe", "steam.exe" } );

	const auto steam_path = utils::other::get_steam_path();
	if (steam_path.empty())
	{
		log_err( "Failed to retrieve steam path!" );
		return false;
	}

	log_info( "Steam path [ %s ], Opening steam...", steam_path.c_str() );

	PROCESS_INFORMATION pi {};
	if (!memory::open_process( steam_path, { "-console", "-applaunch 730" }, pi ))
	{
		log_err( "Failed to open steam! [ open_process_failed ]" );

		utils::other::safe_close_handle( pi.hProcess );
		utils::other::safe_close_handle( pi.hThread );

		return false;
	}

	utils::other::safe_close_handle( pi.hProcess );
	utils::other::safe_close_handle( pi.hThread );

	std::vector<std::uint8_t> cheat {};

	// Reading file and writing it to a variable
	if (!utils::other::read_file_to_memory( std::filesystem::absolute( cheat_filename ).string(), &cheat ))
	{
		log_err( "Failed to write dll to memory! [ read_file_to_memory ]" );
		return false;
	}

	// Inject vac bypass to steam
	if (!map( "steam.exe", L"tier0_s.dll", vac3_data ))
	{
		log_err( "Steam memory mapping failure!" );
		return false;
	}

	// Then inject cheat to csgo
	if (!map( "csgo.exe", L"serverbrowser.dll", cheat ))
	{
		log_err( "Cheat memory mapping failure!" );
		return false;
	}

	log_ok( "All done!" );
	return true;
}

void injector::close_processes( std::vector<std::string> processes )
{
	auto process_list = memory::get_process_list();
	for (const auto &process : processes)
	{
		while (true)
		{
			std::this_thread::sleep_for( 500ms );

			memory::kill_process( process_list, process );

			process_list = memory::get_process_list();
			if (!memory::is_process_open( process_list, process ))
				break;
		}
	}
}