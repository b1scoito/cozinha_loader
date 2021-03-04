#include "pch.h"

// WinMain definition as-is.
//
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	_log( LINFO, "Starting." );

	// sleep for 5 seconds before exiting.
	//
	std::atexit( [ ]( ) { std::this_thread::sleep_for( std::chrono::seconds( 5 ) ); } );

	std::thread close_processes( [ ]( )
		{
			g_injection->close_processes( { "steamservice.exe", "steam.exe", "steamwebhelper.exe", "csgo.exe" } );
		}
	);

	std::thread injection_setup( [ ]( )
		{
			g_injection->setup( );
		}
	);

	close_processes.join( );
	injection_setup.join( );

	return EXIT_SUCCESS;
}