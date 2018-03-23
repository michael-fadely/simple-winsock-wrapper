#pragma once
#include <string>
#include <vector>
#include <Ws2tcpip.h>

#include "typedefs.h"
#include "SocketError.h"
#include "SocketException.h"

namespace sws
{
	/**
	 * \brief Defines the address family of an address.
	 */
	enum class AddressFamily
	{
		/**
		 * \brief No address family, or otherwise invalid.
		 */
		none,
		/**
		 * \brief IPv4
		 */
		inet,
		/**
		 * \brief IPv6
		 */
		inet6,
		/**
		 * \brief Used for address resolution.
		 * Indicates that any address family is OK.
		 */
		any
	};

	class AddressResolveException : public SocketException
	{
	public:
		AddressResolveException(const char* address, port_t port, SocketError error);
		AddressResolveException(const char* address, const char* service, SocketError error);
	};

	class Address
	{
	public:
		/**
		 * \brief IP address or hostname.
		 */
		std::string   address {};
		port_t        port   = 0;
		AddressFamily family = AddressFamily::none;

		Address(std::string address, port_t port = 0, AddressFamily family = AddressFamily::none);
		Address() = default;
		Address(const Address&) = default;
		Address(Address&& other) noexcept;
		~Address() = default;

		Address& operator=(const Address& other) = default;
		Address& operator=(Address&& other) noexcept;

		bool operator==(const Address& other) const;
		bool operator!=(const Address& other) const;

		/**
		 * \brief Converts a native address to \c sws::Address
		 * \param address Native address to convert.
		 * \return The converted address.
		 */
		static Address from_native(const sockaddr* address);

		/**
		 * \brief Resolves a hostname or address.
		 * \param host Hostname or string representation of numeric IP address.
		 * \param service Service (e.g. "HTTP"). Use \c nullptr or "0" for any.
		 * \param family Address family type to resolve. If \c sws::AddressFamily::any then both IPv6 and IPv4 is allowed.
		 * \return A \c std::vector of resolved addresses, if any.
		 * \see sws::Address::get_name
		 */
		static std::vector<Address> get_addresses(const char* host, const char* service, AddressFamily family = AddressFamily::any);

		/**
		 * \brief Resolves a hostname or address.
		 * \param host Hostname or string representation of numeric IP address.
		 * \param port Port number, or \c 0 for any. (\c sws::Socket::any_port)
		 * \param family Address family type to resolve. If \c sws::AddressFamily::any then both IPv6 and IPv4 is allowed.
		 * \return A \c std::vector of resolved addresses, if any.
		 * \see sws::Address::get_name
		 */
		static std::vector<Address> get_addresses(const char* host, port_t port = 0, AddressFamily family = AddressFamily::any);

		/**
		 * \brief Gets the hostname of this address.
		 */
		Address get_name() const;

		/**
		 * \brief Returns the native \c sockaddr structure size for a given native address family.
		 * \param family Native address family (e.g \c AF_INET).
		 * \return The size of the native \c sockaddr structure, or \c 0 if unsupportd.
		 */
		static size_t native_size(ADDRESS_FAMILY family);

		/**
		 * \brief Returns native address converted from this instance.
		 * \return The converted native address.
		 */
		sockaddr_storage to_native() const;

		/**
		 * \brief Returns the native \c sockaddr structure size of this instance's address family.
		 */
		size_t native_size() const;

		/**
		 * \brief Determines if this instance is a numeric address.
		 * For example, \c "127.0.0.1" and \c "::1" are numeric.
		 */
		bool is_numeric() const;

		/**
		 * \brief Returns a string representation of this address.
		 * For example, the address \c "::1" and port \c 8080 become
		 * \c "[::1]:8080"
		 */
		std::string to_string() const;
	};
}
