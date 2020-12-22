#ifndef STREAM_H
#define STREAM_H
typedef unsigned __int64 QWORD; //DWORD * 2

#ifdef _WIN64 or _WIN32
#include <Windows.h>
#else
typedef unsigned char BYTE; //8 BIT
typedef unsigned short WORD; //BYTE * 2
typedef unsigned __int32 DWORD; //WORD * 2

typedef unsigned __int32 SIZE_T;

#define FALSE 0
#define TRUE 1
#endif

#include <string>

/*
DEVELOPER NOTE FOR DEBUGGING:
	ByteStreamWriter::WriteValue identifies as the same of ByteStreamWriter::WriteData.

SPECIAL WRITING SEQUENCE FOR DATA:
	STRINGS: Strings are written in a special format/way.
		[
			UNTIL_NULL_TERM:
				[stringhex] 00

			GIVEN_SIZE:
				[stringsize] [stringhex]
		]
*/



enum StringInterpretMode
{
	/*
	This is for strings that are written as null-terminated ONLY.
	For some reason H1Z1 likes to switch that shit.

	By this, I mean this mode allows reading of strings that don't
	have a size. The method to get the size, is exact same as strlen().
	Follow until first '\0' or 0 byte, which is called the null-terminator.
	*/
	UNTIL_NULL_TERM,

	/*
	This is for strings that are written as non-null terminated (or null-terminated, no difference).
	For some reason H1Z1 likes to switch that shit.

	By this, I mean this mode allows reading of strings that have
	the size given right before the string (in a 4 byte DWORD or __int32).
	It will then use this size and say that is the real string's size. Be careful.
	*/
	GIVEN_SIZE,

	/* Combination of both above. This gives the size, and null-terminating character... Why? */
	GIVEN_SIZE_NULL_TERM,
};


/* This class is responsible for the buffer, it will allocate, resize, and so on. */
class ByteStreamBuffer
{
private:
	BYTE allocatedInst;

protected:
	BYTE *buffer;
	SIZE_T bufferSize;
	int idx;

	BYTE autoBuffer;
	BYTE bigEndian;

public:
	ByteStreamBuffer();
	ByteStreamBuffer(SIZE_T bufferSize, BYTE bigEndian, BYTE autoBuffer = TRUE);
	ByteStreamBuffer(BYTE *buffer, SIZE_T bufferSize, BYTE bigEndian, BYTE autoBuffer = TRUE);
	~ByteStreamBuffer();

	/* Initialize the ByteStreamBuffer like the constructors do. */
	void SetupBuffer(SIZE_T bufferSize, BYTE bigEndian, BYTE autoBuffer = TRUE) { ByteStreamBuffer(bufferSize, bigEndian, autoBuffer); }

	/* Initialize the ByteStreamBuffer like the constructors do. */
	void SetupBuffer(BYTE *buffer, SIZE_T bufferSize, BYTE bigEndian, BYTE autoBuffer = TRUE) { ByteStreamBuffer(buffer, bufferSize, bigEndian, autoBuffer); }


	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	void SetIdx(int idx) { this->idx = idx; }
	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	void SetIndex(int idx) { SetIdx(idx); }

	/* This returns the internal index. */
	int GetIdx() { return this->idx; }
	/* This returns the internal index. */
	int GetIndex() { return this->idx; }

	/* This will return the amount of bytes written. */
	int GetSize() { return this->idx; }
	/* This will return the size of the buffer, in bytes. */
	int GetBufferSize() { return this->bufferSize; }


	/* Returns the internal buffer (if any) */
	BYTE *GetBuffer() { return this->buffer; }

	/* Returns if the internal buffer is big endian (it's interpreted differently) */
	BYTE IsBigEndian() { return this->bigEndian; }

	/* Returns if the internal buffer does auto-buffering (automatically resizes the buffer, as it's being written) */
	BYTE IsAutoBuffering() { return this->autoBuffer; }


	/*
	Resizes the buffer to write to 'bufferSize'.
	This also clears the buffer, so be careful.

	If you want it not to clear the buffer, set
	the argument 'persist' to true.
	*/
	void Resize(SIZE_T bufferSize, bool persist = false);
};


class ByteStreamWriter : private ByteStreamBuffer
{
public:
	ByteStreamWriter();
	/*
	This will setup the internal buffer for usage.
	'bufferSize' can be used as a fixed or starting buffer size,
	meanwhile if you set autoBuffer to TRUE it will automatically
	resize and extend the buffer when writing data if it is too small.
	*/
	ByteStreamWriter(SIZE_T bufferSize, BYTE bigEndian = FALSE, BYTE autoBuffer = TRUE);
	~ByteStreamWriter();

