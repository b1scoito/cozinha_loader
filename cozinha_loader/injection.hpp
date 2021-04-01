#pragma once

using namespace std::chrono_literals;

class injector: public singleton<injector>
{
private:
	bool map( std::string process, std::wstring module_name, std::vector<std::uint8_t> binary_bytes );
	void close_processes( std::vector<std::string> processes );

public:
	std::string cheat_filename = "cheat.dll";

	injector() = default;
	~injector() = default;

	bool call();
};