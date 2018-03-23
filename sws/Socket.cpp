#include <sstream>
#include <Ws2tcpip.h>

#include "../include/sws/enforce.h"
#include "../include/sws/typedefs.h"
#include "../include/sws/Socket.h"
#include "../include/sws/Address.h"
#include "../include/sws/Packet.h"

namespace sws
{
	SocketException::SocketException(const char* msg, SocketError error)
		: native_error(error)
	{
		message = msg;
	}

	SocketException::SocketException(const std::string& msg, SocketError error)
		: message(msg.c_str()),
		  native_error(error)
	{
	}

	char const* SocketException::what() const
	{
		return message;
	}

	bool Socket::is_initialized = false;

	Socket::Socket(Protocol protocol, bool blocking)
		: blocking_(blocking)
	{
		enforce(protocol != Protocol::invalid, "Invalid socket protocol provided.");
		this->protocol_ = protocol;

		if (protocol_ == Protocol::udp)
		{
			datagram = new std::array<uint8_t, datagram_size>();
		}
	}

	Socket::Socket(Socket&& other) noexcept
	{
		*this = std::move(other);
	}

	Socket::~Socket()
	{
		delete datagram;
		close();
	}

	SocketError Socket::initialize()
	{
		if (is_initialized)
		{
			return SocketError::none;
		}

		WSAData wsa_data {};
		auto result = static_cast<SocketError>(WSAStartup(MAKEWORD(2, 2), &wsa_data));

		if (result == SocketError::none)
		{
			is_initialized = true;
		}

		return result;
	}

	SocketError Socket::cleanup()
	{
		return static_cast<SocketError>(WSACleanup());
	}

	SocketState Socket::bind(const Address& address)
	{
		enforce(socket == INVALID_SOCKET, "Cannot bind an already initialized socket.");

		auto native = address.to_native();

		init_socket(native);

		int result = ::bind(socket, reinterpret_cast<const sockaddr*>(&native), static_cast<int>(address.native_size()));

		if (result == SOCKET_ERROR)
		{
			return get_error_state();
		}

		update_addresses();
		return clear_error_state();
	}

	SocketState Socket::connect(const Address& address)
	{
		enforce(!connected_, "Cannot connect on already connected socket.");

		auto native = address.to_native();

		init_socket(native);

		int result = ::connect(socket, reinterpret_cast<sockaddr*>(&native), static_cast<int>(address.native_size()));

		if (result == SOCKET_ERROR)
		{
			return get_error_state();
		}

		connected_ = true;
		update_addresses();
		return clear_error_state();
	}

	int Socket::send(const uint8_t* data, int length) const
	{
		return ::send(socket, reinterpret_cast<const char*>(data), length, 0);
	}

	int Socket::send(const std::vector<uint8_t>& data) const
	{
		return send(data.data(), static_cast<int>(data.size()));
	}

	int Socket::receive(uint8_t* data, int length) const
	{
		return ::recv(socket, reinterpret_cast<char*>(data), length, 0);
	}

	int Socket::receive(std::vector<uint8_t>& data) const
	{
		return receive(data.data(), static_cast<int>(data.size()));
	}

	SocketState Socket::send(Packet& packet)
	{
		if (packet.empty())
		{
			packet.send_reset();
			return clear_error_state();
		}

		// For "connected" UDP, we don't have to worry about partial writes.
		if (protocol_ == Protocol::udp)
		{
			send(packet.data_);
			return get_error_state();
		}

		if (packet.send_pos == -1)
		{
			int sent = send(packet.data_);

			if (sent > 0)
			{
				packet.send_pos = sent;
			}
		}

		auto result = SocketState::done;

		do
		{
			if (packet.send_remainder() <= 0)
			{
				break;
			}

			int sent = send(packet.send_data(), static_cast<int>(packet.send_remainder()));

			if (sent > 0)
			{
				packet.send_pos += sent;
			}

			result = get_error_state();
		} while (result == SocketState::in_progress);

		packet.send_reset();
		return result;
	}

	SocketState Socket::receive(Packet& packet)
	{
		// For "connected" UDP, receive like a datagram.
		if (protocol_ == Protocol::udp)
		{
			return receive_datagram_packet(packet, receive(*datagram));
		}

		if (packet.recv_pos < 0 && packet.recv_target < 0)
		{
			packet.clear();
		}

		if (packet.recv_target < 0)
		{
			packetlen_t size = 0;
			int received = receive(reinterpret_cast<uint8_t*>(&size), sizeof(packetlen_t));

			if (received == sizeof(packetlen_t) && size > 0)
			{
				packet.recv_target = size;
			}
		}

		if (packet.recv_target <= 0)
		{
			return get_error_state();
		}

		packet.resize(packet.recv_target + sizeof(packetlen_t));
		packet.recv_pos = sizeof(packetlen_t);

		int received = receive(packet.recv_data(), static_cast<int>(packet.recv_remainder()));

		if (received > 0)
		{
			packet.recv_pos += received;
		}

		if (!packet.recv_remainder())
		{
			packet.recv_reset();
		}

		return get_error_state();
	}

