#pragma once

#include "Socket.h"

namespace sws
{
	class TcpSocket : public Socket
	{
	public:
		/**
		 * \brief Construct a blocking TCP socket.
		 */
		TcpSocket();

		/**
		 * \brief Construct a TCP socket with blocking specified.
		 * \param blocking Whether or not the socket should be blocking or non-blocking.
		 */
		explicit TcpSocket(bool blocking);

		/**
		 * \brief Begins listening on this socket for incoming connections.
		 * \return \c SocketState::done on success.
		 */
		SocketState listen();

		/**
		 * \brief Accepts an incoming connection, if any.
		 * \param s [out] Accepted connection.
		 * \return \c SocketState::done if a socket has been accepted.
		 */
		SocketState accept(TcpSocket& s);

		// TODO: re-implement
		bool send_all(const uint8_t* data, int length);

		// TODO: re-implement
		bool send_all(const std::vector<uint8_t>& data);

		// TODO: re-implement
		bool receive_all(uint8_t* data, int length);

		// TODO: re-implement
		bool receive_all(std::vector<uint8_t>& data);

		// TODO: re-implement
		template <size_t _size>
		bool send_all(const std::array<uint8_t, _size>& data);

		// TODO: re-implement
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
