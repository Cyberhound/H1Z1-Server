#include "stream.h"



#define SAFETY_CHECK(f) \
BYTE incrIndex = FALSE; \
\
/* check if we have a valid buffer. buffer is always initialized to something, either nullptr or a real buffer. */ \
if (!buffer || !bufferSize) \
	throw std::exception(f " got null buffer or null size."); /* wtf you doing retard xD */ \
 \
/* set the starting index */ \
if (startIndex == -1) \
{ \
	startIndex = this->idx; \
	incrIndex = TRUE; \
	if (startIndex == -1) \
		throw std::exception(f " tried to access buffer with invalid index."); \
}


/* only different interpretation on big endian */
#define AUTO_ENDIAN(result, size) \
if (this->bigEndian) \
{ \
	BYTE data[size]; \
	 \
	for (int i = size - 1; i >= 0; i--) \
		data[i] = ((BYTE *)&result)[i]; \
	 \
	memcpy(&result, data, size); \
}

#define AUTO_ENDIAN2(result) \
if (this->bigEndian) result = (result & 0xFF) << 8 | (result & 0xFF00) >> 8;

#define AUTO_ENDIAN4(result) \
if (this->bigEndian) result = _byteswap_ulong(result);

#define AUTO_ENDIAN8(result) \
if (this->bigEndian) result = _byteswap_uint64(result);


#define AUTO_BUFFER(size) \
if (autoBuffer && (size) > this->bufferSize - this->idx) \
	this->Resize(this->bufferSize + (size), true);

#define AUTO_BUFFERF(size, forcecond) \
if ((autoBuffer || forcecond) && (size) > this->bufferSize - this->idx) \
	this->Resize(this->bufferSize + (size), true);



/* byte stream buffer, responsible for the internal buffer. */
ByteStreamBuffer::ByteStreamBuffer() :
	buffer(nullptr),
	bufferSize(0),
	idx(-1),
	bigEndian(FALSE),
	autoBuffer(TRUE),

	allocatedInst(FALSE)
{}

ByteStreamBuffer::ByteStreamBuffer(SIZE_T _bufferSize, BYTE _bigEndian, BYTE _autoBuffer) :
	buffer(_bufferSize > 0 ? (BYTE *)malloc(bufferSize) : nullptr),
	bufferSize(_bufferSize),
	idx(0),
	bigEndian(_bigEndian),
	autoBuffer(_autoBuffer),

	allocatedInst(TRUE)
{
	memset(buffer, 0, bufferSize);
}

ByteStreamBuffer::ByteStreamBuffer(BYTE *_buffer, SIZE_T _bufferSize, BYTE _bigEndian, BYTE _autoBuffer) :
	buffer(_buffer),
	bufferSize(_bufferSize),
	idx(0),
	bigEndian(_bigEndian),
	autoBuffer(_autoBuffer),

	allocatedInst(FALSE)
{}

ByteStreamBuffer::~ByteStreamBuffer()
{
	if (!allocatedInst || !buffer || !bufferSize)
		return;

	if (buffer)
		free(buffer);
}


void ByteStreamBuffer::Resize(SIZE_T bufferSize, bool persist)
{
	if (!persist)
	{
		/* reallocate the buffer, and clear it then set the size */
		free(buffer);
		buffer = (BYTE *)malloc(bufferSize);
		if (!buffer || buffer == 0)
		{
			/* reallocation failed. */
			bufferSize = 0;
			return;
		}

		memset(buffer, 0, bufferSize);
		this->bufferSize = bufferSize;
		return;
	}

	/* create/store buffers */
	BYTE *oldBuffer = buffer;
	buffer = (BYTE *)malloc(bufferSize);
	memset(buffer, 0, bufferSize);

	/* copy the buffer */
	memcpy(buffer, oldBuffer, this->bufferSize);

	/* free the old buffer & set the new buffer size */
	free(oldBuffer);
	this->bufferSize = bufferSize;
}



/* byte stream writing */
void ByteStreamWriter::WriteData(BYTE *data, SIZE_T dataSize, int startIndex)
{
	AUTO_BUFFER(dataSize);

	/* do some fail-safe checking, "__FUNCTION__" is the function's name. */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + dataSize > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	memcpy(buffer + startIndex, data, dataSize);
	if (incrIndex)
		this->idx = startIndex + dataSize;
}