	/* Initialize the ByteStreamWriter like the constructors do. */
	void Initialize(SIZE_T bufferSize, BYTE bigEndian = FALSE, BYTE autoBuffer = TRUE) { ByteStreamWriter(bufferSize, bigEndian, autoBuffer); }


	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIdx;
	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIndex;

	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIdx;
	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIndex;

	/*
	This will return the amount of bytes written.
	*/
	using ByteStreamBuffer::GetSize;
	/*
	This will return the size of the buffer, in bytes.
	*/
	using ByteStreamBuffer::GetBufferSize;


	/* Returns if the internal buffer is big endian (it's interpreted differently) */
	using ByteStreamBuffer::IsBigEndian;

	/* Returns if the internal buffer does auto-buffering (automatically resizes the buffer, as it's being written) */
	using ByteStreamBuffer::IsAutoBuffering;



	/*
	Resizes the buffer to write to 'bufferSize'.
	This also clears the buffer, so be careful.

	If you want it not to clear the buffer, set
	the argument 'persist' to true.
	*/
	using ByteStreamBuffer::Resize;


	/*
	Appends 'data' to buffer (with length of 'dataSize').

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteData(BYTE *data, SIZE_T dataSize, int startIndex = -1);



	/*
	Writes a string to the internal buffer, 'str'.

	If you set autoResize (to true), it will automatically resize the buffer for you
	JUST ENOUGH to fit the string.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.

	If you set wideString (to true) it will write 'str' as a wide-string.
	If you set wideStringLE (to true) it will write 'str' as a wide-string little endian.
	*/
	void WriteString(std::string str, StringInterpretMode stringMode = StringInterpretMode::GIVEN_SIZE, bool autoResize = true, int startIndex = -1, bool wideString = false, bool wideStringLE = false);

	/*
	Writes a string to the internal buffer, 'str' as a wide string.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.

	If you set autoResize (to true), it will automatically resize the buffer for you
	JUST ENOUGH to fit the string.

	If you set wideStringLE (to true) it will write 'str' as a wide-string little endian.
	*/
	void WriteWString(std::wstring str, StringInterpretMode stringMode = StringInterpretMode::GIVEN_SIZE, bool autoResize = true, int startIndex = -1);


	/*
	Writes a byte onto the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteByte(BYTE data, int startIndex = -1);


	/*
	Writes a WORD or a "short" onto the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteWord(WORD data, int startIndex = -1);


	/*
	Writes a DWORD or a "__int32" onto the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteDword(DWORD data, int startIndex = -1);

	/*
	Reads a QWORD or a "__int64" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	void WriteQword(QWORD data, int startIndex = -1);


	/*
	Writes any value to the internal buffer.

	I suggest using this for:
		- double
		- float
		- any basic struct with no foreign data types [IF YOU KNOW WHAT YOU'RE DOING], though you have to know that reading the struct will give only the struct.
			MUST NOT BE A POINTER. this does only write the bytes of that memory address.
		- other foreign values

	Be careful with this function. It can result to 'undefined byte writing' or,
	where it is not what you expected the memory to be. Pointers are not something you should
	even serialize either, which this will accept any value you give it.


	If you set size, it will use that size instead of sizeof().

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	template<typename Value_T>
	void WriteValue(Value_T data, int size = -1, int startIndex = -1)
	{
		WriteData(&data, size == -1 ? sizeof(data) : size, startIndex);
	}



	/* when you explicitly cast the class to a BYTE *, this will return the actual buffer */
	explicit operator BYTE *()
	{
		return this->buffer;
	}

	/* when you explicitly cast the class to a std::string, this will return the actual buffer (within a string) */
	explicit operator std::string()
	{
		return std::string((char *)this->buffer, bufferSize);
	}

	/* when you explicitly cast the class to a char *, this will return the actual buffer */
	explicit operator char *()
	{
		return (char *)this->buffer;
	}
};


class ByteStreamReader : private ByteStreamBuffer
{
public:
	ByteStreamReader();
	ByteStreamReader(BYTE *buffer, SIZE_T bufferSize, BYTE bigEndian);
	~ByteStreamReader();

