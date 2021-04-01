#include "pch.hpp"

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	// Sleep for 5 seconds before exiting.
	std::atexit( [] { std::this_thread::sleep_for( std::chrono::seconds( 5 ) ); } );

	// Get arguments
	int argc {}; LPWSTR *argv = CommandLineToArgvW( GetCommandLineW(), &argc );

	// If an argument is passed inject the target dll, so we can drag and drop the dll to the exe.
	if (argv[1]) injector::get().cheat_filename = utils::string::wstring_to_string( argv[1] );

	if (!injector::get().call())
	{
		log_err( "Injection failed!" );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}