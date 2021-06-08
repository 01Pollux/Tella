#include "BitBuffers.hpp"

struct bit_buffer_constants
{
	static constexpr int maxbytes_var_int32 = 5;
	static constexpr int max_var = 10;

	static constexpr int coord_int = 14;
	static constexpr int coord_fraction = 5;
	static constexpr int coord_denominator = 1 << coord_fraction;
	static constexpr float coord_resolution = 1.f / (1 << 5);

	static constexpr uint32_t enconde_zigzag(int32_t v) { return (v << 1) ^ (v >> 31); }
	static constexpr uint64_t enconde_zigzag(int64_t v) { return (v << 1) ^ (v >> 63); }

	static constexpr uint32_t deconde_zigzag(uint32_t v) { return (v >> 1) ^ -static_cast<int32_t>(v & 1); }
	static constexpr uint64_t deconde_zigzag(uint64_t v) { return (v >> 1) ^ -static_cast<int64_t>(v & 1); }

	static constexpr uint32_t bit_for_bitnum(int bit)
	{
		constexpr uint32_t bitsForBitnum[]
		{
			(1 << 0),
			(1 << 1),
			(1 << 2),
			(1 << 3),
			(1 << 4),
			(1 << 5),
			(1 << 6),
			(1 << 7),
			(1 << 8),
			(1 << 9),
			(1 << 10),
			(1 << 11),
			(1 << 12),
			(1 << 13),
			(1 << 14),
			(1 << 15),
			(1 << 16),
			(1 << 17),
			(1 << 18),
			(1 << 19),
			(1 << 20),
			(1 << 21),
			(1 << 22),
			(1 << 23),
			(1 << 24),
			(1 << 25),
			(1 << 26),
			(1 << 27),
			(1 << 28),
			(1 << 29),
			(1 << 30),
			(1 << 31),
		};
		static_assert(std::extent_v<decltype(bitsForBitnum)> == 32);
		return bitsForBitnum[bit & 31];
	}

	uint32_t bit_write_bitmask[32][33]{ };
	uint32_t bit_write_littlemask[32]{ };

	constexpr bit_buffer_constants()
	{
		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 33; j++)
			{
				unsigned int k = i + j;

				bit_write_bitmask[i][j] = bit_for_bitnum(i) - 1;
				if (k < 32)
					bit_write_bitmask[i][j] |= ~(bit_for_bitnum(k) - 1);
			}
		}

		for (size_t i = 0; i < 32; i++)
			bit_write_littlemask[i] = 1u << i;
	}
};

static constexpr bit_buffer_constants bit_buffer_c;


void bf_write::start_writing(void* pData, int nBytes, int iStartBit, int nBits)
{
	nBytes &= ~3;

	Data = static_cast<decltype(Data)>(pData);
	DataBytes = nBytes;

	if (nBits == -1)
		DataBits = nBytes << 3;
	else
		DataBits = nBits;

	CurBit = iStartBit;
	IsOverflow = false;
}


void bf_write::write_bit_nocheck(int bit)
{
	if (bit)
		Data[CurBit >> 5] |= 1u << (CurBit & 31);
	else
		Data[CurBit >> 5] &= ~(1u << (CurBit & 31));

	++CurBit;
}


void bf_write::write_bit_at(int bit_pos, int bit)
{
	if (!check_for_overflow(bit_pos))
	{
		if (bit)
			Data[bit_pos >> 5] |= 1u << (bit_pos & 31);
		else
			Data[bit_pos >> 5] &= ~(1u << (bit_pos & 31));
	}
}


void bf_write::write_ubit(unsigned int curData, int numbits)
{
	if (bits_left() < numbits)
	{
		CurBit = DataBits;
		mark_as_overflowed();
		return;
	}

	int mask = CurBit & 31;
	int val = CurBit >> 5;
	CurBit += numbits;

	// Mask in a dword.
	unsigned long* pOut = &Data[val];

	// Rotate data into dword alignment
	curData = (curData << mask) | (curData >> (32 - mask));

	// Calculate bitmasks for first and second word
	unsigned int temp = 1 << (numbits - 1);
	unsigned int mask1 = (temp * 2 - 1) << mask;
	unsigned int mask2 = (temp - 1) >> (31 - mask);

	// Only look beyond current word if necessary (avoid access violation)
	int i = mask2 & 1;
	unsigned long dword1 = pOut[0];
	unsigned long dword2 = pOut[i];

	// Drop bits into place
	dword1 ^= (mask1 & (curData ^ dword1));
	dword2 ^= (mask2 & (curData ^ dword2));

	// Note reversed order of writes so that dword1 wins if mask2 == 0 && i == 0
	pOut[i] = dword2;
	pOut[0] = dword1;
}


