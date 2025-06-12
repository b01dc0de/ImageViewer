#ifndef UTILS_H
#define UTILS_H

void Outf(const char* Fmt, ...);

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

