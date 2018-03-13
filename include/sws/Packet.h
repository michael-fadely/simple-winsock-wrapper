#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "enforce.h"
#include "typedefs.h"

// TODO: document <</>> operators

namespace sws
{
	class Socket;

	/**
	 * \brief Defines the different cursor types used for packet read/write operations.
	 */
	enum class SeekCursor
	{
		/**
		 * \brief Cursor used for reading data.
		 */
		read,
		/**
		 * \brief Cursor used for writing data.
		 */
		write,
		/**
		 * \brief Both read and write cursors.
		 */
		both
	};

	/**
	 * \brief Defines the type of seek to perform on a \c sws::Packet
	 */
	enum class SeekType
	{
		/**
		 * \brief Seek to a position relative to the start of the packet.
		 */
		from_start,
		/**
		 * \brief Seek relative to current position in the packet.
		 */
		relative,
		/**
		 * \brief Seek to a position relative to the end of the packet.
		 */
		from_end
	};

	static_assert(sizeof(float) == 4, "sizeof(float) != 4");
	static_assert(sizeof(double) == 8, "sizeof(double) != 8");

	class Packet
	{
		friend class Socket;

	protected:
		ptrdiff_t send_pos    = -1;
		ptrdiff_t recv_pos    = -1;
		ptrdiff_t recv_target = -1;

		std::vector<uint8_t> buffer;

		ptrdiff_t read_pos  = sizeof(packetlen_t);
		ptrdiff_t write_pos = sizeof(packetlen_t);

	public:
		Packet();
		/**
		 * \brief Constructs a packet with a manually tuned reserve size.
		 * \param reserve Number of bytes to reserve in the internal buffer.
		 */
		explicit Packet(size_t reserve);
		Packet(Packet&) = default;
		Packet(Packet&& other) noexcept;
		virtual ~Packet() = default;

		Packet& operator=(const Packet& other) = default;
		Packet& operator=(Packet&& other) noexcept;

		/**
		 * \brief Seeks a cursor in the packet for advanced read/write.
		 * \param cursor The cursor to seek.
		 * \param type The seek mode.
		 * \param value Value by which to seek (accordng to \p type)
		 */
		void seek(SeekCursor cursor, SeekType type, ptrdiff_t value);

		/**
		 * \brief Returns the position of the given cursor in the packet.
		 * \param cursor Cursor whose position is being queried.
		 * \return The position of the cursor in the packet.
		 */
		ptrdiff_t tell(SeekCursor cursor) const;

		/**
		 * \brief Reads raw data into a buffer.
		 * \param data Buffer to read into.
		 * \param size Size of \p data
		 * \param whole If \c true, requires that read size must equal \p size
		 * \return Number of bytes read.
		 */
		size_t read_data(void* data, size_t size, bool whole);

		/**
		 * \brief Reads raw data into a buffer.
		 * \param data Buffer to read into.
		 * \param whole If \c true, indicates that read size must equal the length of \p data
		 * \return Number of bytes read.
		 */
		size_t read_data(std::vector<uint8_t>& data, bool whole);

		/**
		 * \brief Reads raw data into a buffer.
		 * \tparam _size Templated array size.
		 * \param data Buffer to read into.
		 * \param whole If \c true, indicates that read size must equal the length of \p data
		 * \return Number of bytes read.
		 */
		template <size_t _size>
		size_t read_data(std::array<uint8_t, _size>& data, bool whole);

		/**
		 * \brief Reads a \c std::string out of the packet.
		 * \param data Destination.
		 * \return Number of bytes read.
		 */
		size_t read(std::string& data);

		/**
		 * \brief Reads a \c bool out of the packet.
		 * \param data Destination.
		 * \return Number of bytes read (\c 1).
		 * \remark \c sws::Packet treats \c bool as one byte on all platforms.
		 */
		size_t read(bool& data);

		/**
		 * \brief Reads a byte from the packet.
		 * \param data Destination.
		 * \return \c sizeof(int8_t) on success, \c 0 on failure.
		 */
		size_t read(int8_t& data);

		/**
		 * \brief Reads a byte from the packet.
		 * \param data Destination.
		 * \return \c sizeof(uint8_t) on success, \c 0 on failure.
		 */
		size_t read(uint8_t& data);

		/**
		 * \brief Reads a 16-bit integer from the packet.
		 * \param data Destination.
		 * \return \c sizeof(int16_t) on success, \c 0 on failure.
		 */
		size_t read(int16_t& data);

		/**
		 * \brief Reads a 16-bit integer from the packet.
		 * \param data Destination.
		 * \return \c sizeof(uint16_t) on success, \c 0 on failure.
		 */
		size_t read(uint16_t& data);

		/**
		 * \brief Reads a 32-bit integer from the packet.
		 * \param data Destination.
		 * \return \c sizeof(int32_t) on success, \c 0 on failure.
		 */
		size_t read(int32_t& data);

		/**
		 * \brief Reads a 32-bit integer from the packet.
		 * \param data Destination.
		 * \return \c sizeof(uint32_t) on success, \c 0 on failure.
		 */
		size_t read(uint32_t& data);

		/**
		 * \brief Reads a 64-bit integer from the packet.
		 * \param data Destination.
		 * \return \c sizeof(int64_t) on success, \c 0 on failure.
		 */
		size_t read(int64_t& data);

		/**
		 * \brief Reads a 64-bit integer from the packet.
		 * \param data Destination.
		 * \return \c sizeof(uint64_t) on success, \c 0 on failure.
		 */
		size_t read(uint64_t& data);

