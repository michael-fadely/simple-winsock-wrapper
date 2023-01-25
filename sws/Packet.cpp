#include "../include/sws/Socket.h"
#include "../include/sws/Packet.h"
#include <algorithm>

namespace sws
{
	Packet::Packet()
	{
		data_.reserve(256);
		Packet::clear(); // initializes buffer size, seek positions, etc
	}

	Packet::Packet(size_t reserve)
	{
		enforce(reserve >= sizeof(packetlen_t), "reserve size must be >= sizeof(packetlen_t)");
		data_.reserve(reserve);
		Packet::clear(); // initializes buffer size, seek positions, etc
	}

	Packet::Packet(Packet&& other) noexcept
		: data_(std::move(other.data_)),
		  read_pos_(other.read_pos_),
		  write_pos_(other.write_pos_),
		  send_pos_(other.send_pos_),
		  recv_pos_(other.recv_pos_),
		  recv_target_(other.recv_target_)
	{
		other.send_reset();
		other.recv_reset();
	}

	Packet& Packet::operator=(Packet&& other) noexcept
	{
		if (this != &other)
		{
			data_        = std::move(other.data_);
			read_pos_    = other.read_pos_;
			write_pos_   = other.write_pos_;
			send_pos_    = other.send_pos_;
			recv_pos_    = other.recv_pos_;
			recv_target_ = other.recv_target_;

			other.send_reset();
			other.recv_reset();
		}

		return *this;
	}

	void Packet::seek(SeekCursor cursor, SeekType type, ptrdiff_t value)
	{
		switch (cursor)
		{
			case SeekCursor::read:
				seek_impl(type, read_pos_, value);
				break;

			case SeekCursor::write:
				seek_impl(type, write_pos_, value);
				break;

			case SeekCursor::both:
				seek_impl(type, read_pos_, value);
				seek_impl(type, write_pos_, value);
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
				return read_pos_ - sizeof(packetlen_t);

			case SeekCursor::write:
				return write_pos_ - sizeof(packetlen_t);

			default:
				return -1;
		}
	}

	size_t Packet::read_data(void* data, size_t size, bool whole)
	{
		if (empty())
		{
			return 0;
		}

		auto read_size = std::min(real_size() - read_pos_, size);

		if (!read_size || (whole && read_size < size))
		{
			return 0;
		}

		memcpy(data, &data_[read_pos_], read_size);
		read_pos_ += read_size;
		return read_size;
	}

	size_t Packet::read_data(std::vector<uint8_t>& data, bool whole)
	{
		return read_data(data.data(), data.size(), whole);
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

		const auto char_result = read_data(temp, true);

		if (!char_result)
		{
			return 0;
		}

		result += char_result;
		data = std::string(reinterpret_cast<const char*>(temp.data()), 0, temp.size());
		return result;
	}

	size_t Packet::read(bool& data)
	{
		if constexpr (sizeof(bool) == 1)
		{
			return read_impl(&data);
		}
		else
		{
			uint8_t temp = 0;
			auto result = read_impl(&temp);

			if (result == sizeof(bool))
			{
				enforce(temp <= 1, "bool value out of range");
				data = temp == 1;
			}

			return result;
		}
	}

