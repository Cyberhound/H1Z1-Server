#include "RC4.h"



CRC4::CRC4(unsigned char *key, unsigned int keySize)
{
	Initialize(key, keySize);
}

CRC4::CRC4() :
	sbox_free(0)
{}


void CRC4::Initialize(unsigned char *key, unsigned int keySize)
{
	if (keySize > 256) /* key too big */
		throw std::exception();

	for (int i = 0; i < sizeof(sbox); i++)
		sbox[i] = (unsigned char)i;

	for (int i = 0, j = 0; i < sizeof(this->key); i++)
	{
		j = (j + key[i % keySize] + sbox[i]) % 256;
		std::swap(sbox[i], sbox[j]);
	}

	*(unsigned int *)this->key = keySize;
	memcpy(this->key + sizeof(unsigned int), key, keySize);

	sbox_free = 1;
}


void CRC4::Encrypt(unsigned char *data, unsigned long dataSize)
{
	if (!sbox_free)
		Initialize((unsigned char *)key + 4, *(unsigned int *)key);

	int i = 0;
	int j = 0;

	for (size_t n = 0, len = dataSize; n < len; n++)
	{
		i = (i + 1) % 256;
		j = (j + sbox[i]) % 256;

		std::swap(sbox[i], sbox[j]);
		int rnd = sbox[(sbox[i] + sbox[j]) % 256];

		data[n] = rnd ^ data[n];
	}

	sbox_free = 0;
}


std::string CRC4::CopyEncrypt(unsigned char *data, unsigned long dataSize)
{
	std::string result((char *)data, dataSize);
	Encrypt((unsigned char *)result.c_str(), dataSize);
	return result;
}