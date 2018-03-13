#include "../include/sws/Address.h"

#include <sstream>
#include "../include/sws/Socket.h"

namespace sws
{
	AddressResolveException::AddressResolveException(const char* address, port_t port, SocketError error)
		: native_error(error)
	{
		std::stringstream message;

		message << "Failed to resolve host: " << address << ':' << port
			<< " (error code " << static_cast<int>(error) << ")";

		this->message = message.str();
	}

	AddressResolveException::AddressResolveException(const char* address, const char* service, SocketError error)
		: native_error(error)
	{
		std::stringstream message;

		message << "Failed to resolve host: " << address << ':' << service
			<< " (error code " << static_cast<int>(error) << ")";

		this->message = message.str();
	}

	char const* AddressResolveException::what() const
	{
		return message.c_str();
	}

	Address::Address(Address&& other) noexcept
	{
		*this = std::move(other);
	}

	Address& Address::operator=(Address&& other) noexcept
	{
		address = std::move(other.address);
		port    = other.port;
		family  = other.family;

		return *this;
	}

	bool Address::operator==(const Address& other) const
	{
		return port == other.port &&
			   family == other.family &&
			   address == other.address;
	}

	bool Address::operator!=(const Address& other) const
	{
		return !(*this == other);
	}

	Address Address::from_native(const sockaddr* address)
	{
		Address result;

		switch (address->sa_family)
		{
			case AF_INET:
			{
				std::array<char, INET_ADDRSTRLEN> buffer {};
				auto v4_addr = reinterpret_cast<const sockaddr_in*>(address);

				const char* str = inet_ntop(address->sa_family, &v4_addr->sin_addr, buffer.data(), buffer.size());

				if (str == nullptr)
				{
					throw std::runtime_error("inet_ntop failed");
				}

				result.address = str;
				result.port    = ntohs(v4_addr->sin_port);
				result.family  = AddressFamily::inet;
				break;
			}

			case AF_INET6:
			{
				std::array<char, INET6_ADDRSTRLEN> buffer {};
				auto v6_addr = reinterpret_cast<const sockaddr_in6*>(address);

				const char* str = inet_ntop(address->sa_family, &v6_addr->sin6_addr, buffer.data(), buffer.size());

				if (str == nullptr)
				{
					throw std::runtime_error("inet_ntop failed");
				}

				result.address = str;
				result.port    = ntohs(v6_addr->sin6_port);
				result.family  = AddressFamily::inet6;
				break;
			}

			default:
				throw std::runtime_error("unsupported address family");
		}

		return result;
	}

	std::vector<Address> Address::get_addresses(const char* host, const char* service, AddressFamily family)
	{
		std::vector<Address> addresses;

		addrinfo hints {};

		hints.ai_flags = AI_ALL;

		switch (family)
		{
			case AddressFamily::none:
				throw std::runtime_error("invalid address family");

			case AddressFamily::inet:
				hints.ai_family = AF_INET;
				break;

			case AddressFamily::inet6:
				hints.ai_family = AF_INET6;
				break;

			case AddressFamily::any:
				hints.ai_family = AF_UNSPEC;
				break;

			default:
				throw std::runtime_error("unsupported address family");
		}


		addrinfo* result = nullptr;
		auto error = static_cast<SocketError>(getaddrinfo(host, service, &hints, &result));

		if (error != SocketError::none)
		{
			freeaddrinfo(result);
			throw AddressResolveException(host, static_cast<port_t>(0), error);
		}

		for (auto ptr = result; ptr != nullptr; ptr = ptr->ai_next)
		{
			try
			{
				addresses.push_back(from_native(ptr->ai_addr));
			}
			catch (std::exception&)
			{
				freeaddrinfo(result);
				throw;
			}
		}

		freeaddrinfo(result);
		return addresses;
	}

	std::vector<Address> Address::get_addresses(const char* host, port_t port, AddressFamily family)
	{
		return get_addresses(host, !port ? nullptr : std::to_string(port).c_str(), family);
	}

	Address Address::get_name() const
	{
		std::array<char, NI_MAXHOST> node {};

		const auto native = to_native();

		int result = getnameinfo(reinterpret_cast<const sockaddr*>(&native), static_cast<socklen_t>(native_size()),
								 node.data(), static_cast<DWORD>(node.size()), nullptr, 0, 0);

		if (result != 0)
		{
			throw SocketException("getnameinfo failed", Socket::get_native_error());
		}

		Address address;

		address.address = std::string(node.data(), 0, strnlen(node.data(), node.size()));
		address.port    = port;
		address.family  = family;

		return address;
	}

	size_t Address::native_size(ADDRESS_FAMILY family)
	{
		switch (family)
		{
			case AF_INET:
				return sizeof(sockaddr_in);

			case AF_INET6:
				return sizeof(sockaddr_in6);

			default:
				return 0;
		}
	}

	sockaddr_storage Address::to_native() const
	{
		sockaddr_storage result {};

		switch (family)
		{
			case AddressFamily::inet:
			{
				auto ptr   = reinterpret_cast<sockaddr_in*>(&result);
				auto error = inet_pton(AF_INET, address.c_str(), &ptr->sin_addr);

				if (error == -1)
				{
					throw SocketException("inet_pton failed", Socket::get_native_error());
				}

				if (!error)
				{
					throw std::runtime_error("invalid string passed to inet_pton");
				}

				ptr->sin_family = AF_INET;
				ptr->sin_port   = htons(port);
				break;
			}

			case AddressFamily::inet6:
			{
				auto ptr   = reinterpret_cast<sockaddr_in6*>(&result);
				auto error = inet_pton(AF_INET6, address.c_str(), &ptr->sin6_addr);

				if (error == -1)
				{
					throw SocketException("inet_pton failed", Socket::get_native_error());
				}

				if (!error)
				{
					throw std::runtime_error("invalid string passed to inet_pton");
				}

				ptr->sin6_family = AF_INET6;
				ptr->sin6_port   = htons(port);

				// TODO: remaining fields?
				break;
			}

			default:
				throw std::runtime_error("invalid address family");
		}

		return result;
	}

	size_t Address::native_size() const
	{
		switch (family)
		{
			case AddressFamily::inet:
				return sizeof(sockaddr_in);

			case AddressFamily::inet6:
				return sizeof(sockaddr_in6);

			default:
				throw std::runtime_error("invalid address family");
		}
	}
}