void bf_write::write_sbit(int data, int numbits)
{
	// Force the sign-extension bit to be correct even in the case of overflow.
	int nValue = data;
	int nPreserveBits = (0x7FFFFFFF >> (32 - numbits));
	int nSignExtension = (nValue >> 31) & ~nPreserveBits;
	nValue &= nPreserveBits;
	nValue |= nSignExtension;

	write_ubit(nValue, numbits);
}


void bf_write::write_uint32(uint32_t data)
{
	// Check if align and we have room, slow path if not
	if ((CurBit & 7) == 0 && (CurBit + bit_buffer_constants::maxbytes_var_int32 * 8) <= DataBits)
	{
		uint8_t* target = this->data() + (CurBit >> 3);

		target[0] = data | 0x80;
		if (data >= (1 << 7))
		{
			target[1] = (data >> 7) | 0x80;
			if (data >= (1 << 14))
			{
				target[2] = (data >> 14) | 0x80;
				if (data >= (1 << 21))
				{
					target[3] = (data >> 21) | 0x80;
					if (data >= (1 << 28))
					{
						target[4] = data >> 28;
						CurBit += 5 * 8;
						return;
					}
					else
					{
						target[3] &= 0x7F;
						CurBit += 4 * 8;
						return;
					}
				}
				else
				{
					target[2] &= 0x7F;
					CurBit += 3 * 8;
					return;
				}
			}
			else
			{
				target[1] &= 0x7F;
				CurBit += 2 * 8;
				return;
			}
		}
		else
		{
			target[0] &= 0x7F;
			CurBit += 1 * 8;
			return;
		}
	}
	else // Slow path
	{
		while (data > 0x7F)
		{
			write_ubit((data & 0x7F) | 0x80, 8);
			data >>= 7;
		}
		write_ubit(data & 0x7F, 8);
	}
}


void bf_write::write_uint64(uint64_t data)
{
	// Check if align and we have room, slow path if not
	if ((CurBit & 7) == 0 && (CurBit + bit_buffer_constants::max_var * 8) <= DataBits)
	{
		uint8_t* target = this->data() + (CurBit >> 3);

		// Splitting into 32-bit pieces gives better performance on 32-bit
		// processors.
		uint32_t part0 = static_cast<uint32_t>(data);
		uint32_t part1 = static_cast<uint32_t>(data >> 28);
		uint32_t part2 = static_cast<uint32_t>(data >> 56);

		int size;

		// Here we can't really optimize for small numbers, since the data is
		// split into three parts.  Cheking for numbers < 128, for instance,
		// would require three comparisons, since you'd have to make sure part1
		// and part2 are zero.  However, if the caller is using 64-bit integers,
		// it is likely that they expect the numbers to often be very large, so
		// we probably don't want to optimize for small numbers anyway.  Thus,
		// we end up with a hardcoded binary search tree...
		if (part2 == 0)
		{
			if (part1 == 0)
			{
				if (part0 < (1 << 14))
				{
					if (part0 < (1 << 7))
					{
						size = 1; goto size1;
					}
					else
					{
						size = 2; goto size2;
					}
				}
				else
				{
					if (part0 < (1 << 21))
					{
						size = 3; goto size3;
					}
					else
					{
						size = 4; goto size4;
					}
				}
			}
			else
			{
				if (part1 < (1 << 14))
				{
					if (part1 < (1 << 7))
					{
						size = 5; goto size5;
					}
					else
					{
						size = 6; goto size6;
					}
				}
				else
				{
					if (part1 < (1 << 21))
					{
						size = 7; goto size7;
					}
					else
					{
						size = 8; goto size8;
					}
				}
			}
		}
		else
		{
			if (part2 < (1 << 7))
			{
				size = 9; goto size9;
			}
			else
			{
				size = 10; goto size10;
			}
		}

	size10:	target[9] = static_cast<uint8_t>((part2 >> 7) | 0x80);
	size9:	target[8] = static_cast<uint8_t>((part2) | 0x80);
	size8:	target[7] = static_cast<uint8_t>((part1 >> 21) | 0x80);
	size7:	target[6] = static_cast<uint8_t>((part1 >> 14) | 0x80);
	size6:	target[5] = static_cast<uint8_t>((part1 >> 7) | 0x80);
	size5:	target[4] = static_cast<uint8_t>((part1) | 0x80);
	size4:	target[3] = static_cast<uint8_t>((part0 >> 21) | 0x80);
	size3:	target[2] = static_cast<uint8_t>((part0 >> 14) | 0x80);
	size2:	target[1] = static_cast<uint8_t>((part0 >> 7) | 0x80);
	size1:	target[0] = static_cast<uint8_t>((part0) | 0x80);

		target[size - 1] &= 0x7F;
		CurBit += size * 8;
	}
	else // slow path
	{
		while (data > 0x7F)
		{
			write_ubit((data & 0x7F) | 0x80, 8);
			data >>= 7;
		}
		write_ubit(data & 0x7F, 8);
	}
}