void ByteStreamWriter::WriteString(std::string str, StringInterpretMode stringMode, bool autoResize, int startIndex, bool wideString, bool wideStringLE)
{
	AUTO_BUFFERF((wideString ? str.size() * 2 : str.size()) + sizeof(DWORD), autoResize);

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + str.size() > bufferSize && !autoResize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	switch (stringMode)
	{
		case GIVEN_SIZE:
		{
			if (wideString)
			{
				/* allocate a temporary buffer for writing the string */
				int str_size = str.size() * 2;
				BYTE *data = (BYTE *)malloc(str_size);

				/* convert the string */
				for (SIZE_T i = 0; i < str.size(); i++)
				{
					if (wideStringLE)
					{
						/* little endian wide-string */
						data[0] = 0;
						data[1] = (BYTE)str[i];
					}
					else
					{
						/* big endian wide-string */
						data[0] = (BYTE)str[i];
						data[1] = 0;
					}
					data += 2;
				}

				/* put the string size */
				*(DWORD *)&buffer[startIndex] = str.size();

				/* copy the actual string */
				memcpy(buffer + startIndex + sizeof(DWORD), data, str_size);
				if (incrIndex)
					this->idx = startIndex + str_size + sizeof(DWORD);

				/* free the temporary buffer */
				free(data);
				return;
			}


			/* put the string size */
			*(DWORD *)&buffer[startIndex] = str.size();

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(DWORD), str.c_str(), str.size());
			if (incrIndex)
				this->idx = startIndex + str.size() + sizeof(DWORD);
			break;
		}
		case UNTIL_NULL_TERM:
		{
			if (wideString)
			{
				/* allocate a temporary buffer for writing the string + null-terminator */
				int str_size = str.size() * 2 + sizeof(WORD);
				BYTE *data = (BYTE *)malloc(str_size);

				/* convert the string */
				for (SIZE_T i = 0; i < str.size(); i++)
				{
					if (wideStringLE)
					{
						/* little endian wide-string */
						data[0] = 0;
						data[1] = (BYTE)str[i];
					}
					else
					{
						/* big endian wide-string */
						data[0] = (BYTE)str[i];
						data[1] = 0;
					}
					data += 2;
				}

				/* put a null-terminator */
				*(WORD *)&data[0] = 0;

				/* copy the actual string */
				memcpy(buffer + startIndex, data, str_size);
				if (incrIndex)
					this->idx = startIndex + str_size;

				/* free the temporary buffer */
				free(data);
				return;
			}


			/* put the null-terminator in the string */
			str.push_back('\0');

			/* write the whole string + null-terminator */
			memcpy(buffer + startIndex, str.c_str(), str.size());
			if (incrIndex)
				this->idx = startIndex + str.size();
			break;
		}
		case GIVEN_SIZE_NULL_TERM:
		{
			if (wideString)
			{
				/* allocate a temporary buffer for writing the string */
				int str_size = str.size() * 2 + 2;
				BYTE *data = (BYTE *)malloc(str_size);

				/* convert the string */
				for (SIZE_T i = 0; i < str.size(); i++)
				{
					if (wideStringLE)
					{
						/* little endian wide-string */
						data[0] = 0;
						data[1] = (BYTE)str[i];
					}
					else
					{
						/* big endian wide-string */
						data[0] = (BYTE)str[i];
						data[1] = 0;
					}
					data += 2;
				}

				/* put the null-terminator in the string */
				*(WORD *)&data = 0;

				/* put the string size */
				*(DWORD *)&buffer[startIndex] = str.size();

				/* copy the actual string */
				memcpy(buffer + startIndex + sizeof(DWORD), data, str_size);
				if (incrIndex)
					this->idx = startIndex + str_size + sizeof(DWORD);

				/* free the temporary buffer */
				free(data);
				return;
			}


			/* put the null-terminator in the string */
			str.push_back('\0');

			/* put the string size - 1, "- 1" for null-terminator that is NOT part of the string */
			*(DWORD *)&buffer[startIndex] = str.size() - 1;

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(DWORD), str.c_str(), str.size());
			if (incrIndex)
				this->idx = startIndex + str.size() + sizeof(DWORD);
			break;
		}
	}
}

