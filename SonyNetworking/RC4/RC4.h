#ifndef RC4_H
#define RC4_H
#include <string>



class CRC4
{
private:
	unsigned char key[260]; /* stores size first */
	unsigned char sbox[256];
	unsigned char sbox_free;

public:
	CRC4(unsigned char *key, unsigned int keySize);
	CRC4();
	~CRC4() {}

	void Initialize(unsigned char *key, unsigned int keySize);

	void Encrypt(unsigned char *data, unsigned long dataSize);
	void Decrypt(unsigned char *data, unsigned long dataSize) { Encrypt(data, dataSize); }

	std::string CopyEncrypt(unsigned char *data, unsigned long dataSize);
	std::string CopyDecrypt(unsigned char *data, unsigned long dataSize) { return CopyEncrypt(data, dataSize); }
};
#endif // RC4_H