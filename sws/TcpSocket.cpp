#include <sws/TcpSocket.h>

namespace sws
{
	TcpSocket::TcpSocket()
		: Socket(Protocol::tcp, true)
	{
	}

	TcpSocket::TcpSocket(bool blocking)
		: Socket(Protocol::tcp, blocking)
	{
	}

	SocketError TcpSocket::listen() const
	{
		if (::listen(socket, SOMAXCONN) == SOCKET_ERROR)
		{
			return get_error();
		}

		return SocketError::none;
	}

	SocketError TcpSocket::accept(TcpSocket& s) const
	{
		const NativeSocket sock = ::accept(socket, nullptr, nullptr);

		if (sock == INVALID_SOCKET)
		{
			return get_error();
		}

		s = TcpSocket(blocking_);

		s.socket = sock;
		s.connected_ = true;

		s.blocking(blocking_);
		s.update_addresses();

		return SocketError::none;
	}

	bool TcpSocket::send_all(const uint8_t* data, int length) const
	{
		int total = 0;

		while (total < length)
		{
			const auto sent = send(&data[total], length - total);

			if (sent < 0)
			{
				if (get_error() == SocketError::would_block)
				{
					continue;
				}

				return false;
			}

			if (!sent)
			{
				return false;
			}

			if (sent > 0)
			{
				total += sent;
			}
		}

		return true;
	}

	bool TcpSocket::receive_all(uint8_t* data, int length) const
	{
		int total = 0;

		while (total < length)
		{
			const auto received = receive(&data[total], length - total);

			if (received < 0)
			{
				if (get_error() == SocketError::would_block)
				{
					continue;
				}

				return false;
			}

			if (!received)
			{
				return false;
			}

			if (received > 0)
			{
				total += received;
			}
		}

		return true;
	}
}
