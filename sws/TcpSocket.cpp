#include "../include/sws/TcpSocket.h"

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

	SocketState TcpSocket::listen()
	{
		if (::listen(socket_, SOMAXCONN) == SOCKET_ERROR)
		{
			return get_error_state();
		}

		return clear_error_state();
	}

	SocketState TcpSocket::accept(TcpSocket& s)
	{
		const NativeSocket sock = ::accept(socket_, nullptr, nullptr);

		if (sock == INVALID_SOCKET)
		{
			return get_error_state();
		}

		s = TcpSocket(blocking_);

		s.socket_ = sock;
		s.connected_ = true;

		s.blocking(blocking_);
		s.update_addresses();

		return clear_error_state();
	}

	bool TcpSocket::send_all(const uint8_t* data, int length)
	{
		int total = 0;

		while (total < length)
		{
			const auto sent = send(&data[total], length - total);

			if (!sent)
			{
				return false;
			}

			if (sent < 0)
			{
				if (get_error_inst() == SocketError::would_block)
				{
					continue;
				}

				return false;
			}

			total += sent;
		}

		return true;
	}

	bool TcpSocket::send_all(const std::vector<uint8_t>& data)
	{
		return send_all(data.data(), static_cast<int>(data.size()));
	}

	bool TcpSocket::receive_all(uint8_t* data, int length)
	{
		int total = 0;

		while (total < length)
		{
			const auto received = receive(&data[total], length - total);

			if (!received)
			{
				return false;
			}

			if (received < 0)
			{
				if (get_error_inst() == SocketError::would_block)
				{
					continue;
				}

				return false;
			}

			total += received;
		}

		return true;
	}

	bool TcpSocket::receive_all(std::vector<uint8_t>& data)
	{
		return receive_all(data.data(), static_cast<int>(data.size()));
	}
}