void ByteStreamWriter::WriteWString(std::wstring str, StringInterpretMode stringMode, bool autoResize, int startIndex)
{
	AUTO_BUFFERF(str.size() * 2 + sizeof(DWORD), autoResize);

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + str.size() + sizeof(DWORD) > bufferSize && !autoResize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	switch (stringMode)
	{
		case GIVEN_SIZE:
		{
			/* put the string size */
			*(DWORD *)&buffer[startIndex] = str.size();

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(DWORD), str.c_str(), str.size() * 2);
			if (incrIndex)
				this->idx = startIndex + str.size() * 2 + sizeof(DWORD);
			break;
		}
		case UNTIL_NULL_TERM:
		{
			/* put the null-terminator in the string */
			str.push_back(L'\0');

			/* write the whole string + null-terminator */
			memcpy(buffer + startIndex, str.c_str(), str.size() * 2);
			if (incrIndex)
				this->idx = startIndex + str.size() * 2;
			break;
		}
		case GIVEN_SIZE_NULL_TERM:
		{
			/* put the null-terminator in the string */
			str.push_back('\0');

			/* put the string size - 1, "- 1" for null-terminator that is NOT part of the string */
			*(DWORD *)&buffer[startIndex] = str.size() - 1;

			/* write the whole string */
			memcpy(buffer + startIndex + sizeof(DWORD), str.c_str(), str.size() * 2);
			if (incrIndex)
				this->idx = startIndex + str.size() * 2 + sizeof(DWORD);
			break;
		}
	}
}


void ByteStreamWriter::WriteByte(BYTE data, int startIndex)
{
	AUTO_BUFFER(sizeof(BYTE));

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(BYTE) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	/* write the singular byte xD what a cutie */
	buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(BYTE);
}


void ByteStreamWriter::WriteWord(WORD data, int startIndex)
{
	AUTO_BUFFER(sizeof(WORD));

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(WORD) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	AUTO_ENDIAN2(data);
	*(WORD *)&buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(WORD);
}


void ByteStreamWriter::WriteDword(DWORD data, int startIndex)
{
	AUTO_BUFFER(sizeof(DWORD));

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(DWORD) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	AUTO_ENDIAN4(data);
	*(DWORD *)&buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(DWORD);
}


void ByteStreamWriter::WriteQword(QWORD data, int startIndex)
{
	AUTO_BUFFER(sizeof(QWORD));

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(QWORD) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to write too many bytes to buffer.");

	AUTO_ENDIAN8(data);
	*(QWORD *)&buffer[startIndex] = data;
	if (incrIndex)
		this->idx = startIndex + sizeof(QWORD);
}



/* byte stream reading */
BYTE *ByteStreamReader::ReadData(SIZE_T dataSize, int startIndex)
{
	/* do some fail-safe checking, "__FUNCTION__" is the function's name. */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + dataSize > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	if (incrIndex)
		this->idx = startIndex + dataSize;
	return &buffer[startIndex];
}


