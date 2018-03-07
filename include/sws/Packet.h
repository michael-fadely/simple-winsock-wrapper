#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "enforce.h"
#include "typedefs.h"

namespace sws
{
	class Socket;

	enum class SeekCursor
	{
		read,
		write,
		both
	};

	enum class SeekType
	{
		from_start,
		relative,
		from_end
	};

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
		explicit Packet(size_t   reserve);
		Packet(Packet&  ) = default;
		Packet(Packet&& other) noexcept;
		virtual ~Packet() = default;

		Packet& operator=(Packet&& other) noexcept;

		void seek(SeekCursor cursor, SeekType type, ptrdiff_t value);
		ptrdiff_t tell(SeekCursor cursor) const;

		size_t read_data(void* data, size_t size);
		size_t read_data(std::vector<uint8_t>& data);

		template <size_t _size>
		size_t read_data(std::array<uint8_t, _size>& data);

		size_t read(std::string& data);

		inline size_t read(int8_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(uint8_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(int16_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(uint16_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(int32_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(uint32_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(int64_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(uint64_t& data)
		{
			return read_impl(&data);
		}

		inline size_t read(float& data)
		{
			return read_impl(&data);
		}

		inline size_t read(double& data)
		{
			return read_impl(&data);
		}

		size_t peek(void* data, size_t size);

		inline size_t peek(std::vector<uint8_t>& data)
		{
			return peek(data.data(), data.size());
		}

		size_t write_data(const void* data, size_t size);
		size_t write_data(const std::vector<uint8_t>& data);

		template <size_t _size>
		size_t write_data(const std::array<uint8_t, _size>& data);

		size_t write(const std::string& data);

		inline size_t write(const int8_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const uint8_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const int16_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const uint16_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const int32_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const uint32_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const int64_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const uint64_t& data)
		{
			return write_impl(data);
		}

		inline size_t write(const float& data)
		{
			return write_impl(data);
		}

		inline size_t write(const double& data)
		{
			return write_impl(data);
		}

		Packet& operator>>(std::string& data);
		Packet& operator<<(const std::string& data);

		inline Packet& operator>>(int8_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(uint8_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(int16_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(uint16_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(int32_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(uint32_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(int64_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(uint64_t& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(float& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator>>(double& data)
		{
			read_enforced(&data);
			return *this;
		}

		inline Packet& operator<<(const int8_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const uint8_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const int16_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const uint16_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const int32_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const uint32_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const int64_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const uint64_t& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const float& data)
		{
			write_enforced(data);
			return *this;
		}

		inline Packet& operator<<(const double& data)
		{
			write_enforced(data);
			return *this;
		}

		virtual void clear();

		bool full() const;
		bool empty() const;
		size_t work_size() const;
		size_t real_size() const;
		bool verify_size() const;

		void resize(size_t size);
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
	size_t Packet::read_data(std::array<uint8_t, _size>& data)
	{
		return read_data(data.data(), data.size());
	}

	template <size_t _size>
	size_t Packet::write_data(const std::array<uint8_t, _size>& data)
	{
		return write_data(data.data(), data.size());
	}

	template <typename T>
	size_t Packet::read_impl(T* data)
	{
		return read_data(data, sizeof(T));
	}

	template <typename T>
	void Packet::read_enforced(T* data)
	{
		enforce(read_data(data, sizeof(T)) == sizeof(T), "Failed to read data from packet.");
	}

	template <typename T>
	size_t Packet::write_impl(const T& data)
	{
		return write_data(&data, sizeof(T));
	}

	template <typename T>
	void Packet::write_enforced(const T& data)
	{
		enforce(write_data(&data, sizeof(T)) == sizeof(T), "Failed to write data to packet.");
	}
}