	void Socket::close()
	{
		if (socket != INVALID_SOCKET)
		{
			shutdown(socket, SD_BOTH);
			closesocket(socket);
			socket = INVALID_SOCKET;
		}

		remote_address_ = {};
		local_address_  = {};
		connected_      = false;
	}

	const Address& Socket::remote_address() const
	{
		return remote_address_;
	}

	const Address& Socket::local_address() const
	{
		return local_address_;
	}

	SocketError Socket::native_error() const
	{
		return native_error_;
	}

	Socket& Socket::operator=(Socket&& s) noexcept
	{
		close();

		socket          = s.socket;
		protocol_       = s.protocol_;
		blocking_       = s.blocking_;
		local_address_  = s.local_address_;
		remote_address_ = s.remote_address_;
		datagram        = s.datagram;

		s.socket   = INVALID_SOCKET;
		s.datagram = nullptr;

		return *this;
	}

	SocketError Socket::get_native_error()
	{
		return static_cast<SocketError>(WSAGetLastError());
	}

	void Socket::init_socket(const sockaddr_storage& native)
	{
		if (socket != INVALID_SOCKET)
		{
			return;
		}

		socket = ::socket(native.ss_family,
						  protocol_ == Protocol::tcp ? SOCK_STREAM : SOCK_DGRAM,
						  protocol_ == Protocol::tcp ? IPPROTO_TCP : IPPROTO_UDP);

		if (socket == INVALID_SOCKET)
		{
			throw SocketException("::socket failed", get_error_inst());
		}

		basic_enforce(blocking(blocking_) == SocketState::done);
	}

	void Socket::update_local_address()
	{
		sockaddr_storage addr {};
		socklen_t len = sizeof(sockaddr_storage);
		auto ptr = reinterpret_cast<sockaddr*>(&addr);

		if (getsockname(socket, ptr, &len) == SOCKET_ERROR)
		{
			return;
		}

		local_address_ = Address::from_native(ptr);
	}

	void Socket::update_remote_address()
	{
		sockaddr_storage addr {};
		socklen_t len = sizeof(sockaddr_storage);
		auto ptr = reinterpret_cast<sockaddr*>(&addr);

		if (getpeername(socket, ptr, &len) == SOCKET_ERROR)
		{
			return;
		}

		remote_address_ = Address::from_native(ptr);
	}

	void Socket::update_addresses()
	{
		update_local_address();
		update_remote_address();
	}

	SocketError Socket::get_error_inst()
	{
		native_error_ = get_native_error();
		return native_error_;
	}

	SocketState Socket::get_error_state()
	{
		return to_state(get_error_inst());
	}

	SocketError Socket::clear_error()
	{
		native_error_ = SocketError::none;
		return native_error_;
	}

	SocketState Socket::clear_error_state()
	{
		clear_error();
		return SocketState::done;
	}

	SocketState Socket::receive_datagram_packet(Packet& packet, int received)
	{
		if (received == SOCKET_ERROR || !received)
		{
			return get_error_state();
		}

		enforce(static_cast<size_t>(received) >= sizeof(packetlen_t),
			"packet too small to be a packet");

		auto size = *reinterpret_cast<packetlen_t*>(&datagram[0]);

		enforce(size == static_cast<packetlen_t>(received) - sizeof(packetlen_t),
			"packet contains malformed size");

		packet.clear();
		packet.resize(received);
		packet.write_pos = 0;
		packet.write_data(datagram->data(), received, true);
		return clear_error_state();
	}

	bool Socket::blocking() const
	{
		return blocking_;
	}

	SocketState Socket::blocking(bool value)
	{
		blocking_ = value;

		if (socket == INVALID_SOCKET)
		{
			return clear_error_state();
		}

		unsigned long mode = value ? 0 : 1;

		if (ioctlsocket(socket, FIONBIO, &mode) == SOCKET_ERROR)
		{
			return get_error_state();
		}

		return clear_error_state();
	}

	Protocol Socket::protocol() const
	{
		return protocol_;
	}
}
