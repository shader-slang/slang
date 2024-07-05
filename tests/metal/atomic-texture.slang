//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL_FLOAT): -target metal -stage compute -entry computeMain -DFLOAT
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// METAL_FLOAT: 'float' atomic texture operations are disallowed with Metal target's

//METALLIB: @computeMain

RWBuffer<uint> uintBuffer;
RWBuffer<int> intBuffer;
RWBuffer<float> floatBuffer;

RWTexture1D<uint> uintTexture1D;
RWTexture1D<int> intTexture1D;

RWTexture1DArray<uint> uintTexture1DArray;
RWTexture1DArray<int> intTexture1DArray;

RWTexture2D<uint> uintTexture2D;
RWTexture2D<int> intTexture2D;

RWTexture2DArray<uint> uintTexture2DArray;
RWTexture2DArray<int> intTexture2DArray;

RWTexture3D<uint> uintTexture3D;
RWTexture3D<int> intTexture3D;
void test()
{
    int valInt = 1;
    int originalValueInt;
    int compareValueInt = 1;

    uint valUInt = 1;
    uint originalValueUInt;
    int compareValueUInt = 1;

    float valFloat = 1.0f;
    float originalValueFloat;

#ifdef FLOAT
    //float
	InterlockedExchange(floatBuffer[0], valFloat);
	InterlockedExchange(floatBuffer[0], valFloat, originalValueFloat);
#endif //FLOAT
 
    // buffer
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
	InterlockedAdd(intBuffer[0], valInt);
	InterlockedAnd(intBuffer[0], valInt);
	InterlockedMax(intBuffer[0], valInt);
	InterlockedMin(intBuffer[0], valInt);
	InterlockedOr(intBuffer[0], valInt);
	InterlockedXor(intBuffer[0], valInt);
	InterlockedAdd(intBuffer[0], valInt, originalValueInt);
	InterlockedAnd(intBuffer[0], valInt, originalValueInt);
	InterlockedMax(intBuffer[0], valInt, originalValueInt);
	InterlockedMin(intBuffer[0], valInt, originalValueInt);
	InterlockedOr(intBuffer[0], valInt, originalValueInt);
	InterlockedXor(intBuffer[0], valInt, originalValueInt);
	InterlockedExchange(intBuffer[0], valInt, originalValueInt);
	InterlockedCompareExchange(intBuffer[0], valInt, compareValueInt, originalValueInt);

	InterlockedAdd(uintBuffer[0], valUInt);
	InterlockedAnd(uintBuffer[0], valUInt);
	InterlockedMax(uintBuffer[0], valUInt);
	InterlockedMin(uintBuffer[0], valUInt);
	InterlockedOr(uintBuffer[0], valUInt);
	InterlockedXor(uintBuffer[0], valUInt);
	InterlockedAdd(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedAnd(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedMax(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedMin(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedOr(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedXor(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedExchange(uintBuffer[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(uintBuffer[0], valUInt, compareValueUInt, originalValueUInt);

    // Texture1D
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
	InterlockedAdd(intTexture1D[0], valInt);
	InterlockedAnd(intTexture1D[0], valInt);
	InterlockedMax(intTexture1D[0], valInt);
	InterlockedMin(intTexture1D[0], valInt);
	InterlockedOr(intTexture1D[0], valInt);
	InterlockedXor(intTexture1D[0], valInt);
	InterlockedAdd(intTexture1D[0], valInt, originalValueInt);
	InterlockedAnd(intTexture1D[0], valInt, originalValueInt);
	InterlockedMax(intTexture1D[0], valInt, originalValueInt);
	InterlockedMin(intTexture1D[0], valInt, originalValueInt);
	InterlockedOr(intTexture1D[0], valInt, originalValueInt);
	InterlockedXor(intTexture1D[0], valInt, originalValueInt);
	InterlockedExchange(intTexture1D[0], valInt, originalValueInt);
	InterlockedCompareExchange(intTexture1D[0], valInt, compareValueInt, originalValueInt);

	InterlockedAdd(uintTexture1D[0], valUInt);
	InterlockedAnd(uintTexture1D[0], valUInt);
	InterlockedMax(uintTexture1D[0], valUInt);
	InterlockedMin(uintTexture1D[0], valUInt);
	InterlockedOr(uintTexture1D[0], valUInt);
	InterlockedXor(uintTexture1D[0], valUInt);
	InterlockedAdd(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedAnd(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedMax(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedMin(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedOr(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedXor(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedExchange(uintTexture1D[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(uintTexture1D[0], valUInt, compareValueUInt, originalValueUInt);

    // Texture1DArray
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
	InterlockedAdd(intTexture1DArray[0], valInt);
	InterlockedAnd(intTexture1DArray[0], valInt);
	InterlockedMax(intTexture1DArray[0], valInt);
	InterlockedMin(intTexture1DArray[0], valInt);
	InterlockedOr(intTexture1DArray[0], valInt);
	InterlockedXor(intTexture1DArray[0], valInt);
	InterlockedAdd(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedAnd(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedMax(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedMin(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedOr(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedXor(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedExchange(intTexture1DArray[0], valInt, originalValueInt);
	InterlockedCompareExchange(intTexture1DArray[0], valInt, compareValueInt, originalValueInt);

	InterlockedAdd(uintTexture1DArray[0], valUInt);
	InterlockedAnd(uintTexture1DArray[0], valUInt);
	InterlockedMax(uintTexture1DArray[0], valUInt);
	InterlockedMin(uintTexture1DArray[0], valUInt);
	InterlockedOr(uintTexture1DArray[0], valUInt);
	InterlockedXor(uintTexture1DArray[0], valUInt);
	InterlockedAdd(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedAnd(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedMax(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedMin(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedOr(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedXor(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedExchange(uintTexture1DArray[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(uintTexture1DArray[0], valUInt, compareValueUInt, originalValueUInt);

    // Texture2D
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
	InterlockedAdd(intTexture2D[0], valInt);
	InterlockedAnd(intTexture2D[0], valInt);
	InterlockedMax(intTexture2D[0], valInt);
	InterlockedMin(intTexture2D[0], valInt);
	InterlockedOr(intTexture2D[0], valInt);
	InterlockedXor(intTexture2D[0], valInt);
	InterlockedAdd(intTexture2D[0], valInt, originalValueInt);
	InterlockedAnd(intTexture2D[0], valInt, originalValueInt);
	InterlockedMax(intTexture2D[0], valInt, originalValueInt);
	InterlockedMin(intTexture2D[0], valInt, originalValueInt);
	InterlockedOr(intTexture2D[0], valInt, originalValueInt);
	InterlockedXor(intTexture2D[0], valInt, originalValueInt);
	InterlockedExchange(intTexture2D[0], valInt, originalValueInt);
	InterlockedCompareExchange(intTexture2D[0], valInt, compareValueInt, originalValueInt);

	InterlockedAdd(uintTexture2D[0], valUInt);
	InterlockedAnd(uintTexture2D[0], valUInt);
	InterlockedMax(uintTexture2D[0], valUInt);
	InterlockedMin(uintTexture2D[0], valUInt);
	InterlockedOr(uintTexture2D[0], valUInt);
	InterlockedXor(uintTexture2D[0], valUInt);
	InterlockedAdd(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedAnd(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedMax(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedMin(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedOr(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedXor(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedExchange(uintTexture2D[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(uintTexture2D[0], valUInt, compareValueUInt, originalValueUInt);

    // Texture2DArray
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak

// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_fetch_add
// METAL: .atomic_fetch_and
// METAL: .atomic_fetch_max
// METAL: .atomic_fetch_min
// METAL: .atomic_fetch_or
// METAL: .atomic_fetch_xor
// METAL: .atomic_exchange
// METAL: .atomic_compare_exchange_weak
	InterlockedAdd(intTexture2DArray[0], valInt);
	InterlockedAnd(intTexture2DArray[0], valInt);
	InterlockedMax(intTexture2DArray[0], valInt);
	InterlockedMin(intTexture2DArray[0], valInt);
	InterlockedOr(intTexture2DArray[0], valInt);
	InterlockedXor(intTexture2DArray[0], valInt);
	InterlockedAdd(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedAnd(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedMax(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedMin(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedOr(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedXor(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedExchange(intTexture2DArray[0], valInt, originalValueInt);
	InterlockedCompareExchange(intTexture2DArray[0], valInt, compareValueInt, originalValueInt);

	InterlockedAdd(uintTexture2DArray[0], valUInt);
	InterlockedAnd(uintTexture2DArray[0], valUInt);
	InterlockedMax(uintTexture2DArray[0], valUInt);
	InterlockedMin(uintTexture2DArray[0], valUInt);
	InterlockedOr(uintTexture2DArray[0], valUInt);
	InterlockedXor(uintTexture2DArray[0], valUInt);
	InterlockedAdd(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedAnd(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedMax(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedMin(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedOr(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedXor(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedExchange(uintTexture2DArray[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(uintTexture2DArray[0], valUInt, compareValueUInt, originalValueUInt);

    // Texture3D

    InterlockedAdd(intTexture3D[0], valInt);
	InterlockedAnd(intTexture3D[0], valInt);
	InterlockedMax(intTexture3D[0], valInt);
	InterlockedMin(intTexture3D[0], valInt);
	InterlockedOr(intTexture3D[0], valInt);
	InterlockedXor(intTexture3D[0], valInt);
	InterlockedAdd(intTexture3D[0], valInt, originalValueInt);
	InterlockedAnd(intTexture3D[0], valInt, originalValueInt);
	InterlockedMax(intTexture3D[0], valInt, originalValueInt);
	InterlockedMin(intTexture3D[0], valInt, originalValueInt);
	InterlockedOr(intTexture3D[0], valInt, originalValueInt);
	InterlockedXor(intTexture3D[0], valInt, originalValueInt);
	InterlockedExchange(intTexture3D[0], valInt, originalValueInt);
	InterlockedCompareExchange(intTexture3D[0], valInt, compareValueInt, originalValueInt);

	InterlockedAdd(uintTexture3D[0], valUInt);
	InterlockedAnd(uintTexture3D[0], valUInt);
	InterlockedMax(uintTexture3D[0], valUInt);
	InterlockedMin(uintTexture3D[0], valUInt);
	InterlockedOr(uintTexture3D[0], valUInt);
	InterlockedXor(uintTexture3D[0], valUInt);
	InterlockedAdd(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedAnd(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedMax(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedMin(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedOr(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedXor(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedExchange(uintTexture3D[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(uintTexture3D[0], valUInt, compareValueUInt, originalValueUInt);

}

[numthreads(1, 1, 1)]
void computeMain()
{
    test();
}
