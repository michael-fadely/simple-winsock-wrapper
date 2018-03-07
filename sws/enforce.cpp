#include <stdexcept>
#include <sws/enforce.h>

namespace sws
{
	void enforce(bool condition, const char* message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void enforce(bool condition, const std::string& message)
	{
		enforce(condition, message.c_str());
	}

	void enforce(bool condition, std::function<std::string()> fn)
	{
		if (!condition)
		{
			std::string str = fn();
			throw std::runtime_error(str);
		}
	}
}
