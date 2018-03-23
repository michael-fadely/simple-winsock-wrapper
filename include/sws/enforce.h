#pragma once

#include <stdexcept>

namespace sws
{
	#define enforce(CONDITION, MESSAGE) if (!(CONDITION)) throw std::logic_error((MESSAGE))
	#define basic_enforce(CONDITION) enforce(CONDITION, "Enforcement failed.")
}
