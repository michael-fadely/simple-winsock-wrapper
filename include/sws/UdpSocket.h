#pragma once

#include "Socket.h"

namespace sws
{
	class UdpSocket : public Socket
	{
	public:
		/**
		 * \brief Construct a blocking UDP socket.
		 */
		UdpSocket();

		/**
		 * \brief Construct a UDP socket with blocking specified.
		 * \param blocking Whether or not the socket should be blocking or non-blocking.
		 */
		explicit UdpSocket(bool blocking);

		/**
		 * \brief Sends a raw buffer to an address.
		 * \param data Buffer of data to be sent.
		 * \param length Length of \p data.
		 * \param address Address to send to.
		 * \return \c -1 on error, or length of \p data on success.
		 * \remark Note that this method does not perform error handling.
		 */
		int send_to(const uint8_t* data, int length, const Address& address) const;

		/**
		 * \brief Sends a raw buffer to an address.
		 * \param data Buffer to be sent.
		 * \param address Address to send to.
		 * \return \c -1 on error, or length of \p data on success.
		 * \remark Note that this method does not perform error handling.
		 */
		int send_to(const std::vector<uint8_t>& data, const Address& address) const;

		/**
		 * \brief Receives a raw buffer of data from an address.
		 * \param data Buffer to receive into.
		 * \param length Length of \p data.
		 * \param address Address of the data's origin.
		 * \return \c -1 on error, non-zero positive number of bytes received on success.
		 * \remark Note that this method does not perform error handling.
		 */
		int receive_from(uint8_t* data, int length, Address& address) const;

		/**
		 * \brief Receives a raw buffer of data from an address.
		 * \param data Buffer to receive into.
		 * \param address Address of the data's origin.
		 * \return \c -1 on error, non-zero positive number of bytes received on success.
		 * \remark Note that this method does not perform error handling.
		 */
		int receive_from(std::vector<uint8_t>& data, Address& address) const;

		/**
		 * \brief Sends a packet to an address.
		 * \param packet \c sws::Packet to send.
		 * \param address Address to send to.
		 * \return \c sws::SocketState::done on success.
		 */
		SocketState send_to(const Packet& packet, const Address& address);

		/**
		 * \brief Receives a packet from an address.
		 * \param packet \c sws::Packet to receive into.
		 * \param address Address of packet's origin.
		 * \return \c sws::SocketState::done on success.
		 */
		SocketState receive_from(Packet& packet, Address& address);

		/**
		 * \brief Sends a raw buffer to an address.
		 * \tparam _size Templated array size.
		 * \param data Buffer to send.
		 * \param address Address to send to.
		 * \return \c -1 on error, or length of \p data on success.
		 * \remark Note that this method does not perform error handling.
		 */
		template <size_t _size>
		int send_to(const std::array<uint8_t, _size>& data, const Address& address);

		/**
		 * \brief Receives a raw buffer of data from an address.
		 * \tparam _size Templated array size.
		 * \param data Buffer to receive into.
		 * \param address Address of the data's origin.
		 * \return \c -1 on error, or length of \p data on success.
		 * \remark Note that this method does not perform error handling.
		 */
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
