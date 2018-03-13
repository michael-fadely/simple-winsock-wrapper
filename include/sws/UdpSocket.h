#pragma once

#include "Socket.h"

namespace sws
{
	class UdpSocket : public Socket
	{
	public:
		UdpSocket();

		explicit UdpSocket(bool blocking);

		int send_to(const uint8_t* data, int length, const Address& address) const;

		int send_to(const std::vector<uint8_t>& data, const Address& address) const;

		int receive_from(uint8_t* data, int length, Address& address) const;

		int receive_from(std::vector<uint8_t>& data, Address& address) const;

		SocketState send_to(Packet& packet, const Address& address);

		SocketState receive_from(Packet& packet, Address& address);

		template <size_t _size>
		int send_to(const std::array<uint8_t, _size>& data, const Address& address);

		template <size_t _size>
		int receive_from(std::array<uint8_t, _size>& data, Address& address);
	};

	template <size_t _size>
	int UdpSocket::send_to(const std::array<uint8_t, _size>& data, const Address& address)
	{
		return send_to(data.data(), static_cast<int>(data.size()), address);
	}

	template <size_t _size>
	int UdpSocket::receive_from(std::array<uint8_t, _size>& data, Address& address)
	{
		return receive_from(data.data(), static_cast<int>(data.size()), address);
	}
}
