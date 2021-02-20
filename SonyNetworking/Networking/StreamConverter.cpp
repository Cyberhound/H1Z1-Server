#include "StreamConverter.h"
#include <RC4/RC4.h>



/* Networking::StreamConverter::* */
void Networking::StreamConverter::Initialize(ByteStreamWriter &writer, BYTE *encKey, int encKeyLen)
{
	buffer.SetupBuffer(writer.GetBuffer(), writer.GetBufferSize(), writer.IsBigEndian(), writer.IsAutoBuffering());
	this->encKey = encKey;
	this->encKeyLen = encKeyLen;
}

void Networking::StreamConverter::Initialize(ByteStreamReader &reader, BYTE *encKey, int encKeyLen)
{
	buffer.SetupBuffer(reader.GetBuffer(), reader.GetBufferSize(), reader.IsBigEndian(), reader.IsAutoBuffering());
	this->encKey = encKey;
	this->encKeyLen = encKeyLen;
}

void Networking::StreamConverter::Initialize(ByteStreamBuffer &buffer, BYTE *encKey, int encKeyLen)
{
	this->buffer = buffer;
	this->encKey = encKey;
	this->encKeyLen = encKeyLen;
}


void Networking::StreamConverter::Convert(int startIndex)
{
	/* Note: Encryption == Decryption, it's XOR. Reversable. */
	CRC4 crypto(encKey, encKeyLen);
	crypto.Encrypt(buffer.GetBuffer() + startIndex, buffer.GetBufferSize() - startIndex);
}



/* Networking::StreamConverter constructors deconstructors */
Networking::StreamConverter::StreamConverter() :
	buffer(nullptr, 0, FALSE)
{}

Networking::StreamConverter::StreamConverter(ByteStreamWriter &_writer, BYTE *_encKey, int _encKeyLen) :
	buffer(_writer.GetBuffer(), _writer.GetBufferSize(), _writer.IsBigEndian(), _writer.IsAutoBuffering()),
	encKey(_encKey),
	encKeyLen(_encKeyLen)
{}

Networking::StreamConverter::StreamConverter(ByteStreamReader &_reader, BYTE *_encKey, int _encKeyLen) :
	buffer(_reader.GetBuffer(), _reader.GetBufferSize(), _reader.IsBigEndian(), _reader.IsAutoBuffering()),
	encKey(_encKey),
	encKeyLen(_encKeyLen)
{}

Networking::StreamConverter::~StreamConverter()
{}