std::string ByteStreamReader::ReadString(StringInterpretMode stringMode, int startIndex, bool wideString, bool wideStringLE)
{
	DWORD str_size;

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	switch (stringMode)
	{
		case GIVEN_SIZE:
		case GIVEN_SIZE_NULL_TERM:
		{
			/* make sure we can read a DWORD */
			if (startIndex + sizeof(DWORD) > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

			str_size = ReadDword(startIndex);
			startIndex += 4;

			/* now make sure we can read the string (assuming this DWORD is the string's size) */
			if (startIndex + str_size > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");
			break;
		}
		case UNTIL_NULL_TERM:
		{
			/* search for the '0' character, aka null-terminator (inclusive for str_size). */
			str_size = 0;
			BYTE *pBuffer = buffer + startIndex + (wideStringLE ? 1 : 0);
			if (wideString)
			{
				while (*pBuffer && pBuffer <= buffer + bufferSize)
				{
					str_size++;
					pBuffer += 2;
				}
			}
			else
			{
				while (*pBuffer && pBuffer <= buffer + bufferSize)
				{
					str_size++;
					pBuffer++;
				}
			}

			if (str_size == 1)
			{
				/* 'read' the null-terminator */
				this->idx++;
				return "";
			}
			break;
		}
	}


	/* wide string? */
	if (wideString)
	{
		/* allocate a temporary buffer for writing the string */
		BYTE *data = (BYTE *)malloc(str_size / 2);

		/* convert the string (from wide to normal) */
		for (SIZE_T i = 0; i < str_size / 2; i++)
		{
			if (wideStringLE)
			{
				/* little endian wide-string */
				*data = (BYTE)buffer[startIndex + i + 1];
			}
			else
			{
				/* big endian wide-string */
				*data = (BYTE)buffer[startIndex + i];
			}
			data++;
		}

		std::string result = std::string((char *)data, str_size / 2);
		if (incrIndex)
			this->idx = startIndex + str_size + (stringMode == GIVEN_SIZE_NULL_TERM ? 2 : 0);

		/* free the temporary buffer */
		free(data);
		return result;
	}

	/* read the string, as a normal string. */
	std::string result = std::string((char *)buffer + startIndex, str_size);
	if (incrIndex)
		this->idx = startIndex + str_size + (stringMode == GIVEN_SIZE_NULL_TERM ? 1 : 0);
	return result;
}

std::wstring ByteStreamReader::ReadWString(StringInterpretMode stringMode, int startIndex, bool wideStringLE)
{
	DWORD str_size;

	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	switch (stringMode)
	{
		case GIVEN_SIZE:
		case GIVEN_SIZE_NULL_TERM:
		{
			/* make sure we can read a DWORD */
			if (startIndex + sizeof(DWORD) > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

			str_size = ReadDword(startIndex);
			startIndex += 4;

			/* now make sure we can read the string (assuming this DWORD is the string's size) */
			if (startIndex + str_size > bufferSize)
				throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");
			break;
		}
		case UNTIL_NULL_TERM:
		{
			/* search for the '0' character, aka null-terminator (inclusive for str_size). */
			str_size = 0;
			BYTE *pBuffer = buffer + startIndex + (wideStringLE ? 1 : 0);
			while (*pBuffer && pBuffer <= buffer + bufferSize)
			{
				str_size++;
				pBuffer += 2;
			}

			if (str_size == 1)
			{
				/* 'read' the null-terminator */
				this->idx++;
				return L"";
			}
			break;
		}
	}


	std::wstring result = std::wstring((wchar_t *)(buffer + startIndex), str_size / 2);
	if (incrIndex)
		this->idx = startIndex + str_size + (stringMode == GIVEN_SIZE_NULL_TERM ? 1 : 0);
	return result;
}


BYTE ByteStreamReader::ReadByte(int startIndex)
{
	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(BYTE) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	/* read the singular byte xD what a cutie */
	BYTE result = buffer[startIndex];
	if (incrIndex)
		this->idx = startIndex + sizeof(BYTE);
	return result;
}


WORD ByteStreamReader::ReadWord(int startIndex)
{
	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(WORD) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	WORD result = *(WORD *)&buffer[startIndex];
	AUTO_ENDIAN2(result);
	if (incrIndex)
		this->idx = startIndex + sizeof(WORD);
	return result;
}


DWORD ByteStreamReader::ReadDword(int startIndex)
{
	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(DWORD) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	DWORD result = *(DWORD *)&buffer[startIndex];
	AUTO_ENDIAN4(result);
	if (incrIndex)
		this->idx = startIndex + sizeof(DWORD);
	return result;
}


QWORD ByteStreamReader::ReadQword(int startIndex)
{
	/* do some fail-safe checking, "__FUNCTION__" is the function's name (for exceptions and debugging ig). */
	SAFETY_CHECK(__FUNCTION__);

	if (startIndex + sizeof(QWORD) > bufferSize)
		throw std::exception(__FUNCTION__ " tried to read too many bytes from the buffer.");

	QWORD result = *(QWORD *)&buffer[startIndex];
	AUTO_ENDIAN8(result);
	if (incrIndex)
		this->idx = startIndex + sizeof(QWORD);
	return result;
}



/* stream writer constructors */
ByteStreamWriter::ByteStreamWriter() :
	ByteStreamBuffer(0, FALSE)
{}

ByteStreamWriter::ByteStreamWriter(SIZE_T _bufferSize, BYTE _bigEndian, BYTE _autoBuffer) :
	ByteStreamBuffer(_bufferSize, _bigEndian == TRUE, _autoBuffer == TRUE)
{}


ByteStreamWriter::~ByteStreamWriter()
{}


/* stream reader constructors */
ByteStreamReader::ByteStreamReader() :
	ByteStreamBuffer()
{}

ByteStreamReader::ByteStreamReader(BYTE *_buffer, SIZE_T _bufferSize, BYTE _bigEndian) :
	ByteStreamBuffer(_buffer, _bufferSize, _bigEndian)
{}


ByteStreamReader::~ByteStreamReader()
{
	buffer = nullptr;
	bufferSize = 0;
}