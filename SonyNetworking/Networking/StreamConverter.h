#ifndef STREAMCONVERTER_H
#define STREAMCONVERTER_H
#include "../stream/stream.h"

#ifdef SONYNETWORKING_EXPORTS
#define NETAPI __declspec(dllexport)
#else
#define NETAPI __declspec(dllimport)
#endif



namespace Networking
{
	/* This will encrypt/decrypt the streams, before sending and after recieving packets. */
	class NETAPI StreamConverter
	{
	private:
		/* this is the buffer, from the streams */
		ByteStreamBuffer buffer;

		BYTE *encKey;
		int encKeyLen;

	public:
		StreamConverter();
		StreamConverter(ByteStreamWriter &writer, BYTE *encKey, int encKeyLen);
		StreamConverter(ByteStreamReader &reader, BYTE *encKey, int encKeyLen);
		~StreamConverter();

		/* Initialize the ByteStreamReader like the constructors do. */
		void Initialize(ByteStreamWriter &writer, BYTE *encKey, int encKeyLen);

		/* Initialize the ByteStreamReader like the constructors do. */
		void Initialize(ByteStreamReader &reader, BYTE *encKey, int encKeyLen);

		/* Initialize the ByteStreamReader like the constructors do. */
		void Initialize(ByteStreamBuffer &buffer, BYTE *encKey, int encKeyLen);


		/* This will encrypt OR decrypt the stream. You may still use the same stream reader/writer/buffer. */
		void Convert(int startIndex = 0);
	};
}
#endif // STREAMCONVERTER_H