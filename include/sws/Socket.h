#pragma once
#include <WinSock2.h>

#include <array>
#include <memory>
#include <vector>

#include "typedefs.h"
#include "Address.h"
#include "SocketError.h"

namespace sws
{
	class Packet;
	using NativeSocket = SOCKET;

	/**
	 * \brief Defines the protocol of a socket (TCP, UDP).
	 */
	enum class Protocol
	{
		invalid,
		tcp,
		udp
	};

	class Socket
	{
		static bool is_initialized_;

	public:
		/**
		 * \brief Used for resolving addresses.
		 * Indicates that the system should provide any port.
		 */
		static constexpr port_t any_port = 0;

		/**
		 * \brief Used for resolving addresses.
		 * Indicates that the system should provide any address.
		 */
		static constexpr auto any_address = "\0";

		/**
		 * \brief Maximum datagram size.
		 */
		static constexpr size_t datagram_size = 65536;

	protected:
		NativeSocket socket_ = INVALID_SOCKET;

		Protocol protocol_       = Protocol::invalid;
		Address  remote_address_ = {};
		Address  local_address_  = {};
		bool     blocking_       = true;
		bool     connected_      = false;

		SocketError native_error_ = SocketError::none;

		std::unique_ptr<std::array<uint8_t, datagram_size>> datagram_;

		/**
		 * \brief Construct a socket.
		 * \param protocol The protocol of the socket.
		 * \param blocking Whether or not the socket should block.
		 */
		Socket(Protocol protocol, bool blocking);

	public:
		/**
		 * \brief Specifically disallows copy operations. Sockets must be moved.
		 * \see std::move
		 */
		Socket(Socket& other) = delete;

		/**
		 * \brief Specifically disallows copy operations. Sockets must be moved.
		 * \see std::move
		 */
		Socket& operator=(Socket& s) = delete;

		// FIXME: This might break with TcpSocket + UdpSocket. Maybe make protected?
		/**
		 * \brief Move constructor. \p rhs will be invalidated.
		 * \see std::move
		 */
		Socket(Socket&& rhs) noexcept;

		// FIXME: This might break with TcpSocket + UdpSocket. Maybe make protected?
		/**
		 * \brief Move assignment operator. \p rhs will be invalidated.
		 */
		Socket& operator=(Socket&& rhs) noexcept;

		/**
		 * \brief Destructor. Automatically calls \c Socket::close
		 */
		~Socket();

		/**
		 * \brief Initializes Winsock. Must be called before creating sockets.
		 * \return Native error code.
		 * \see sws::SocketError
		 */
		static SocketError initialize();

		/**
		 * \brief Cleans up after Winsock.
		 * It is recommended that this is called on program exit or
		 * when no more sockets are required.
		 * \return Native error code.
		 * \see sws::SocketError
		 */
		static SocketError cleanup();

		/**
		 * \brief Binds this socket to the specified address and port.
		 * \param address The address and port to bind to.
		 * \return \c sws::SocketState::done on success.
		 * \see sws::SocketState
		 * \see sws::Address
		 */
		SocketState bind(const Address& address);

		/**
		 * \brief Connects this socket to the specified address and port.
		 * \param address The address and port to connect to.
		 * \return \c sws::SocketState::done on success.
		 * \remark This may be used with UDP sockets for convenience.
		 * \see sws::SocketState
		 * \see sws::Address
		 */
		SocketState connect(const Address& address);

		/**
		 * \brief Send a raw buffer of data.
		 * \param data Pointer to an array of data to send.
		 * \param length Length of the array.
		 * \return \c -1 on error, \c 0 if the socket is closed, > \c 0 on success.
		 * \remark Note that this method does not provide error handling.
		 */
		int send(const uint8_t* data, int length) const;

		/**
		 * \brief Send a raw buffer of data.
		 * \param data std::vector of data to be sent.
		 * \return \c -1 on error, \c 0 if the socket is closed, > \c 0 on success.
		 * \remark Note that this method does not provide error handling.
		 */
		int send(const std::vector<uint8_t>& data) const;

		/**
		 * \brief Receive a raw buffer of data.
		 * \param data Destination buffer.
		 * \param length Length of \p data
		 * \return \c -1 on error, \c 0 if the socket is closed, > \c 0 on success.
		 * \remark Note that this method does not provide error handling.
		 */
		int receive(uint8_t* data, int length) const;

