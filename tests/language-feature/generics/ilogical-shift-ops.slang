//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// The shift operators `<<`, `>>`, `<<=` and `>>=` apply to a value of a generic type constrained
// to `ILogical` or `IInteger` with a scalar builtin-integer shift amount, and forward to the
// `shl`/`shr` requirements. Operands are loaded from a buffer so that the shifts are not folded at
// compile time.

enum Flags : uint
{
    None = 0,
    A = 1,
    B = 2,
    C = 4,
    D = 8,
}

// Shifts of builtin integer, `bool` and enum constants fold at compile time, so they can size an
// array, initialize an enum case and label a `case`.
enum Derived
{
    First = 1,
    Second = First << 1,
    Third = Second << 1,
}
static const bool kFlagSet = true;
static const int kBoolShift = (kFlagSet << 0) | (true << 2);
static const int kIntShift = 1u << 3;
static const Flags kEnumShift = Flags.A << 3;
groupshared int gScratch[true << 3];

T shlLogical<T : ILogical>(T value, uint amount)
{
    return value << amount;
}
T shrLogical<T : ILogical>(T value, uint amount)
{
    return value >> amount;
}
T shlAssignLogical<T : ILogical>(T value, int amount)
{
    value <<= amount;
    return value;
}
T shrAssignLogical<T : ILogical>(T value, int16_t amount)
{
    value >>= amount;
    return value;
}
T shlInteger<T : IInteger>(T value, int amount)
{
    return value << amount;
}
T shrInteger<T : IInteger>(T value, uint64_t amount)
{
    return value >> amount;
}
T shlAssignInteger<T : IInteger>(T value, uint amount)
{
    value <<= amount;
    return value;
}
T shrAssignInteger<T : IInteger>(T value, int8_t amount)
{
    value >>= amount;
    return value;
}
// Applies all four shift operators; the result is `value * 2` when no bits are shifted out.
T allShifts<T : ILogical>(T value, uint amount)
{
    T result = value << amount;
    result >>= 1;
    result <<= 2;
    return result >> amount;
}
int shlBool<T : __BuiltinIntegerType>(bool value, T amount)
{
    return value << amount;
}

//TEST_INPUT:ubuffer(data=[1 2 3 -16 0x80000000], stride=4):name=inputBuffer
StructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int one = inputBuffer[0];
    int two = inputBuffer[1];
    int three = inputBuffer[2];
    int minusSixteen = inputBuffer[3];
    uint highBit = uint(inputBuffer[4]);

    int i = 0;

    // Signed and unsigned 32-bit scalars.
    outputBuffer[i++] = shlLogical(three, uint(two));                  // CHECK: 12
    outputBuffer[i++] = shrLogical(minusSixteen, uint(two));           // CHECK-NEXT: -4
    outputBuffer[i++] = int(shrLogical(highBit, 31u));                 // CHECK-NEXT: 1
    outputBuffer[i++] = shlAssignLogical(one, three);                  // CHECK-NEXT: 8
    outputBuffer[i++] = shrAssignLogical(minusSixteen, int16_t(three)); // CHECK-NEXT: -2
    outputBuffer[i++] = shlInteger(three, two);                        // CHECK-NEXT: 12
    outputBuffer[i++] = int(shrInteger(highBit, uint64_t(30)));        // CHECK-NEXT: 2
    outputBuffer[i++] = shlAssignInteger(three, uint(three));          // CHECK-NEXT: 24
    outputBuffer[i++] = shrAssignInteger(minusSixteen, int8_t(one));   // CHECK-NEXT: -8

    // A 64-bit value keeps its high bits across the shift.
    int64_t wide = shlLogical(int64_t(one), 40u);
    outputBuffer[i++] = int(wide >> 32);                               // CHECK-NEXT: 256
    outputBuffer[i++] = int(shrInteger(wide, uint64_t(38)));           // CHECK-NEXT: 4

    // An integer vector shifts every lane by the same scalar amount.
    int3 lanes = int3(one, two, three);
    int3 shiftedLeft = shlLogical(lanes, uint(two));
    outputBuffer[i++] = shiftedLeft.x;                                 // CHECK-NEXT: 4
    outputBuffer[i++] = shiftedLeft.y;                                 // CHECK-NEXT: 8
    outputBuffer[i++] = shiftedLeft.z;                                 // CHECK-NEXT: 12
    int3 shiftedRight = shrAssignLogical(shiftedLeft * 4, int16_t(three));
    outputBuffer[i++] = shiftedRight.x;                                // CHECK-NEXT: 2
    outputBuffer[i++] = shiftedRight.y;                                // CHECK-NEXT: 4
    outputBuffer[i++] = shiftedRight.z;                                // CHECK-NEXT: 6

    // Every operator for each conforming type.
    outputBuffer[i++] = allShifts(three, uint(two));                   // CHECK-NEXT: 6
    outputBuffer[i++] = int(allShifts(highBit >> 4, uint(two)));       // CHECK-NEXT: 268435456
    outputBuffer[i++] = int(allShifts(wide, uint(three)) >> 32);       // CHECK-NEXT: 512
    outputBuffer[i++] = allShifts(lanes, uint(three)).z;               // CHECK-NEXT: 6
    outputBuffer[i++] = int(allShifts(Flags.B, uint(two)));            // CHECK-NEXT: 4

    // An enum conforms to `ILogical` through `__EnumType`.
    Flags flag = Flags(one);
    outputBuffer[i++] = int(shlLogical(flag, uint(three)));            // CHECK-NEXT: 8
    outputBuffer[i++] = int(shrAssignLogical(Flags.D, int16_t(two)));  // CHECK-NEXT: 2

    // A `bool` left operand of a builtin shift is promoted to `int`, so the result is an `int`.
    bool flagSet = one != 0;
    let promoted = flagSet << two;
    outputBuffer[i++] = promoted;                                      // CHECK-NEXT: 4
    outputBuffer[i++] = (flagSet << 1) >> 1;                           // CHECK-NEXT: 1
    outputBuffer[i++] = shlBool(flagSet, uint(two));                   // CHECK-NEXT: 4
    outputBuffer[i++] = flagSet << flagSet;                            // CHECK-NEXT: 2

    int boolSized[kBoolShift];
    int intSized[kIntShift];
    boolSized[kBoolShift - 1] = kBoolShift;
    intSized[kIntShift - 1] = kIntShift;
    gScratch[0] = boolSized[kBoolShift - 1] + intSized[kIntShift - 1];
    outputBuffer[i++] = gScratch[0];                                   // CHECK-NEXT: 13
    int label = 0;
    switch (two * 4)
    {
    case (true << 1):
        label = 1;
        break;
    case int(kEnumShift):
        label = 2;
        break;
    case int(Derived.Third):
        label = 3;
        break;
    }
    outputBuffer[i++] = label;                                         // CHECK-NEXT: 2
}
