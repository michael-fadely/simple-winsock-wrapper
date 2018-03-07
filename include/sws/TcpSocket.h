#pragma once

#include "Socket.h"

namespace sws
{
	class TcpSocket : public Socket
	{
	public:
		TcpSocket();
		explicit TcpSocket(bool blocking);

		/**
		* \brief Begins listening for connections.
		* \return \c SocketError::none on success.
		*/
		SocketError listen() const;

		/**
		* \brief Accept a connection after a call to \c Socket::listen.
		* \param s Destination socket.
		* \return \c SocketError::none on success.
		*/
		SocketError accept(TcpSocket& s) const;

		bool send_all(const uint8_t* data, int length) const;

		inline bool send_all(const std::vector<uint8_t>& data) const
		{
			return send_all(data.data(), static_cast<int>(data.size()));
		}

		bool receive_all(uint8_t* data, int length) const;

		inline bool receive_all(std::vector<uint8_t>& data) const
		{
			return receive_all(data.data(), static_cast<int>(data.size()));
		}

		template <size_t _size>
		bool send_all(const std::array<uint8_t, _size>& data) const;

		template <size_t _size>
		bool receive_all(std::array<uint8_t, _size>& data) const;
	};

	template <size_t _size>
	bool TcpSocket::send_all(const std::array<uint8_t, _size>& data) const
	{
		return send_all(data.data(), static_cast<int>(data.size()));
	}

	template <size_t _size>
	bool TcpSocket::receive_all(std::array<uint8_t, _size>& data) const
	{
		return receive_all(data.data(), static_cast<int>(data.size()));
	}
}