	template<typename buffer_T>
	ByteStreamReader(buffer_T buffer, BYTE bigEndian)
	{
		return ByteStreamReader((BYTE *)buffer, sizeof(buffer), bigEndian);
	}

	template<typename buffer_T>
	ByteStreamReader(buffer_T buffer, SIZE_T bufferSize, BYTE bigEndian)
	{
		return ByteStreamReader((BYTE *)buffer, bufferSize, bigEndian);
	}

	/* Initialize the ByteStreamReader like the constructors do. */
	void Initialize(BYTE *buffer, SIZE_T bufferSize, BYTE bigEndian) { ByteStreamReader(buffer, bufferSize, bigEndian); }

	/* Initialize the ByteStreamReader like the constructors do. */
	template<typename buffer_T>
	void Initialize(buffer_T buffer, SIZE_T bufferSize, BYTE bigEndian) { ByteStreamReader((BYTE *)buffer, bufferSize, bigEndian); }

	/* Initialize the ByteStreamReader like the constructors do. */
	template<typename buffer_T>
	void Initialize(buffer_T buffer, BYTE bigEndian) { ByteStreamReader((BYTE *)buffer, sizeof(buffer), bigEndian); }


	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIdx;
	/*
	Changes the internal index. Using internal indexing
	will increase while it writes data to the buffer.
	*/
	using ByteStreamBuffer::SetIndex;

	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIdx;
	/*
	This returns the internal index.
	*/
	using ByteStreamBuffer::GetIndex;

	/*
	This will return the amount of bytes written.
	*/
	using ByteStreamBuffer::GetSize;
	/*
	This will return the size of the buffer, in bytes.
	*/
	using ByteStreamBuffer::GetBufferSize;


	/* Returns if the internal buffer is big endian (it's interpreted differently) */
	using ByteStreamBuffer::IsBigEndian;

	/* Returns if the internal buffer does auto-buffering (automatically resizes the buffer, as it's being written) */
	using ByteStreamBuffer::IsAutoBuffering;



	/*
	Gets a pointer to the buffer (and checks for the information being valid).

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	BYTE *ReadData(SIZE_T dataSize, int startIndex = -1);



	/*
	Reads a string from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.

	If you set wideString (to true) it will read as a wide-string.
	If you set wideStringLE (to true) it will read as a wide-string little endian.
	*/
	std::string ReadString(StringInterpretMode stringMode = StringInterpretMode::GIVEN_SIZE, int startIndex = -1, bool wideString = false, bool wideStringLE = false);

	/*
	Reads a string from the internal buffer, as a wide string.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	std::wstring ReadWString(StringInterpretMode stringMode = StringInterpretMode::GIVEN_SIZE, int startIndex = -1, bool wideStringLE = false);


	/*
	Reads a byte from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	BYTE ReadByte(int startIndex = -1);


	/*
	Reads a WORD or a "short" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	WORD ReadWord(int startIndex = -1);


	/*
	Reads a DWORD or a "__int32" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	DWORD ReadDword(int startIndex = -1);

	/*
	Reads a QWORD or a "__int64" from the internal buffer.

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	QWORD ReadQword(int startIndex = -1);


	/*
	Reads any value from the internal buffer.

	I suggest using this for:
		- double
		- float
		- any basic struct with no foreign data types [IF YOU KNOW WHAT YOU'RE DOING], though you have to know that reading the struct will give only the struct.
			MUST NOT BE A POINTER. this does only write the bytes of that memory address.
		- other foreign values

	Be careful with this function. It can result to 'undefined byte reading' or,
	where it is not what you expected the memory to be. Pointers are not something you should
	even deserialize either, which this will accept any value you give it.


	If you set size, it will use that size instead of sizeof().

	If you choose to set startIndex, it will start at that index.
	Doing so, will not increase the internal index.
	*/
	template<typename Value_T>
	Value_T ReadValue(int size = -1, int startIndex = -1)
	{
		return *(Value_T *)ReadData(size == -1 ? sizeof(Value_T) : size, startIndex);
	}



	/* when you explicitly cast the class to a BYTE *, this will return the actual buffer */
	explicit operator BYTE *()
	{
		return this->buffer;
	}

	/* when you explicitly cast the class to a std::string, this will return the actual buffer (within a string) */
	explicit operator std::string()
	{
		return std::string((char *)this->buffer, bufferSize);
	}

	/* when you explicitly cast the class to a char *, this will return the actual buffer */
	explicit operator char *()
	{
		return (char *)this->buffer;
	}
};
#endif // STREAM_H