#include "pch.hpp"

INT WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	std::atexit( [] { std::this_thread::sleep_for( 10s ); } );

	int argc; auto* const argv = CommandLineToArgvW( GetCommandLineW(), &argc );

#ifndef _DEBUG
	const std::filesystem::path dll_path = argv[1] ? argv[1] : L"cheat.dll";
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

	// this function will inject vac3 bypass on steam and the dll on the target process
	if ( !g_injector->init( str_proc_name, dll_path ) )
		return EXIT_FAILURE;

	log_ok( "Done." );

	return EXIT_SUCCESS;
}