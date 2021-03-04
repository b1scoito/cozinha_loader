#pragma once

namespace mem
{
	inline bool open_process( std::string path, std::vector<std::string> arguments, PROCESS_INFORMATION &process )
	{
		STARTUPINFO si;
		{
			ZeroMemory( &si, sizeof( si ) );
			si.cb = sizeof( si );
		}

		std::string wstr {};
		wstr += path;

		for ( const auto &i : arguments )
			wstr += " " + i;

		return CreateProcess( nullptr, const_cast< char * >( wstr.c_str( ) ), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &process );
	}

	inline bool is_process_open( const std::string &process )
	{
		if ( process.empty( ) )
			return false;

		auto file = process;

		if ( file.find_last_of( "." ) != std::string::npos )
			file.erase( file.find_last_of( "." ), std::string::npos );

		file += ".exe";

		const auto handle = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );

		PROCESSENTRY32 entry {}; entry.dwSize = sizeof( entry );

		if ( !Process32First( handle, &entry ) )
			return false;

		while ( Process32Next( handle, &entry ) )
		{
			if ( u::to_lower( entry.szExeFile ).compare( u::to_lower( file ) ) == 0 )
			{
				const auto process = OpenProcess( PROCESS_VM_READ, false, entry.th32ProcessID );

				if ( process != nullptr )
					CloseHandle( process );

				CloseHandle( handle );

				return true;
			}
		}

		return false;
	}

	inline bool kill_process( const std::string &process )
	{
		if ( process.empty( ) )
			return false;

		auto file = process;

		if ( file.find_last_of( "." ) != std::string::npos )
			file.erase( file.find_last_of( "." ), std::string::npos );

		file += ".exe";

		const auto handle = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );

		PROCESSENTRY32 entry {}; entry.dwSize = sizeof( entry );

		if ( !Process32First( handle, &entry ) )
			return false;

		while ( Process32Next( handle, &entry ) )
		{
			if ( u::to_lower( entry.szExeFile ).compare( u::to_lower( file ) ) == 0 )
			{
				const auto process = OpenProcess( PROCESS_TERMINATE, false, entry.th32ProcessID );

				if ( process != nullptr )
				{
					TerminateProcess( process, 9 );
					CloseHandle( process );
				}

				CloseHandle( handle );

				return true;
			}
		}

		return false;
	}

	inline int get_process_id_by_name( const std::string &process )
	{
		if ( process.empty( ) )
			return false;

		auto file = process;

		if ( file.find_last_of( "." ) != std::string::npos )
			file.erase( file.find_last_of( "." ), std::string::npos );

		file += ".exe";

		const auto handle = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );

		PROCESSENTRY32 entry {}; entry.dwSize = sizeof( entry );

		if ( !Process32First( handle, &entry ) )
			return 0;

		while ( Process32Next( handle, &entry ) )
		{
			if ( u::to_lower( entry.szExeFile ).compare( u::to_lower( file ) ) == 0 )
			{
				CloseHandle( handle );
				return entry.th32ProcessID;
			}
		}

		return 0;
	}
}