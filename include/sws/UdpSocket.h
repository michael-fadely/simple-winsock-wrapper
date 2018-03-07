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

		inline int send_to(const std::vector<uint8_t>& data, const Address& address) const
		{
			return send_to(data.data(), static_cast<int>(data.size()), address);
		}

		int receive_from(uint8_t* data, int length, Address& address) const;

		inline int receive_from(std::vector<uint8_t>& data, Address& address) const
		{
			return receive_from(data.data(), static_cast<int>(data.size()), address);
		}

		SocketError send_to(Packet& packet, const Address& address) const;

		SocketError receive_from(Packet& packet, Address& address) const;

		template <size_t _size>
		int send_to(const std::array<uint8_t, _size>& data, const Address& address) const;

		template <size_t _size>
		int receive_from(std::array<uint8_t, _size>& data, Address& address) const;
	};

	template <size_t _size>
	int UdpSocket::send_to(const std::array<uint8_t, _size>& data, const Address& address) const
	{
		return send_to(data.data(), static_cast<int>(data.size()), address);
	}

	template <size_t _size>
	int UdpSocket::receive_from(std::array<uint8_t, _size>& data, Address& address) const
	{
		return receive_from(data.data(), static_cast<int>(data.size()), address);
	}
}
