#include "pch.hpp"

// WinMain definition as-is.
int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	// Sleep for 5 seconds before exiting.
	std::atexit( [] { std::this_thread::sleep_for( std::chrono::seconds( 5 ) ); } );
	int argc {}; LPWSTR *argv = CommandLineToArgvW( GetCommandLineW(), &argc );

	// if an argument is passed inject the target dll, so we can drag and drop the dll to the exe.
	if (argv[1] != nullptr)
		g_inj->cheat_filename = utils::string::wstring_to_string( argv[1] );

	if (!g_inj->run())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}