#include "../include/sws/Socket.h"
#include "../include/sws/Packet.h"
#include <algorithm>

namespace sws
{
	Packet::Packet()
	{
		buffer.reserve(256);
		Packet::clear(); // initializes buffer size, seek positions, etc
	}

	Packet::Packet(size_t reserve)
	{
		enforce(reserve >= sizeof(packetlen_t), "reserve size must be >= sizeof(packetlen_t)");
		buffer.reserve(reserve);
		Packet::clear(); // initializes buffer size, seek positions, etc
	}

	Packet::Packet(Packet&& other) noexcept
	{
		*this = std::move(other);
	}

	Packet& Packet::operator=(Packet&& other) noexcept
	{
		buffer      = std::move(other.buffer);
		read_pos    = other.read_pos;
		write_pos   = other.write_pos;
		send_pos    = other.send_pos;
		recv_target = other.recv_target;
		recv_pos    = other.recv_pos;

		other.send_reset();
		other.recv_reset();

		return *this;
	}

	void Packet::seek(SeekCursor cursor, SeekType type, ptrdiff_t value)
	{
		switch (cursor)
		{
			case SeekCursor::read:
				seek_impl(type, read_pos, value);
				break;

			case SeekCursor::write:
				seek_impl(type, write_pos, value);
				break;

			case SeekCursor::both:
				seek_impl(type, read_pos, value);
				seek_impl(type, write_pos, value);
				break;

			default:
				break;
		}
	}

	ptrdiff_t Packet::tell(SeekCursor cursor) const
	{
		switch (cursor)
		{
			case SeekCursor::read:
				return read_pos - sizeof(packetlen_t);

			case SeekCursor::write:
				return write_pos - sizeof(packetlen_t);

			default:
				return -1;
		}
	}

	size_t Packet::read_data(void* data, size_t size)
	{
		// TODO: enforce?
		if (read_pos + size > this->real_size())
		{
			return 0;
		}

		memcpy(data, &buffer[read_pos], size);
		read_pos += size;
		return size;
	}

	size_t Packet::read_data(std::vector<uint8_t>& data)
	{
		return read_data(data.data(), data.size());
	}

	size_t Packet::read(std::string& data)
	{
		int16_t size;
		auto result = read(size);

		if (!result)
		{
			return 0;
		}

		enforce(size >= 0, "Malformed string.");

		std::vector<uint8_t> temp(size);

		const auto char_result = read_data(temp);

		if (!char_result)
		{
			return 0;
		}

		result += char_result;
		data = std::string(reinterpret_cast<const char*>(temp.data()), 0, temp.size());
		return result;
	}

	size_t Packet::peek(void* data, size_t size)
	{
		const auto pos    = tell(SeekCursor::read);
		const auto result = read_data(data, size);
		seek(SeekCursor::read, SeekType::from_start, pos);
		return result;
	}

	size_t Packet::write_data(const void* data, size_t size)
	{
		if (full())
		{
			return 0;
		}

		if (write_pos + size > Socket::datagram_size)
		{
			return 0;
		}

		if (write_pos + size > buffer.size())
		{
			buffer.resize(write_pos + (size - (buffer.size() - write_pos)));
		}

		memcpy(&buffer[write_pos], data, size);

		ptrdiff_t old_pos = write_pos;
		write_pos += size;

		// if we're at 0, don't update the size; we're writing it now!
		if (old_pos > 0)
		{
			update_size();
		}

		return size;
	}

	size_t Packet::write_data(const std::vector<uint8_t>& data)
	{
		return write_data(data.data(), data.size());
	}

	size_t Packet::write(const std::string& data)
	{
		// arbitrarily limiting strings to this length
		enforce(data.length() <= 32767, "String too long!");

		const size_t result = write(static_cast<int16_t>(data.length())) + write_data(data.c_str(), data.length());

		enforce(result == sizeof(int16_t) + data.length(), "Failed to write whole string to packet.");

		return result;
	}

	Packet& Packet::operator>>(std::string& data)
	{
		enforce(read(data), "Failed to read string from packet.");
		return *this;
	}

	Packet& Packet::operator<<(const std::string& data)
	{
		write(data);
		return *this;
	}

	void Packet::clear()
	{
		read_pos  = sizeof(packetlen_t);
		write_pos = sizeof(packetlen_t);

		resize(sizeof(packetlen_t));
	}

	bool Packet::full() const
	{
		return buffer.size() == Socket::datagram_size;
	}

	bool Packet::empty() const
	{
		return !work_size();
	}

	size_t Packet::work_size() const
	{
		const size_t s = buffer.size();

		return s > sizeof(packetlen_t) ? s - sizeof(packetlen_t) : 0;
	}

	size_t Packet::real_size() const
	{
		return buffer.size();
	}

	bool Packet::verify_size() const
	{
		return buffer.size() >= sizeof(packetlen_t)
			   && *reinterpret_cast<const packetlen_t*>(&buffer[0]) == static_cast<packetlen_t>(work_size());
	}

	void Packet::resize(size_t size)
	{
		size = std::max(sizeof(packetlen_t), std::min(Socket::datagram_size, size));
		buffer.resize(size);
		update_size();
	}

	void Packet::shrink_to_fit()
	{
		buffer.shrink_to_fit();
	}

	const std::vector<uint8_t>& Packet::data_vector() const
	{
		return buffer;
	}

	void Packet::update_size()
	{
		const ptrdiff_t last_write = tell(SeekCursor::write);

		write_pos = 0;
		*this << static_cast<packetlen_t>(work_size());

		seek(SeekCursor::write, SeekType::from_start, last_write);
	}

	ptrdiff_t Packet::send_remainder() const
	{
		return send_pos < 1 ? 0 : buffer.size() - send_pos;
	}

	ptrdiff_t Packet::recv_remainder() const
	{
		return (recv_target < 0 || recv_pos < 0) ? 0 : recv_target - recv_pos;
	}

	uint8_t* Packet::send_data()
	{
		return &buffer[send_pos];
	}

	uint8_t* Packet::recv_data()
	{
		return &buffer[recv_pos];
	}

	void Packet::send_reset()
	{
		send_pos = -1;
	}

	void Packet::recv_reset()
	{
		recv_pos    = -1;
		recv_target = -1;
	}

	void Packet::seek_impl(SeekType type, ptrdiff_t& pos, ptrdiff_t value) const
	{
		ptrdiff_t result;

		const ptrdiff_t work_pos = pos - sizeof(packetlen_t);

		switch (type)
		{
			case SeekType::from_start:
				enforce(value >= 0, "Seek position must be non-negative.");
				enforce(static_cast<size_t>(value) <= work_size(), "Seek beyond end of buffer.");
				pos = value + sizeof(packetlen_t);
				break;

			case SeekType::relative:
				result = work_pos + value;
				enforce(result >= 0, "Seek amount places cursor below zero.");
				enforce(static_cast<size_t>(result) <= work_size(), "Seek beyond end of buffer.");
				pos = result + sizeof(packetlen_t);
				break;

			case SeekType::from_end:
				enforce(value >= 0, "Seek position must be non-negative.");

				result = static_cast<ptrdiff_t>(work_size() - value);
				enforce(result >= 0, "Seek amount places cursor below zero.");

				pos = result + sizeof(packetlen_t);
				break;

			default:
				break;
		}

		enforce(pos >= static_cast<ptrdiff_t>(sizeof(packetlen_t)), "I fucked up");
	}
}
