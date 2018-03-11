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

		return sendto(socket, reinterpret_cast<const char*>(data), length, 0,
			reinterpret_cast<const sockaddr*>(&native_address), size);
	}

	int UdpSocket::receive_from(uint8_t* data, int length, Address& address) const
	{
		sockaddr_storage native {};
		int size = sizeof(sockaddr_storage);

		auto ptr = reinterpret_cast<sockaddr*>(&native);

		int result = recvfrom(socket, reinterpret_cast<char*>(data), length, 0, ptr, &size);

		address = Address::from_native(ptr);
		return result;
	}

	SocketError UdpSocket::send_to(Packet& packet, const Address& address) const
	{
		if (packet.empty())
		{
			return SocketError::none;
		}

		int sent = send_to(packet.data_vector(), address);

		if (!sent || sent == SOCKET_ERROR)
		{
			return get_error();
		}

		return SocketError::none;
	}

	SocketError UdpSocket::receive_from(Packet& packet, Address& address) const
	{
		return receive_datagram_packet(packet, receive_from(*datagram, address));
	}
}
