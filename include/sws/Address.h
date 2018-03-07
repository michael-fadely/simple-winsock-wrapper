#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Ws2tcpip.h>

#include "typedefs.h"
#include "SocketError.h"

namespace sws
{
	enum class AddressFamily
	{
		none,
		inet,
		inet6,
		any
	};

	class AddressResolveException : public std::exception
	{
		std::basic_string<char> message {};

	public:
		AddressResolveException(const char* address, port_t port, SocketError error);
		AddressResolveException(const char* address, const char* service, SocketError error);
		char const* what() const override;

		const SocketError native_error;
	};

	class Address
	{
	public:
		std::string   address {};
		port_t        port   = 0;
		AddressFamily family = AddressFamily::none;

		Address() = default;
		Address(Address&) = default;
		Address(Address&& other) noexcept;
		~Address() = default;

		Address& operator=(const Address& other) = default;
		Address& operator=(Address&& other) noexcept;

		bool operator==(const Address& other) const;
		bool operator!=(const Address& other) const;

		static Address from_native(const sockaddr* address);

		static std::vector<Address> get_addresses(const char* host, const char* service, AddressFamily family = AddressFamily::any);
		static std::vector<Address> get_addresses(const char* host, port_t port = 0, AddressFamily family = AddressFamily::any);

		Address get_name() const;

		static size_t native_size(uint16_t family);
		sockaddr_storage to_native() const;

		size_t native_size() const;
	};
}
