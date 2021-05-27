#include "pch.hpp"

auto on_exit() -> void;

int WINAPI WinMain( _In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd )
{
	std::atexit( on_exit );

	// ~ get arguments
	//
	int argc; auto* argv = CommandLineToArgvW( GetCommandLineW(), &argc );

	// ~ if an argument is passed inject the target dll, so we can drag and drop the dll to the exe.
	//
	if ( argv[1] ) utils::vars::cheat_filename = utils::string::wstring_to_string( argv[1] );

	std::string proc_name;
	std::cout << "Target process name: ";
	std::cin >> proc_name;

	if ( !g_injector.call( proc_name ) )
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

auto on_exit() -> void
{
	std::this_thread::sleep_for( 10s );
}