void bf_write::write_sint32(int32_t data)
{
	write_uint32(bit_buffer_constants::enconde_zigzag(data));
}


void bf_write::write_sint64(int64_t data)
{
	write_uint64(bit_buffer_constants::enconde_zigzag(data));
}


bool bf_write::write_bits(const void* in_data, int numbits)
{
	const uint8_t* pOut = static_cast<const uint8_t*>(in_data);
	int bits_left = numbits;

	if (check_for_overflow(numbits))
		return false;

	// Align output to dword boundary
	while (((unsigned long)pOut & 3) != 0 && bits_left >= 8)
	{
		write_ubit(*pOut, 8);
		++pOut;
		bits_left -= 8;
	}

	if ((bits_left >= 32) && (CurBit & 7) == 0)
	{
		int numbytes = bits_left >> 3;
		int numbits = numbytes << 3;

		std::copy_n(pOut, numbytes, this->data() + (CurBit >> 3));

		pOut += numbytes;
		bits_left -= numbits;
		CurBit += numbits;
	}

	if (bits_left >= 32)
	{
		unsigned long iBitsRight = (CurBit & 31);
		unsigned long iBitsLeft = 32 - iBitsRight;
		unsigned long bitMaskLeft = bit_buffer_c.bit_write_bitmask[iBitsRight][32];
		unsigned long bitMaskRight = bit_buffer_c.bit_write_bitmask[0][iBitsRight];

		unsigned long* pData = &Data[CurBit >> 5];

		// Read dwords.
		while (bits_left >= 32)
		{
			unsigned long curData = *reinterpret_cast<const unsigned long*>(pOut);
			pOut += sizeof(unsigned long);

			*pData &= bitMaskLeft;
			*pData |= curData << iBitsRight;

			pData++;

			if (iBitsLeft < 32)
			{
				curData >>= iBitsLeft;
				*pData &= bitMaskRight;
				*pData |= curData;
			}

			bits_left -= 32;
			CurBit += 32;
		}
	}


	// write remaining bytes
	while (bits_left >= 8)
	{
		write_ubit(*pOut, 8);
		++pOut;
		bits_left -= 8;
	}

	// write remaining bits
	if (bits_left)
		write_ubit(*pOut, bits_left);
	
	return has_overflown();
}


void bf_write::write_angle(float fAngle, int numbits)
{
	uint32_t shift = bit_buffer_constants::bit_for_bitnum(numbits);
	uint32_t mask = shift - 1;

	uint32_t d = static_cast<uint32_t>((fAngle / 360.0) * shift) & mask;

	write_ubit(d, numbits);
}


void bf_write::write_coord(const float f)
{
	int	signbit = (f <= -bit_buffer_constants::coord_resolution);
	int	intval = static_cast<int>(abs(f));
	int	fractval = abs(static_cast<int>((f * bit_buffer_constants::coord_denominator)) & (bit_buffer_constants::coord_denominator - 1));

	// Send the bit flags that indicate whether we have an integer part and/or a fraction part.
	write_bit(intval);
	write_bit(fractval);
	
	if (intval || fractval)
	{
		// Send the sign bit
		write_bit(signbit);

		// Send the integer if we have one.
		if (intval)
		{
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			write_ubit(static_cast<uint32_t>(intval), bit_buffer_constants::coord_int);
		}

		// Send the fraction if we have one
		if (fractval)
		{
			write_ubit(static_cast<uint32_t>(fractval), bit_buffer_constants::coord_int);
		}
	}
}


