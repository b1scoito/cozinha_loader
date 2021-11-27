#pragma once

#include <shared_mutex>

enum class msg_type_t: std::uint32_t
{
	LNONE = 0,
	LDEBUG = 9,		/* blue */
	LSUCCESS = 10,	/* green */
	LERROR = 12,	/* red */
	LPROMPT = 11,	/* pink */
	LWARN = 14		/* yellow */
};

inline std::ostream& operator<< ( std::ostream& os, const msg_type_t type )
{
	switch ( type )
	{
		case msg_type_t::LDEBUG:	return os << ".";
		case msg_type_t::LSUCCESS:	return os << "+";
		case msg_type_t::LERROR:	return os << "!";
		case msg_type_t::LPROMPT:	return os << ">";
		case msg_type_t::LWARN:		return os << "*";
		default: return os << "";
	}
}

class logger
{
private:
	std::shared_timed_mutex mutex {};

public:
	logger( std::wstring_view title_name = {} )
	{
		AllocConsole();
		AttachConsole( GetCurrentProcessId() );

		if ( !title_name.empty() )
			SetConsoleTitle( title_name.data() );

		FILE* conin, * conout;

		freopen_s( &conin, "conin$", "r", stdin );
		freopen_s( &conout, "conout$", "w", stdout );
		freopen_s( &conout, "conout$", "w", stderr );
	}

	~logger()
	{
		const auto handle = FindWindow( L"ConsoleWindowClass", nullptr );
		ShowWindow( handle, SW_HIDE );
		FreeConsole();
	}

	template< typename ... arg >
	void print( const msg_type_t type, const std::string_view func, std::wstring_view format, arg ... args )
	{
		static auto* h_console = GetStdHandle( STD_OUTPUT_HANDLE );
		std::unique_lock<decltype( mutex )> lock( mutex );

		const auto formatted = string::format( format, args ... );

		if ( type != msg_type_t::LNONE )
		{
			SetConsoleTextAttribute( h_console, static_cast<WORD>( type ) );
			std::wcout << L"[";
			std::cout << type;
			std::wcout << L"] ";

#ifdef _DEBUG
			SetConsoleTextAttribute( h_console, 15 /* white */ );
			std::wcout << L"[ ";
			SetConsoleTextAttribute( h_console, static_cast<WORD>( type ) );
			std::cout << func;
			SetConsoleTextAttribute( h_console, 15 /* white */ );
			std::wcout << L" ] ";
#endif
		}

		if ( type == msg_type_t::LDEBUG )
			SetConsoleTextAttribute( h_console, 8 /* gray */ );
		else
			SetConsoleTextAttribute( h_console, 15 /* white */ );

		if ( type == msg_type_t::LPROMPT )
			std::wcout << formatted;
		else
			std::wcout << formatted << L"\n";
	}
};

inline auto g_logger = logger( L"> cozinha loader" );
#define log_debug(...)	g_logger.print( msg_type_t::LDEBUG, __FUNCTION__, __VA_ARGS__ )
#define log_ok(...)		g_logger.print( msg_type_t::LSUCCESS, __FUNCTION__, __VA_ARGS__ )
#define log_err(...)	g_logger.print( msg_type_t::LERROR, __FUNCTION__, __VA_ARGS__ )
#define log_prompt(...) g_logger.print( msg_type_t::LPROMPT, __FUNCTION__, __VA_ARGS__ )
#define log_warn(...)	g_logger.print( msg_type_t::LWARN, __FUNCTION__, __VA_ARGS__ )
#define log_raw(...)	g_logger.print( msg_type_t::LNONE, __FUNCTION__, __VA_ARGS__ )