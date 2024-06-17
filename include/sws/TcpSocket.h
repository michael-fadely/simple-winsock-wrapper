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
		bool send_all(std::span<const uint8_t> data);

		// TODO: re-implement
		bool receive_all(uint8_t* data, int length);

		// TODO: re-implement
		bool receive_all(std::span<uint8_t> data);
	};
}
