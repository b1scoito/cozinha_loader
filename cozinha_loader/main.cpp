#include "pch.hpp"

INT WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	std::atexit( [] { std::this_thread::sleep_for( 10s ); } );

	int argc {}; const auto* argv = CommandLineToArgvW( GetCommandLineW(), &argc );

#ifndef _DEBUG
	std::filesystem::path cheat_name = argv[1] ? string::to_utf8( argv[1] ) : "cheat.dll";
#else
	std::filesystem::path cheat_name = "C:\\Users\\biscuit\\Desktop\\owo\\cozinha_loader\\cozinha_loader\\out\\debug\\celestial_c.dll";
#endif

	if ( !std::filesystem::exists( cheat_name ) )
	{
		log_err( "DLL not found! Place a dll file called cheat.dll in the same folder as the loader, or drag'n'drop the dll into the exe." );
		return EXIT_FAILURE;
	}

	log_debug( "DLL path - [ %s ]", std::filesystem::absolute( cheat_name ).string().c_str() );

	std::string proc_name;
	log_prompt( "Target process name: " );

	std::cin >> proc_name;
	std::cin.clear();

	// ~ this function will inject vac3 bypass and the cheat dll on the target process
	if ( !g_injector->init( proc_name, cheat_name ) )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}