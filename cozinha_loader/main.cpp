#include "pch.hpp"
#include "injection.hpp"

INT WINAPI WinMain( _In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	std::atexit( [] { std::this_thread::sleep_for( 10s ); } );

	std::int32_t argc; auto* const argv = CommandLineToArgvW( GetCommandLineW(), &argc );

	if ( !vars::get_global_vars( ) )
	{
		log_err( L"Failed to load config!" );
		return EXIT_FAILURE;
	}

	const std::filesystem::path dll_path = argv[1] ? argv[1] : string::to_unicode( vars::str_dll_name );
	
	if ( !std::filesystem::exists( dll_path ) )
	{
		log_err( L"DLL Not found. Place a DLL file called cheat.dll in the same folder as the loader, or drag'n'drop the dll into the exe." );
		return EXIT_FAILURE;
	}
	
	log_debug( L"DLL Path: %s", std::filesystem::absolute( dll_path ).wstring().data() );
	
	if ( !g_injector->initalize( dll_path ) )
		return EXIT_FAILURE;
	
	log_ok( L"Done." );

	return EXIT_SUCCESS;
}