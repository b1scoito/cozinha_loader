#pragma once

#include "inih/inih.hpp"

namespace vars
{
	inline std::string str_process_name { "csgo.exe" };
	inline std::string str_dll_name { "cheat.dll" };
	inline std::string str_steam_mod_name { "tier0_s.dll" };
	inline std::string str_process_mod_name { "serverbrowser.dll" };

	inline bool b_inject_vac_bypass { true };
	inline bool b_open_game_automatically { false };

	inline bool get_global_vars()
	{
		const auto cur_path = std::filesystem::path( util::get_executable_path().data() ).parent_path();
		std::filesystem::current_path( cur_path );

		if ( !std::filesystem::exists( "cozinha_loader.ini" ) )
		{
			std::ofstream out( "cozinha_loader.ini" );

			// Wow...
			std::stringstream ss;
			ss << "[launch_options]" << std::endl;
			ss << "process_name = csgo.exe" << std::endl;
			ss << "dll_name = cheat.dll ; Remembering that a dll can still be drag'n'dropped into the loader, this is here to facilitate faster injections" << std::endl;
			ss << "inject_vac_bypass = true" << std::endl;
			ss << "open_game_automatically = false" << std::endl;
			ss << std::endl;
			ss << "[advanced]" << std::endl;
			ss << "steam_mod_name = tier0_s.dll" << std::endl;
			ss << "process_mod_name = serverbrowser.dll" << std::endl;
			out << ss.str();

			out.close();
		}

		INIReader reader( "cozinha_loader.ini" );

		if ( reader.ParseError() < 0 )
			return false;

		// launch_options
		if ( reader.HasSection( "launch_options" ) )
		{
			if (reader.HasValue( "launch_options", "process_name" ) )
				str_process_name = reader.GetString( "launch_options", "process_name", "csgo.exe" );

			if (reader.HasValue( "launch_options", "dll_name" ) )
				str_dll_name = reader.GetString( "launch_options", "dll_name", "cheat.dll" );

			if ( reader.HasValue( "launch_options", "inject_vac_bypass" ) )
				b_inject_vac_bypass = reader.GetBoolean( "launch_options", "inject_vac_bypass", true );

			if ( reader.HasValue( "launch_options", "open_game_automatically" ) )
				b_open_game_automatically = reader.GetBoolean( "launch_options", "open_game_automatically", false );
		}

		// advanced
		if ( reader.HasSection( "advanced" ) )
		{
			if ( reader.HasValue( "advanced", "steam_mod_name" ) )
				str_steam_mod_name = reader.GetString( "advanced", "steam_mod_name", "tier0_s.dll" );

			if ( reader.HasValue( "advanced", "process_mod_name" ) )
				str_process_mod_name = reader.GetString( "advanced", "process_mod_name", "serverbrowser.dll" );
		}

		return true;
	}
}