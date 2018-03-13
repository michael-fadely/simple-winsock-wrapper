#pragma once
#include <WinSock2.h>

#include <array>
#include <string>
#include <vector>

#include "typedefs.h"
#include "Address.h"
#include "SocketError.h"

namespace sws
{
	class Packet;
	using NativeSocket = SOCKET;

	enum class Protocol
	{
		invalid,
		tcp,
		udp
	};

	class SocketException : public std::exception
	{
		const char* message = nullptr;

	public:
		const SocketError native_error;

		SocketException(const char*        msg, SocketError error);
		SocketException(const std::string& msg, SocketError error);

		char const* what() const override;
	};

	class Socket
	{
		static bool is_initialized;

	public:
		static const port_t any_port      = 0;
		static const size_t datagram_size = 65536;

	protected:
		NativeSocket socket = INVALID_SOCKET;

		Protocol protocol_       = Protocol::invalid;
		Address  remote_address_ = {};
		Address  local_address_  = {};
		bool     blocking_       = true;
		bool     connected_      = false;

		SocketError native_error_ = SocketError::none;

		std::array<uint8_t, datagram_size>* datagram = nullptr;

	public:
		Socket(Protocol protocol, bool blocking);
		Socket(Socket&  other) = delete;
		Socket(Socket&& other) noexcept;
		~Socket();

		static SocketError initialize();
		static SocketError cleanup();

		SocketState bind(const Address& address);

		SocketState connect(const Address& address);

		int send(const uint8_t* data, int length) const;

		int send(const std::vector<uint8_t>& data) const;

		int receive(uint8_t* data, int length) const;

		int receive(std::vector<uint8_t>& data) const;

		SocketState send(Packet& packet);

		SocketState receive(Packet& packet);

		template <size_t _size>
		int send(const std::array<uint8_t, _size>& data) const;

		template <size_t _size>
		int receive(std::array<uint8_t, _size>& data) const;

		void close();

		const Address& remote_address() const;
		const Address& local_address() const;
		SocketError native_error() const;

		bool blocking() const;
		void blocking(bool value);

		Protocol protocol() const;

		Socket& operator=(Socket&  s) = delete;
		Socket& operator=(Socket&& s) noexcept;

		static SocketError get_error();

	protected:
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
