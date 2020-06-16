#include "../include/sws/Address.h"

#include <sstream>
#include <utility>
#include "../include/sws/Socket.h"
#include "../include/sws/SocketException.h"
#include "hash_combine.h"

// TODO: address parser
// TODO: numeric address storage class

namespace sws
{
	AddressResolveException::AddressResolveException(const char* address, port_t port, SocketError error)
		: SocketException(error)
	{
		std::stringstream message;

		message << "Failed to resolve host: "
			<< (address == nullptr ? "[any]" : address) << ':' << port
			<< " (error code " << static_cast<int>(error) << ")"
			<< this->message;

		this->message = message.str();
	}

	AddressResolveException::AddressResolveException(const char* address, const char* service, SocketError error)
		: SocketException(error)
	{
		std::stringstream message;

		message << "Failed to resolve host: " << (address == nullptr ? "[any]" : address)
			<< ':' << (service == nullptr ? "[any]" : service)
			<< " (error code " << static_cast<int>(error) << ")"
			<< this->message;

		this->message = message.str();
	}

	Address::Address(std::string address, port_t port, AddressFamily family)
		: address(std::move(address)),
		  port(port),
		  family(family)
	{
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

	bool Address::is_numeric() const
	{
		if (address.empty())
		{
			return false;
		}

		switch (family)
		{
			case AddressFamily::inet:
				for (auto c : address)
				{
					if (c != '.' && (c < '0' || c > '9'))
					{
						return false;
					}
				}

				return true;

			case AddressFamily::inet6:
				for (auto c : address)
				{
					if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
					{
						continue;
					}

					if (c >= '0' && c <= '9')
					{
						continue;
					}

					if (c == ':')
					{
						continue;
					}

					return false;
				}

				return true;

			default:
				return false;
		}
	}

	std::string Address::to_string() const
	{
		std::stringstream result;

		if (family != AddressFamily::inet6 || !is_numeric())
		{
			result << address;

			if (port)
			{
				result << ':' << port;
			}

			return result.str();
		}

		// ipv6
		if (port)
		{
			result << '[' << address << "]:" << port;
		}
		else
		{
			result << address;
		}

		return result.str();
	}
}

size_t std::hash<sws::AddressFamily>::operator()(const sws::AddressFamily& x) const noexcept
{
	return std::hash<size_t> {}(static_cast<size_t>(x));
}

size_t std::hash<sws::Address>::operator()(const sws::Address& x) const noexcept
{
	auto hash = std::hash<std::string> {}(x.address);
	hash_combine(hash, x.port);
	hash_combine(hash, x.family);
	return hash;
}