		/**
		 * \brief Receive a raw buffer of data.
		 * \param data Destination buffer.
		 * \return \c -1 on error, \c 0 if the socket is closed, > \c 0 on success.
		 * \remark Note that this method does not provide error handling.
		 */
		int receive(std::vector<uint8_t>& data) const;

		/**
		 * \brief Sends a \c sws::Packet to a connected peer.
		 * \param packet sws::Packet to be sent.
		 * \return \c sws::SocketState::done on success.
		 * \remark Note that this method prepends the data with a \c sizeof(packetlen_t) byte length.
		 * \see sws::SocketState
		 * \see sws::Packet
		 */
		SocketState send(Packet& packet);

		/**
		 * \brief Receives a \c sws::Packet from a connected peer.
		 * \param packet \c sws::Packet to receive into.
		 * \return \c sws::SocketState::done on success.
		 * \remark Note that this method expects the received data to start with a \c sizeof(packetlen_t) byte length.
		 * \see sws::SocketState
		 * \see sws::Packet
		 */
		SocketState receive(Packet& packet);

		/**
		 * \brief Sends a raw buffer of data.
		 * \tparam _size Templated array length.
		 * \param data \c std::array of data to be sent.
		 * \return \c -1 on error, \c 0 if the socket is closed, > \c 0 on success.
		 * \remark Note that this method does not provide error handling.
		 */
		template <size_t _size>
		int send(const std::array<uint8_t, _size>& data) const;

		/**
		 * \brief Receives a raw buffer of data.
		 * \tparam _size Templated array length.
		 * \param data \c std::array of data to be sent.
		 * \return \c -1 on error, \c 0 if the socket is closed, > \c 0 on success.
		 * \remark Note that this method does not provide error handling.
		 */
		template <size_t _size>
		int receive(std::array<uint8_t, _size>& data) const;

		/**
		 * \brief Closes this socket (unbinds, etc).
		 */
		void close() noexcept;

		/**
		 * \brief Gets the remote address/port the socket is connected to.
		 * \remark Note that this does not work (returns empty) with UDP sockets,
		 * even if \c Socket::connect is called on one.
		 * \see sws::Address
		 */
		[[nodiscard]] const Address& remote_address() const;

		/**
		 * \brief Gets the local address/port of this socket.
		 * \see sws::Address
		 */
		[[nodiscard]] const Address& local_address() const;

		/**
		 * \brief Gets the last native socket error.
		 * \remark If a method returns \c sws::SocketState::error then
		 * this method may be used to dig deeper into the error.
		 * \see sws::SocketError
		 */
		[[nodiscard]] SocketError native_error() const;

		/**
		 * \brief Gets the current blocking state.
		 */
		[[nodiscard]] bool blocking() const;

		/**
		 * \brief Sets the current blocking state.
		 * \param value Blocking state to set.
		 * \return \c sws::SocketState::done on success.
		 * \remark
		 * If the socket is not connected, this method will
		 * always succeed, and clear any stored error state.
		 * The blocking state will be updated once the native
		 * socket is connected or bound.
		 * 
		 * \see sws::SocketState
		 */
		SocketState blocking(bool value);

		/**
		 * \brief Gets the protocol of this socket.
		 * \see sws::Protocol
		 */
		[[nodiscard]] Protocol protocol() const;

		/**
		 * \brief Checks if the socket is currently open.
		 */
		[[nodiscard]] bool is_open() const;

		/**
		 * \brief Gets the last native socket error.
		 */
		[[nodiscard]] static SocketError get_native_error();

	protected:
		void init_socket(const sockaddr_storage& native);

		void update_local_address();
		void update_remote_address();
		void update_addresses();

		SocketError get_error_inst();
		SocketState get_error_state();

		SocketError clear_error();
		SocketState clear_error_state();

		SocketState receive_datagram_packet(Packet& packet, int received);
	};

	template <size_t _size>
	int Socket::send(const std::array<uint8_t, _size>& data) const
	{
		return send(data.data(), static_cast<int>(data.size()));
	}

	template <size_t _size>
	int Socket::receive(std::array<uint8_t, _size>& data) const
	{
		return receive(data.data(), static_cast<int>(data.size()));
	}
}
