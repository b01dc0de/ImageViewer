#ifndef UTILS_H
#define UTILS_H

void Outf(const char* Fmt, ...);

bool MatchStr(const char* A, const char* B);

struct FileContentsT
{
	const char* Name;
	size_t Size;
	byte* Contents;
};

void Release(FileContentsT& FileContents);
FileContentsT ReadFileContents(const char* InFileName);
//void WriteFileContents(FileContentsT& InFileContents, const char* OutFileName);

struct FileReaderT
{
	FileContentsT& File;
	size_t ReadIdx;

	FileReaderT(FileContentsT& _File) : File(_File) , ReadIdx(0) { }
	bool IsDone()
	{
		return ReadIdx >= File.Size;
	}
	void ReadData(byte* OutData, size_t NumBytes)
	{
		ASSERT(OutData && ReadIdx + NumBytes <= File.Size);
		if (OutData && ReadIdx + NumBytes <= File.Size)
		{
            memcpy(OutData, File.Contents + ReadIdx, NumBytes);
            ReadIdx += NumBytes;
		}
	}
	u8 ReadU8()
	{
		ASSERT(ReadIdx + sizeof(u8) <= File.Size);
		u8 Result = *(u8*)(File.Contents + ReadIdx);
		ReadIdx += sizeof(u8);
		return Result;
	}
	u16 ReadU16()
	{
		ASSERT(ReadIdx + sizeof(u16) <= File.Size);
		u16 Result = *(u16*)(File.Contents + ReadIdx);
		ReadIdx += sizeof(u16);
		return Result;
	}
	u32 ReadU32()
	{
		ASSERT(ReadIdx + sizeof(u32) <= File.Size);
		u32 Result = *(u32*)(File.Contents + ReadIdx);
		ReadIdx += sizeof(u32);
		return Result;
	}
	u64 ReadU64()
	{
		ASSERT(ReadIdx + sizeof(u64) <= File.Size);
		u64 Result = *(u64*)(File.Contents + ReadIdx);
		ReadIdx += sizeof(u64);
		return Result;
	}
	f32 ReadF32()
	{
		ASSERT(ReadIdx + sizeof(f32) <= File.Size);
		f32 Result = *(f32*)(File.Contents + ReadIdx);
		ReadIdx += sizeof(f32);
		return Result;
	}
	f64 ReadF64()
	{
		ASSERT(ReadIdx + sizeof(f64) <= File.Size);
		f64 Result = *(f64*)(File.Contents + ReadIdx);
		ReadIdx += sizeof(f64);
		return Result;
	}
};

template <typename T>
struct Array
{
	size_t Capacity = 0;
	size_t Num = 0;
	T* Data = nullptr;

	static constexpr float GrowthFactor = 2.0f;
	static constexpr size_t DefaultInitCapacity = 16;

	Array(size_t InitCapacity = DefaultInitCapacity)
	{
		Capacity = InitCapacity;
		Num = 0;
		Data = new T[InitCapacity];
	}
	Array(const Array&) = default;
	Array& operator=(const Array&) = default;
	~Array()
	{
		if (Data) { delete[] Data; }
		Capacity = 0;
		Num = 0;
		Data = nullptr;
	}

	void Grow()
	{
		size_t OldCapacity = Capacity;
		T* OldData = Data;

		Capacity = (size_t)(Capacity * GrowthFactor);
		Data = new T[Capacity];

		memcpy(Data, OldData, sizeof(T) * Num);
		delete[] OldData;
	}

	void Add(T& Item)
	{
		if (Num >= Capacity) { Grow(); }

		Data[Num++] = Item;
	}

	T& operator[](size_t Idx)
	{
		return Data[Idx];
	}

	T* operator*()
	{
		return Data;
	}
};

#endif // UTILS_H