	size_t Packet::read(int8_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(uint8_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(int16_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(uint16_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(int32_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(uint32_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(int64_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(uint64_t& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(float& data)
	{
		return read_impl(&data);
	}

	size_t Packet::read(double& data)
	{
		return read_impl(&data);
	}

	size_t Packet::write_data(const void* data, size_t size, bool whole)
	{
		if (full())
		{
			return 0;
		}

		size_t write_end = write_pos_ + size;

		if (whole && write_end > Socket::datagram_size)
		{
			return 0;
		}

		const size_t write_size = std::min(Socket::datagram_size - write_pos_, size);
		write_end = write_pos_ + write_size;

		if (write_end > data_.size())
		{
			data_.resize(data_.size() + (write_end - data_.size()));
		}

		memcpy(&data_[write_pos_], data, write_size);

		const ptrdiff_t old_pos = write_pos_;
		write_pos_ += write_size;

		// if we're at 0, don't update the size; we're writing it now!
		if (old_pos > 0)
		{
			update_size();
		}

		return write_size;
	}

	size_t Packet::write_data(const std::vector<uint8_t>& data, bool whole)
	{
		return write_data(data.data(), data.size(), whole);
	}

	size_t Packet::write(const std::string& data)
	{
		// arbitrarily limiting strings to this length
		enforce(data.length() <= 32767, "String too long!");

		const size_t result = write(static_cast<int16_t>(data.length())) + write_data(data.c_str(), data.length(), true);

		enforce(result == sizeof(int16_t) + data.length(), "Failed to write whole string to packet.");

		return result;
	}

	size_t Packet::write(const int8_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const uint8_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const bool& data)
	{
		if constexpr (sizeof(bool) == 1)
		{
			return write_impl(data);
		}
		else
		{
			return write_impl(static_cast<uint8_t>(data));
		}
	}

	size_t Packet::write(const int16_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const uint16_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const int32_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const uint32_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const int64_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const uint64_t& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const float& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const double& data)
	{
		return write_impl(data);
	}

	size_t Packet::write(const Packet& packet)
	{
		if (packet.empty())
		{
			return 0;
		}

		return write_data(&packet.data().at(sizeof(packetlen_t)), packet.work_size(), true);
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

	Packet& Packet::operator>>(bool& data)
	{
		uint8_t temp = 0;
		*this >> temp;
		enforce(temp <= 1, "bool value out of range");
		data = temp == 1;

		return *this;
	}

	Packet& Packet::operator>>(char& data)
	{
		static_assert(sizeof(char) == 1, "non-byte sized char detected");
		return *this >> *reinterpret_cast<int8_t*>(&data);
	}

	Packet& Packet::operator>>(int8_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(uint8_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(int16_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(uint16_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(int32_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(uint32_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(int64_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(uint64_t& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(float& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator>>(double& data)
	{
		read_enforced(&data);
		return *this;
	}

	Packet& Packet::operator<<(const bool& data)
	{
		if constexpr (sizeof(bool) == 1)
		{
			write_enforced(data);
			return *this;
		}
		else
		{
			uint8_t value = data ? 1 : 0;
			return *this << value;
		}
	}

	Packet& Packet::operator<<(const char& data)
	{
		static_assert(sizeof(char) == 1, "non-byte sized char detected");
		return *this << *reinterpret_cast<const int8_t*>(&data);
	}

	Packet& Packet::operator<<(const int8_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const uint8_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const int16_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const uint16_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const int32_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const uint32_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const int64_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const uint64_t& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const float& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const double& data)
	{
		write_enforced(data);
		return *this;
	}

	Packet& Packet::operator<<(const Packet& packet)
	{
		auto result = write(packet);

		enforce(result == packet.work_size(), "Failed to write packet data to packet instance.");

		return *this;
	}

	void Packet::clear()
	{
		read_pos_  = sizeof(packetlen_t);
		write_pos_ = sizeof(packetlen_t);

		resize(sizeof(packetlen_t));
	}

	bool Packet::full() const
	{
		return data_.size() == Socket::datagram_size;
	}

	bool Packet::empty() const
	{
		return !work_size();
	}

	bool Packet::end() const
	{
		return static_cast<size_t>(read_pos_) == real_size();
	}

	size_t Packet::work_size() const
	{
		const size_t s = data_.size();

		return s > sizeof(packetlen_t) ? s - sizeof(packetlen_t) : 0;
	}

	size_t Packet::real_size() const
	{
		return data_.size();
	}

	bool Packet::verify_size() const
	{
		return data_.size() >= sizeof(packetlen_t) &&
		       *reinterpret_cast<const packetlen_t*>(data_.data()) == static_cast<packetlen_t>(work_size());
	}

	void Packet::resize(size_t size)
	{
		size = std::max(sizeof(packetlen_t), std::min(Socket::datagram_size, size));
		data_.resize(size);
		update_size();
	}

	void Packet::shrink_to_fit()
	{
		data_.shrink_to_fit();
	}

	const std::vector<uint8_t>& Packet::data() const
	{
		return data_;
	}

	void Packet::update_size()
	{
		const ptrdiff_t last_write = tell(SeekCursor::write);

		write_pos_ = 0;
		*this << static_cast<packetlen_t>(work_size());

		seek(SeekCursor::write, SeekType::from_start, last_write);
	}

	ptrdiff_t Packet::get_send_remainder() const
	{
		return send_pos_ < 1 ? 0 : data_.size() - send_pos_;
	}

	ptrdiff_t Packet::get_recv_remainder() const
	{
		if (recv_target_ < 0 || recv_pos_ < static_cast<ptrdiff_t>(sizeof(packetlen_t)))
		{
			return 0;
		}

		return recv_target_ - (recv_pos_ - sizeof(packetlen_t));
	}

	uint8_t* Packet::get_send_data()
	{
		return &data_[send_pos_];
	}

	uint8_t* Packet::get_recv_data()
	{
		return &data_[recv_pos_];
	}

	void Packet::send_reset()
	{
		send_pos_ = -1;
	}

	void Packet::recv_reset()
	{
		recv_pos_    = -1;
		recv_target_ = -1;
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
