#pragma once

#include "Socket.h"

namespace sws
{
	class TcpSocket : public Socket
	{
	public:
		TcpSocket();
		explicit TcpSocket(bool blocking);

		SocketState listen();

		SocketState accept(TcpSocket& s);

		bool send_all(const uint8_t* data, int length);

		bool send_all(const std::vector<uint8_t>& data);

		bool receive_all(uint8_t* data, int length);

		bool receive_all(std::vector<uint8_t>& data);

		template <size_t _size>
		bool send_all(const std::array<uint8_t, _size>& data);

		template <size_t _size>
		bool receive_all(std::array<uint8_t, _size>& data);
	};

	template <size_t _size>
	bool TcpSocket::send_all(const std::array<uint8_t, _size>& data)
	{
		return send_all(data.data(), static_cast<int>(data.size()));
	}

	template <size_t _size>
	bool TcpSocket::receive_all(std::array<uint8_t, _size>& data)
	{
		return receive_all(data.data(), static_cast<int>(data.size()));
	}
}
