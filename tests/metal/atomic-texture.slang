//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL_FLOAT): -target metal -stage compute -entry computeMain -DFLOAT
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

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

// METAL_FLOAT: 'float' atomic texture operations are disallowed with Metal target's

//METALLIB: @computeMain

RWBuffer<uint> outputBufferUInt;
RWBuffer<int> outputBufferInt;
RWBuffer<float> outputBufferFloat;

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


    // int
	InterlockedAdd(outputBufferInt[0], valInt);
	InterlockedAnd(outputBufferInt[0], valInt);
	InterlockedMax(outputBufferInt[0], valInt);
	InterlockedMin(outputBufferInt[0], valInt);
	InterlockedOr(outputBufferInt[0], valInt);
	InterlockedXor(outputBufferInt[0], valInt);

	InterlockedAdd(outputBufferInt[0], valInt, originalValueInt);
	InterlockedAnd(outputBufferInt[0], valInt, originalValueInt);
	InterlockedMax(outputBufferInt[0], valInt, originalValueInt);
	InterlockedMin(outputBufferInt[0], valInt, originalValueInt);
	InterlockedOr(outputBufferInt[0], valInt, originalValueInt);
	InterlockedXor(outputBufferInt[0], valInt, originalValueInt);
	InterlockedExchange(outputBufferInt[0], valInt, originalValueInt);
	InterlockedCompareExchange(outputBufferInt[0], valInt, compareValueInt, originalValueInt);

    // uint
	InterlockedAdd(outputBufferUInt[0], valUInt);
	InterlockedAnd(outputBufferUInt[0], valUInt);
	InterlockedMax(outputBufferUInt[0], valUInt);
	InterlockedMin(outputBufferUInt[0], valUInt);
	InterlockedOr(outputBufferUInt[0], valUInt);
	InterlockedXor(outputBufferUInt[0], valUInt);

	InterlockedAdd(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedAnd(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedMax(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedMin(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedOr(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedXor(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedExchange(outputBufferUInt[0], valUInt, originalValueUInt);
	InterlockedCompareExchange(outputBufferUInt[0], valUInt, compareValueUInt, originalValueUInt);

#ifdef FLOAT
    //float
	InterlockedExchange(outputBufferFloat[0], valFloat);
	InterlockedExchange(outputBufferFloat[0], valFloat, originalValueFloat);
#endif //FLOAT
}

[numthreads(1, 1, 1)]
void computeMain()
{
    test();
}