void bf_write::write_vec3(const Vector& fa)
{
	int xflag = (fa[0] >= bit_buffer_constants::coord_resolution) || (fa[0] <= -bit_buffer_constants::coord_resolution);
	int yflag = (fa[1] >= bit_buffer_constants::coord_resolution) || (fa[1] <= -bit_buffer_constants::coord_resolution);
	int zflag = (fa[2] >= bit_buffer_constants::coord_resolution) || (fa[2] <= -bit_buffer_constants::coord_resolution);

	write_bit(xflag);
	write_bit(yflag);
	write_bit(zflag);

	if (xflag)	write_coord(fa[0]);
	if (yflag)	write_coord(fa[1]);
	if (zflag)	write_coord(fa[2]);
}


void bf_write::write_char(int8_t val)
{
	write_sbit(val, sizeof(char) << 3);
}


void bf_write::write_byte(uint8_t val)
{
	write_ubit(val, sizeof(int8_t) << 3);
}


void bf_write::write_short(int16_t val)
{
	write_sbit(val, sizeof(int16_t) << 3);
}


void bf_write::write_word(uint16_t val)
{
	write_ubit(val, sizeof(uint16_t) << 3);
}


void bf_write::write_long(int32_t val)
{
	write_sbit(val, sizeof(int32_t) << 3);
}


void bf_write::write_longlong(int64_t val)
{
	uint32_t* longs = reinterpret_cast<uint32_t*>(&val);

	// Insert the two DWORDS according to network endian
	const short endianIndex = 0x0100;
	const int8_t* idx = reinterpret_cast<const int8_t*>(&endianIndex);
	write_ubit(longs[*idx++], sizeof(int32_t) << 3);
	write_ubit(longs[*idx], sizeof(int32_t) << 3);
}


bool bf_write::write_string(const char* str)
{
	if (str)
	{
		do
		{
			write_char(*str);
			++str;
		} while (*(str - 1) != 0);
	}
	else
		write_char(0);

	return has_overflown();
}


void bf_read::start_reading(const void* pData, int nBytes, int iStartBit, int nBits)
{
	Data = static_cast<const uint8_t*>(pData);
	DataBytes = nBytes;

	if (nBits == -1)
		DataBits = DataBytes << 3;
	else
		DataBits = nBits;

	CurBit = iStartBit;
	IsOverflow = false;
}



int bf_read::read_bit()
{
	if (bits_left() <= 0)
	{
		mark_as_overflowed();
		return 0;
	}

	unsigned int value = ((unsigned long*)Data)[CurBit >> 5] >> (CurBit & 31);
	++CurBit;
	return value & 1;
}


