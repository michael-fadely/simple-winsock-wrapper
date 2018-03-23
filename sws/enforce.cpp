#include <stdexcept>
#include "../include/sws/enforce.h"

namespace sws
{
	void enforce(bool condition, const char* message)
	{
		if (!condition)
		{
			throw std::logic_error(message);
		}
	}

	void enforce(bool condition, const std::string& message)
	{
		enforce(condition, message.c_str());
	}

	void enforce(bool condition, const std::function<std::string()>& fn)
	{
		if (!condition)
		{
			throw std::logic_error(fn());
		}
	}
}