		/**
		 * \brief Reads a 32-bit floating point number from the packet.
		 * \param data Destination.
		 * \return \c sizeof(float) on success, \c 0 on failure.
		 */
		size_t read(float& data);

		/**
		 * \brief Reads a 64-bit floating point number from the packet.
		 * \param data Destination.
		 * \return \c sizeof(double) on success, \c 0 on failure.
		 */
		size_t read(double& data);

		/**
		 * \brief Writes a raw buffer into the packet.
		 * \param data Buffer to write to the packet.
		 * \param size Length of \p data
		 * \param whole If \c true, indicates that write size must equal the length of \p data
		 * \return Number of bytes written.
		 */
		size_t write_data(const void* data, size_t size, bool whole);

		/**
		 * \brief Writes a raw buffer into the packet.
		 * \param data Buffer to write to the packet.
		 * \param whole If \c true, indicates that write size must equal the length of \p data
		 * \return Number of bytes written.
		 */
		size_t write_data(const std::vector<uint8_t>& data, bool whole);

		/**
		 * \brief Writes a raw buffer into the packet.
		 * \param data Buffer to write to the packet.
		 * \param whole If \c true, indicates that write size must equal the length of \p data
		 * \return Number of bytes written.
		 */
		template <size_t _size>
		size_t write_data(const std::array<uint8_t, _size>& data, bool whole);

		/**
		 * \brief Writes a string to the packet with 16-bit integer length.
		 * \param data Source.
		 * \return Number of bytes written.
		 */
		size_t write(const std::string& data);

		/**
		 * \brief Writes a byte into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const int8_t& data);

		/**
		 * \brief Writes a byte into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const uint8_t& data);

		/**
		 * \brief Writes a \c bool into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 * \remark \c sws::Packet treats \c bool as one byte on all platforms.
		 */
		size_t write(const bool& data);

		/**
		 * \brief Writes a 16-bit integer into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const int16_t& data);

		/**
		 * \brief Writes a 16-bit integer into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const uint16_t& data);

		/**
		 * \brief Writes a 32-bit integer into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const int32_t& data);

		/**
		 * \brief Writes a 32-bit integer into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const uint32_t& data);

		/**
		 * \brief Writes a 64-bit integer into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const int64_t& data);

		/**
		 * \brief Writes a 64-bit integer into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const uint64_t& data);

		/**
		 * \brief Writes a 32-bit floating point number into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const float& data);

		/**
		 * \brief Writes a 64-bit floating point number into the packet.
		 * \param data Data to write.
		 * \return \c 0 on failure, \p data's size on success.
		 */
		size_t write(const double& data);

		Packet& operator>>(std::string& data);
		Packet& operator<<(const std::string& data);

		Packet& operator>>(bool& data);

		Packet& operator>>(int8_t& data);

		Packet& operator>>(uint8_t& data);

		Packet& operator>>(int16_t& data);

		Packet& operator>>(uint16_t& data);

		Packet& operator>>(int32_t& data);

		Packet& operator>>(uint32_t& data);

		Packet& operator>>(int64_t& data);

		Packet& operator>>(uint64_t& data);

		Packet& operator>>(float& data);

		Packet& operator>>(double& data);

		Packet& operator<<(const bool& data);

		Packet& operator<<(const int8_t& data);

		Packet& operator<<(const uint8_t& data);

		Packet& operator<<(const int16_t& data);

		Packet& operator<<(const uint16_t& data);

		Packet& operator<<(const int32_t& data);

		Packet& operator<<(const uint32_t& data);

		Packet& operator<<(const int64_t& data);

		Packet& operator<<(const uint64_t& data);

		Packet& operator<<(const float& data);

		Packet& operator<<(const double& data);

		/**
		 * \brief Clears the internal buffer.
		 */
		virtual void clear();

		bool full() const;
		bool empty() const;
		bool end() const;
		size_t work_size() const;
		size_t real_size() const;
		bool verify_size() const;

		/**
		 * \brief Resizes the internal buffer.
		 * \param size New size in bytes.
		 */
		void resize(size_t size);

		/**
		 * \brief Shrinks the internal buffer to fit its data.
		 */
		void shrink_to_fit();

		const std::vector<uint8_t>& data_vector() const;

	protected:
		void update_size();

		ptrdiff_t send_remainder() const;
		ptrdiff_t recv_remainder() const;

		uint8_t* send_data();
		uint8_t* recv_data();

		void send_reset();
		void recv_reset();

		void seek_impl(SeekType type, ptrdiff_t& pos, ptrdiff_t value) const;

		template <typename T>
		size_t read_impl(T* data);

		template <typename T>
		void read_enforced(T* data);

		template <typename T>
		size_t write_impl(const T& data);

		template <typename T>
		void write_enforced(const T& data);
	};

	template <size_t _size>
	size_t Packet::read_data(std::array<uint8_t, _size>& data, bool whole)
	{
		return read_data(data.data(), data.size(), whole);
	}

	template <size_t _size>
	size_t Packet::write_data(const std::array<uint8_t, _size>& data, bool whole)
	{
		return write_data(data.data(), data.size(), whole);
	}

	template <typename T>
	size_t Packet::read_impl(T* data)
	{
		return read_data(data, sizeof(T), true);
	}

	template <typename T>
	void Packet::read_enforced(T* data)
	{
		enforce(read_data(data, sizeof(T), true) == sizeof(T), "Failed to read data from packet.");
	}

	template <typename T>
	size_t Packet::write_impl(const T& data)
	{
		return write_data(&data, sizeof(T), true);
	}

	template <typename T>
	void Packet::write_enforced(const T& data)
	{
		enforce(write_data(&data, sizeof(T), true) == sizeof(T), "Failed to write data to packet.");
	}
}
