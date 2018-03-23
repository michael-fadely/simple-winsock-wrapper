#pragma once

#include <functional>

namespace sws
{
	void enforce(bool condition, const char* message = "Enforcement failed.");
	void enforce(bool condition, const std::string& message);
	void enforce(bool condition, const std::function<std::string()>& fn);
}