void bf_read::read_bits(void* data, int nBits)
{
	int8_t* pOut = static_cast<int8_t*>(data);
	int nBitsLeft = nBits;

	// align output to dword boundary
	while (((size_t)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		*pOut = read_ubit(8);
		++pOut;
		nBitsLeft -= 8;
	}

		// read dwords
	while (nBitsLeft >= 32)
	{
		*((unsigned long*)pOut) = read_ubit(32);
		pOut += sizeof(unsigned long);
		nBitsLeft -= 32;
	}

	// read remaining bytes
	while (nBitsLeft >= 8)
	{
		*pOut = read_ubit(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// read remaining bits
	if (nBitsLeft)
		*pOut = read_ubit(nBitsLeft);
}


float bf_read::read_angle(int numbits)
{
	float shift = static_cast<float>((bit_buffer_constants::bit_for_bitnum(numbits)));
	return static_cast<float>(read_ubit(numbits)) * (360.0f / shift);
}


unsigned int bf_read::peek_ubit(int numbits)
{
	bf_read savebf = *this;

	uint32_t r = 0;
	for (int i = 0; i < numbits; i++)
	{
		int val = read_bit();

		// Append to current stream
		if (val)
			r |= bit_buffer_constants::bit_for_bitnum(i);
	}

	*this = savebf;

	return r;
}


uint32_t bf_read::read_ubit(int numbits)
{
	if (bits_left() < numbits)
	{
		CurBit = DataBits;
		mark_as_overflowed();
		return 0;
	}

	unsigned int iStartBit = CurBit & 31u;
	int iLastBit = CurBit + numbits - 1;
	unsigned int iWordOffset1 = CurBit >> 5;
	unsigned int iWordOffset2 = iLastBit >> 5;

	CurBit += numbits;

	unsigned int bitmask = (2 << (numbits - 1)) - 1;

	unsigned int dw1 = Data[iWordOffset1] >> iStartBit;
	unsigned int dw2 = Data[iWordOffset2] << (32 - iStartBit);

	return (dw1 | dw2) & bitmask;
}


// Append numbits least significant bits from data to the current bit stream
int32_t bf_read::read_sbit(int numbits)
{
	uint32_t r = read_ubit(numbits);
	uint32_t s = 1 << (numbits - 1);
	if (r >= s)
	{
		// sign-extend by removing sign bit and then subtracting sign bit again
		r = r - s - s;
	}
	return r;
}


uint32_t bf_read::read_uint32()
{
	uint32_t result = 0;
	int count = 0;
	uint32_t b;

	do
	{
		if (count == bit_buffer_constants::maxbytes_var_int32)
			return result;

		b = read_ubit(8);
		result |= (b & 0x7F) << (7 * count);
		++count;
	} while (b & 0x80);

	return result;
}


uint64_t bf_read::read_uint64()
{
	uint64_t result = 0;
	int count = 0;
	uint64_t b;

	do
	{
		if (count == bit_buffer_constants::max_var)
			return result;

		b = read_ubit(8);
		result |= static_cast<uint64_t>(b & 0x7F) << (7 * count);
		++count;
	} while (b & 0x80);

	return result;
}


int32_t bf_read::read_int32()
{
	uint32_t value = read_uint32();
	return bit_buffer_constants::deconde_zigzag(value);
}


int64_t bf_read::read_int64()
{
	uint64_t value = read_uint64();
	return bit_buffer_constants::deconde_zigzag(value);
}


// Basic Coordinate Routines (these contain bit-field size AND fixed point scaling constants)
float bf_read::read_coord()
{
	float value = 0.0;

	// Read the required integer and fraction flags
	int intval = read_bit();
	int fractval = read_bit();

	// If we got either parse them, otherwise it's a zero.
	if (intval || fractval)
	{
		// Read the sign bit
		int signbit = read_bit();

		// If there's an integer, read it in
		// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
		if (intval)
			intval = read_ubit(bit_buffer_constants::coord_int) + 1;

		// If there's a fraction, read it in
		if (fractval)
			fractval = read_ubit(bit_buffer_constants::coord_fraction);

		// Calculate the correct floating point value
		value = intval + ((float)fractval * bit_buffer_constants::coord_resolution);

		// Fixup the sign if negative.
		if (signbit)
			value = -value;
	}

	return value;
}


void bf_read::read_vec3(Vector& fa)
{
	fa = NULL_VECTOR;

	int xflag = read_bit();
	int yflag = read_bit();
	int zflag = read_bit();

	if (xflag)
		fa[0] = read_coord();
	if (yflag)
		fa[1] = read_coord();
	if (zflag)
		fa[2] = read_coord();
}


int64_t bf_read::read_longlong()
{
	int64_t ret;
	uint32_t* longs = (uint32_t*)&ret;

	// Read the two DWORDs according to network endian
	const short endianIndex = 0x0100;
	const int8_t* idx = reinterpret_cast<const int8_t*>(&endianIndex);
	longs[*idx++] = read_ubit(sizeof(int32_t) << 3);
	longs[*idx] = read_ubit(sizeof(int32_t) << 3);

	return ret;
}


bool bf_read::read_string(char* str, int maxLen, bool bLine, int* pOutNumChars)
{
	bool bTooSmall = false;
	int i = 0;

	while (1)
	{
		char val = read_char();
		if (val == 0)
			break;
		else if (bLine && val == '\n')
			break;

		if (i < (maxLen - 1))
		{
			str[i] = val;
			++i;
		}
		else
		{
			bTooSmall = true;
		}
	}

	str[i] = 0;

	if (pOutNumChars)
		*pOutNumChars = i;

	return !has_overflown() && !bTooSmall;
}