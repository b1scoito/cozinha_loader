#include "pch.hpp"

INT WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	std::atexit( [] { std::this_thread::sleep_for( 10s ); } );

	int argc; const auto* argv = CommandLineToArgvW( GetCommandLineW(), &argc );

#ifndef _DEBUG
	const std::filesystem::path dll_path = argv[1] ? string::to_utf8( argv[1] ) : "cheat.dll";
#else
	const std::filesystem::path dll_path = "D:\\cheat.dll";
#endif

	if ( !std::filesystem::exists( dll_path ) )
	{
		log_err( "DLL not found! Place a dll file called cheat.dll in the same folder as the loader, or drag'n'drop the dll into the exe." );
		return EXIT_FAILURE;
	}

	log_debug( "DLL path - [ %s ]", std::filesystem::absolute( dll_path ).string().c_str() );

	std::string str_proc_name;
	log_prompt( "Target process name -> " );

	std::cin >> str_proc_name;
	std::cin.clear();

	const auto start = std::chrono::high_resolution_clock::now();

	// ~ this function will inject vac3 bypass and the dll on the target process
	if ( !g_injector->init( str_proc_name, dll_path ) )
		return EXIT_FAILURE;

	const auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed( end - start );

	log_debug( "Done in %.3fms.", elapsed.count() );

	return EXIT_SUCCESS;
}