#include "pch.hpp"
#include "injection.hpp"

bool injector::map( std::string process, std::wstring module_name, std::vector<std::uint8_t> binary_bytes ) {
	log_debug( "Waiting for [ %s ] to be opened!", process.c_str() );
	// Wait for process to be opened
	auto process_list = memory::get_process_list();
	while (true) {
		std::this_thread::sleep_for( 500ms );

		process_list = memory::get_process_list();
		if (memory::is_process_open( process_list, process ))
			break;
	}

	if (process.find( "csgo" ) != std::string::npos) {
		// Bypassing injection block by csgo (-allow_third_party_software) the easiest way.
		const auto bypass_nt_open_file = []( DWORD pid ) {
			const auto h_process = OpenProcess( PROCESS_ALL_ACCESS, false, pid );

			LPVOID nt_open_file_address {};
			if (nt_open_file_address) {
				// Get NtOpenFile memory address
				nt_open_file_address = GetProcAddress( LoadLibrary( "ntdll" ), "NtOpenFile" );
				char bytes[5];
				// Copy 5 bytes to NtOpenFile procedure address
				std::memcpy( bytes, nt_open_file_address, 5 );
				// Write it to memory.
				WriteProcessMemory( h_process, nt_open_file_address, bytes, 5, nullptr );
			}

			CloseHandle( h_process );
		};

		bypass_nt_open_file( memory::get_process_id_by_name( process_list, process ) );
	}

	// Spawning blackbone process variable
	blackbone::Process bb_process {};

	// Attaching blackbone to the process
	bb_process.Attach( memory::get_process_id_by_name( process_list, process ), PROCESS_ALL_ACCESS );
	log_debug( "Injecting into [ %s ] waiting for [ %ls ]", process.c_str(), module_name.c_str() );

	// Wait for a process module so we can continue with injection.
	auto mod_ready = false;
	while (!mod_ready) {
		std::this_thread::sleep_for( 500ms );

		for (const auto &mod : bb_process.modules().GetAllModules()) {
			if (mod.first.first == module_name) {
				mod_ready = true;
				break;
			}
		}

		if (mod_ready)
			break;
	}

	// Resolve PE imports
	const auto mod_callback = []( blackbone::CallbackType type, void *, blackbone::Process &, const blackbone::ModuleData &modInfo ) {
		std::string user32 = "user32.dll";
		if (type == blackbone::PreCallback) {
			if (modInfo.name == std::wstring( user32.begin(), user32.end() ))
				return blackbone::LoadData( blackbone::MT_Native, blackbone::Ldr_Ignore );
		}
		return blackbone::LoadData( blackbone::MT_Default, blackbone::Ldr_Ignore );
	};

	// Mapping dll bytes to the process
	if (!bb_process.mmap().MapImage( binary_bytes.size(), binary_bytes.data(), false, blackbone::WipeHeader, mod_callback, nullptr, nullptr ).success()) {
		log_err( "Failed to inject into [ %s ]! [ blackbone_mapping_failed ]", process.c_str() );
		bb_process.Detach();

		return EXIT_FAILURE;
	}

	// Detach blackbone from the target process.
	bb_process.Detach();

	log_ok( "Injected into [ %s ] successfully!", process.c_str() );
	return EXIT_SUCCESS;
}

bool injector::run() {
	if (!std::filesystem::exists( cheat_filename )) {
		log_err( "[ %s ] not found! Try dragging and dropping the dll into the loader or putting a cheat dll called cheat.dll in the same folder as the loader.", cheat_filename.c_str() );
		return EXIT_FAILURE;
	}

	close_processes( { "csgo.exe", "steam.exe" } );

	const auto steam_path = utils::other::get_steam_path();
	if (steam_path.empty()) {
		log_err( "Failed to retrieve steam path!" );
		return EXIT_FAILURE;
	}

	log_info( "Steam path [ %s ]", steam_path.c_str() );

	// Open steam with console and game opened.
	PROCESS_INFORMATION pi {};
	if (!memory::open_process( steam_path, { "-console", "-applaunch 730" }, pi )) {
		log_err( "Failed to open steam! [ open_process_failed ]" );

		if (pi.hProcess && pi.hThread) {
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}

		return EXIT_FAILURE;
	}

	if (pi.hProcess && pi.hThread) {
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}

	std::vector<std::uint8_t> cheat {};

	// Reading file and writing it to a variable
	if (!utils::other::read_file_to_memory( std::filesystem::absolute( cheat_filename ).string(), &cheat )) {
		log_err( "Failed to write dll to memory! [ read_file_to_memory ]" );
		return EXIT_FAILURE;
	}

	// Inject vac bypass to steam
	map( "steam.exe", L"tier0_s.dll", vac3_data );

	// Then inject cheat to csgo
	map( "csgo.exe", L"serverbrowser.dll", cheat );

	log_ok( "All done!" );
	return EXIT_SUCCESS;
}

void injector::close_processes( std::vector<std::string> processes ) {
	auto process_list = memory::get_process_list();
	for (const auto &process : processes) {
		log_debug( "Trying to close [ %s ]", process.c_str() );
		while (true) {
			std::this_thread::sleep_for( 500ms );

			memory::kill_process( process_list, process );

			process_list = memory::get_process_list();
			if (!memory::is_process_open( process_list, process ))
				break;
		}
	}
}