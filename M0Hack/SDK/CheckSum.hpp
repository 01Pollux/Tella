#pragma once

#include <type_traits>
#include <format>

using CRC32_t = uint32_t;

void CRC32_Init(CRC32_t*);
void CRC32_ProcessBuffer(CRC32_t*, const void* p, int len);
void CRC32_Final(CRC32_t*);

CRC32_t	CRC32_GetTableEntry(size_t slot);

inline CRC32_t CRC32_ProcessSingleBuffer(const void* p, int len)
{
	CRC32_t crc;

	CRC32_Init(&crc);
	CRC32_ProcessBuffer(&crc, p, len);
	CRC32_Final(&crc);

	return crc;
}


#define MD5_DIGEST_LENGTH 16  
#define MD5_BIT_LENGTH MD5_DIGEST_LENGTH * sizeof(uint8_t)

struct MD5Value
{
	uint8_t bits[MD5_BIT_LENGTH]{ };
	
	bool is_zero() const noexcept
	{
		for (const uint8_t bit : bits)
			if (bit)
				return false;
		return true;
	}

	std::string to_string() const noexcept
	{
		std::string str;
		for (auto bit : bits)
			str += std::format("{:x} ", bit);
		return str;
	}

	bool operator==(const MD5Value& src) const = default;
	bool operator!=(const MD5Value& src) const = default;
};


struct MD5Context
{
	uint32_t	Buf[4]{ };
	uint32_t	Bits[2]{ };
	uint8_t		In[64]{ };
};

void MD5Init(MD5Context* context);
void MD5Update(MD5Context* context, const uint8_t* buf, uint32_t len);
void MD5Final(uint8_t digest[MD5_DIGEST_LENGTH], MD5Context* context);

char* MD5_Print(uint8_t* digest, int hashlen);

void MD5_ProcessSingleBuffer(const void* p, int len, MD5Value& md5Result);

unsigned int MD5_PseudoRandom(uint32_t nSeed);