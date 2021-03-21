#pragma once

class injector
{
private:
	bool map( std::string process, std::wstring module_name, std::filesystem::path path_to_dll );
	void close_processes( std::vector<std::string> processes );
public:
	std::string vac3_filename = "vac3_b.dll";
	std::string cheat_filename = "cheat.dll";

	injector() = default;
	~injector() = default;
	bool run();
};

inline auto g_inj = std::make_unique<injector>();