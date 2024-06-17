#include "../include/sws/UdpSocket.h"
#include "../include/sws/Packet.h"

namespace sws
{
	UdpSocket::UdpSocket()
		: Socket(Protocol::udp, true)
	{
	}

	UdpSocket::UdpSocket(bool blocking)
		: Socket(Protocol::udp, blocking)
	{
	}

	int UdpSocket::send_to(const uint8_t* data, int length, const Address& address) const
	{
		const auto native_address = address.to_native();
		const auto size = static_cast<int>(address.native_size());

		return sendto(socket_,
		              reinterpret_cast<const char*>(data),
		              length,
		              0,
		              reinterpret_cast<const sockaddr*>(&native_address),
		              size);
	}

	int UdpSocket::send_to(std::span<const uint8_t> data, const Address& address) const
	{
		return send_to(data.data(), static_cast<int>(data.size()), address);
	}

	int UdpSocket::receive_from(uint8_t* data, int length, Address& address) const
	{
		sockaddr_storage native {};
		int size = sizeof(sockaddr_storage);

		auto ptr = reinterpret_cast<sockaddr*>(&native);

		const int result = recvfrom(socket_, reinterpret_cast<char*>(data), length, 0, ptr, &size);

		if (result != SOCKET_ERROR)
		{
			address = Address::from_native(ptr);
		}

		return result;
	}

	int UdpSocket::receive_from(std::span<uint8_t> data, Address& address) const
	{
		return receive_from(data.data(), static_cast<int>(data.size()), address);
	}

	SocketState UdpSocket::send_to(const Packet& packet, const Address& address)
	{
		if (packet.empty())
		{
			return clear_error_state();
		}

		const int sent = send_to(packet.data(), address);

		if (!sent || sent == SOCKET_ERROR)
		{
			return get_error_state();
		}

		return clear_error_state();
	}

	SocketState UdpSocket::receive_from(Packet& packet, Address& address)
	{
		return receive_datagram_packet(packet, receive_from(*datagram_, address));
	}
}
