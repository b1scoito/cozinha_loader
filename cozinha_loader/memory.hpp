#pragma once

namespace memory
{
	inline bool open_process( std::string path, std::vector<std::string> arguments, PROCESS_INFORMATION& pi )
	{
		STARTUPINFO si;
		{
			ZeroMemory( &si, sizeof( si ) );
			si.cb = sizeof( si );
		}

		ZeroMemory( &pi, sizeof( pi ) );

		std::string str_path{};
		str_path += path;

		for ( const auto& arg : arguments )
			str_path += (" " + arg);

		return CreateProcess( nullptr, str_path.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi );
	}

	inline bool is_process_open( const std::vector<std::pair<std::uint32_t, std::string>>& vec_processes, std::string_view str_proc )
	{
		if ( vec_processes.empty() )
			return {};

		if ( str_proc.empty() )
			return {};

		auto target = utils::string::to_lower( str_proc.data() );
		for ( const auto& ctx : vec_processes )
		{
			auto ep = utils::string::to_lower( ctx.second );
			if ( target.find( ".exe" ) == std::string::npos )
			{
				if ( ep.find( target ) == std::string::npos )
					continue;
			}
			else
			{
				if ( ep != target )
					continue;
			}

			const auto h_process = OpenProcess( PROCESS_VM_READ, false, ctx.first );
			if ( h_process != nullptr )
			{
				CloseHandle( h_process );
				return true;
			}
		}

		return {};
	}

	inline bool kill_process( const std::vector<std::pair<std::uint32_t, std::string>>& vec_processes, std::string_view str_proc )
	{
		if ( vec_processes.empty() )
			return {};

		if ( str_proc.empty() )
			return {};

		auto executed = false;
		auto target = utils::string::to_lower( str_proc.data() );
		for ( const auto& ctx : vec_processes )
		{
			auto ep = utils::string::to_lower( ctx.second );
			if ( target.find( ".exe" ) == std::string::npos )
			{
				if ( ep.find( target ) == std::string::npos )
					continue;
			}
			else
			{
				if ( ep != target )
					continue;
			}

			const auto h_process = OpenProcess( PROCESS_TERMINATE, false, ctx.first );
			if ( h_process != nullptr )
			{
				TerminateProcess( h_process, 9 );
				CloseHandle( h_process );

				executed = true;
			}
		}

		return executed;
	}

	inline std::uint32_t get_process_id_by_name( const std::vector<std::pair<std::uint32_t, std::string>>& vec_processes, std::string_view str_proc )
	{
		if ( vec_processes.empty() )
			return {};

		if ( str_proc.empty() )
			return {};

		auto target = utils::string::to_lower( str_proc.data() );
		for ( const auto& ctx : vec_processes )
		{
			auto ep = utils::string::to_lower( ctx.second );
			if ( target.find( ".exe" ) == std::string::npos )
			{
				if ( ep.find( target ) == std::string::npos )
					continue;
			}
			else
			{
				if ( ep != target )
					continue;
			}

			return ctx.first;
		}

		return {};
	}

	inline std::vector<std::pair<std::uint32_t, std::string>> get_process_list()
	{
		std::vector<std::pair<std::uint32_t, std::string>> vec_list{};

		const auto h_handle = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );

		PROCESSENTRY32 m_entry{};
		m_entry.dwSize = sizeof( m_entry );

		if ( !Process32First( h_handle, &m_entry ) )
			return {};

		while ( Process32Next( h_handle, &m_entry ) )
			vec_list.emplace_back( m_entry.th32ProcessID, m_entry.szExeFile );

		CloseHandle( h_handle );

		return vec_list;
	}
}