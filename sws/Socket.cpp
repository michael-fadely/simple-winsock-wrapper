#include <sstream>
#include <utility>
#include <WS2tcpip.h>

#include "../include/sws/enforce.h"
#include "../include/sws/typedefs.h"
#include "../include/sws/Socket.h"
#include "../include/sws/Address.h"
#include "../include/sws/Packet.h"

namespace sws
{
	bool Socket::is_initialized_ = false;

	Socket::Socket(Protocol protocol, bool blocking)
		: protocol_(protocol),
		  blocking_(blocking)
	{
		enforce(protocol != Protocol::invalid, "Invalid socket protocol provided.");

		if (protocol_ == Protocol::udp)
		{
			datagram_ = std::make_unique<std::array<uint8_t, datagram_size>>();
		}
	}

	Socket::Socket(Socket&& rhs) noexcept
		: socket_(std::exchange(rhs.socket_, INVALID_SOCKET)),
		  protocol_(rhs.protocol_),
		  remote_address_(std::move(rhs.remote_address_)),
		  local_address_(std::move(rhs.local_address_)),
		  blocking_(rhs.blocking_),
		  connected_(std::exchange(rhs.connected_, false)),
		  native_error_(std::exchange(rhs.native_error_, SocketError::none)),
		  datagram_(std::move(rhs.datagram_))
	{
	}

	Socket& Socket::operator=(Socket&& rhs) noexcept
	{
		if (&rhs != this)
		{
			close();

			socket_         = std::exchange(rhs.socket_, INVALID_SOCKET);
			protocol_       = rhs.protocol_;
			remote_address_ = std::move(rhs.remote_address_);
			local_address_  = std::move(rhs.local_address_);
			blocking_       = rhs.blocking_;
			connected_      = std::exchange(rhs.connected_, false);
			native_error_   = std::exchange(rhs.native_error_, SocketError::none);
			datagram_       = std::move(rhs.datagram_);
		}

		return *this;
	}

	Socket::~Socket()
	{
		close();
	}

	SocketError Socket::initialize()
	{
		if (is_initialized_)
		{
			return SocketError::none;
		}

		WSAData wsa_data {};
		const auto result = static_cast<SocketError>(WSAStartup(MAKEWORD(2, 2), &wsa_data));

		if (result == SocketError::none)
		{
			is_initialized_ = true;
		}

		return result;
	}

	SocketError Socket::cleanup()
	{
		return static_cast<SocketError>(WSACleanup());
	}

	SocketState Socket::bind(const Address& address)
	{
		enforce(socket_ == INVALID_SOCKET, "Cannot bind an already initialized socket.");

		const auto native = address.to_native();

		init_socket(native);

		const int result = ::bind(socket_, reinterpret_cast<const sockaddr*>(&native), static_cast<int>(address.native_size()));

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

		const int result = ::connect(socket_, reinterpret_cast<sockaddr*>(&native), static_cast<int>(address.native_size()));

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
		return ::send(socket_, reinterpret_cast<const char*>(data), length, 0);
	}

	int Socket::send(std::span<const uint8_t> data) const
	{
		return send(data.data(), static_cast<int>(data.size()));
	}

	int Socket::receive(uint8_t* data, int length) const
	{
		return ::recv(socket_, reinterpret_cast<char*>(data), length, 0);
	}

	int Socket::receive(std::span<uint8_t> data) const
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
			static_cast<void>(send(packet.data_));
			return get_error_state();
		}

		if (packet.send_pos_ == -1)
		{
			const int sent = send(packet.data_);

			if (sent > 0)
			{
				packet.send_pos_ = sent;
			}
		}

		auto result = SocketState::done;

		do
		{
			if (packet.get_send_remainder() <= 0)
			{
				break;
			}

			const int sent = send(packet.get_send_data(), static_cast<int>(packet.get_send_remainder()));

			if (sent > 0)
			{
				packet.send_pos_ += sent;
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
			return receive_datagram_packet(packet, receive(*datagram_));
		}

		if (packet.recv_pos_ < 0 && packet.recv_target_ < 0)
		{
			packet.clear();
		}

		if (packet.recv_target_ < 0)
		{
			packetlen_t size = 0;
			const int received = receive(reinterpret_cast<uint8_t*>(&size), sizeof(packetlen_t));

			if (received == sizeof(packetlen_t) && size > 0)
			{
				packet.recv_target_ = size;
			}
		}

		if (packet.recv_target_ <= 0)
		{
			return get_error_state();
		}

		packet.resize(packet.recv_target_ + sizeof(packetlen_t));
		packet.recv_pos_ = sizeof(packetlen_t);

		const int received = receive(packet.get_recv_data(), static_cast<int>(packet.get_recv_remainder()));

		if (received > 0)
		{
			packet.recv_pos_ += received;
		}

		if (!packet.get_recv_remainder())
		{
			packet.recv_reset();
		}

		return get_error_state();
	}

	void Socket::close() noexcept
	{
		if (socket_ != INVALID_SOCKET)
		{
			shutdown(socket_, SD_BOTH);
			closesocket(socket_);
			socket_ = INVALID_SOCKET;
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

	SocketError Socket::get_native_error()
	{
		return static_cast<SocketError>(WSAGetLastError());
	}

	void Socket::init_socket(const sockaddr_storage& native)
	{
		if (socket_ != INVALID_SOCKET)
		{
			return;
		}

		socket_ = ::socket(native.ss_family,
		                   protocol_ == Protocol::tcp ? SOCK_STREAM : SOCK_DGRAM,
		                   protocol_ == Protocol::tcp ? IPPROTO_TCP : IPPROTO_UDP);

		if (socket_ == INVALID_SOCKET)
		{
			throw SocketException("::socket failed", get_error_inst());
		}

		enforce(blocking(blocking_) == SocketState::done);
	}

	void Socket::update_local_address()
	{
		sockaddr_storage addr {};
		socklen_t len = sizeof(sockaddr_storage);
		const auto ptr = reinterpret_cast<sockaddr*>(&addr);

		if (getsockname(socket_, ptr, &len) == SOCKET_ERROR)
		{
			return;
		}

		local_address_ = Address::from_native(ptr);
	}

	void Socket::update_remote_address()
	{
		sockaddr_storage addr {};
		socklen_t len = sizeof(sockaddr_storage);
		const auto ptr = reinterpret_cast<sockaddr*>(&addr);

		if (getpeername(socket_, ptr, &len) == SOCKET_ERROR)
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

		uint8_t* data = datagram_->data();
		const auto size = *reinterpret_cast<packetlen_t*>(data);

		enforce(size == static_cast<packetlen_t>(received) - sizeof(packetlen_t),
		        "packet contains malformed size");

		packet.clear();
		packet.resize(received);
		packet.write_pos_ = 0;
		packet.write_data(datagram_->data(), received, true);
		return clear_error_state();
	}

	bool Socket::blocking() const
	{
		return blocking_;
	}

	SocketState Socket::blocking(bool value)
	{
		blocking_ = value;

		if (socket_ == INVALID_SOCKET)
		{
			return clear_error_state();
		}

		unsigned long mode = value ? 0 : 1;

		if (ioctlsocket(socket_, FIONBIO, &mode) == SOCKET_ERROR)
		{
			return get_error_state();
		}

		return clear_error_state();
	}

	Protocol Socket::protocol() const
	{
		return protocol_;
	}

	bool Socket::is_open() const
	{
		return socket_ != INVALID_SOCKET;
	}
}
