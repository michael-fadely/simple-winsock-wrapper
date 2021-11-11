#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../include/sws/SocketException.h"

namespace sws
{
	SocketException::SocketException(sws::SocketError error)
		: native_error(error)
	{
		append_error_string();
	}

	SocketException::SocketException(const char* msg, sws::SocketError error)
		: message(msg),
		  native_error(error)
	{
		append_error_string();
	}

	SocketException::SocketException(std::string msg, sws::SocketError error)
		: message(std::move(msg)),
		  native_error(error)
	{
		append_error_string();
	}

	void SocketException::append_error_string()
	{
	#if defined(_WIN32) && _WIN32 == 1
		char* buffer = nullptr;

		const DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		                                  nullptr,
		                                  static_cast<DWORD>(native_error),
		                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		                                  reinterpret_cast<LPSTR>(&buffer),
		                                  0,
		                                  nullptr);

		if (size > 0 && buffer != nullptr)
		{
			message.append("\n");
			message.append(std::string(buffer, strnlen(buffer, size)));
		}

		if (buffer != nullptr)
		{
			LocalFree(buffer);
		}
	#else
		#error Platform not yet supported
	#endif
	}

	char const* SocketException::what() const
	{
		return message.c_str();
	}
}
