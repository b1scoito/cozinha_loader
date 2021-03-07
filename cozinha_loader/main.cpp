#include "pch.h"

// WinMain definition as-is.
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// Sleep for 5 seconds before exiting.
	std::atexit( [ ]( )
		{
			std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
		}
	);

	g_injection->close_processes( { "csgo.exe", "SteamService.exe", "steam.exe", "steamwebhelper.exe" } );

	if ( !g_injection->setup( ) )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}