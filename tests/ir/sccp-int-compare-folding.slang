//TEST(compute):COMPARE_COMPUTE(targets=cpu+vk,filecheck-buffer=CHECK):-output-using-type -g0
//TEST:SIMPLE(filecheck=FOLD):-target spirv-asm -entry computeMain -stage compute

// IR SCCP folds integer and bool comparisons in the type of their operands. For each integer type
// below, `lo` is less than `hi` when compared in that type, and greater than `hi` when compared
// with the other signedness. For example, `int(-1) < int(0)` is true, but compared as unsigned,
// -1 is 0xFFFFFFFF and the comparison is false. Likewise, `0U < 0xFFFFFFFFU` is true, but compared
// as signed, 0xFFFFFFFFU is -1. A fold with the wrong signedness therefore flips the result of
// every `<`, `<=`, `>` and `>=` between `lo` and `hi`. For bool, `lo` is false and `hi` is true.
// `lo2` equals `lo` and is a separate variable, so that the comparisons between equal values
// distinguish `<` from `<=` and `>` from `>=`.
//
// The operands are local variables initialized with constants, which the front end does not fold,
// so SCCP folds the comparisons during IR lowering, the same way for every target. The FOLD test
// checks that no comparison is left in the SPIR-V. Each comparison stores 1 when true and 0 when
// false, and every type expects the same 12 results.
//
// The run uses `-g0` because SPIR-V emit cannot describe `intptr_t` and `uintptr_t` locals in
// debug info (#13200). SCCP folds the comparisons the same way with or without debug info.

//TEST_INPUT:ubuffer(data=[0], count=132, stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

#define COMPARE_GROUP(T, LO, HI, BASE)                 \
    {                                                  \
        T lo = LO;                                     \
        T hi = HI;                                     \
        T lo2 = LO;                                    \
        outputBuffer[BASE + 0] = (lo == lo2) ? 1 : 0;  \
        outputBuffer[BASE + 1] = (lo == hi) ? 1 : 0;   \
        outputBuffer[BASE + 2] = (lo != hi) ? 1 : 0;   \
        outputBuffer[BASE + 3] = (lo != lo2) ? 1 : 0;  \
        outputBuffer[BASE + 4] = (lo < hi) ? 1 : 0;    \
        outputBuffer[BASE + 5] = (lo <= hi) ? 1 : 0;   \
        outputBuffer[BASE + 6] = (lo > hi) ? 1 : 0;    \
        outputBuffer[BASE + 7] = (lo >= hi) ? 1 : 0;   \
        outputBuffer[BASE + 8] = (lo < lo2) ? 1 : 0;   \
        outputBuffer[BASE + 9] = (lo <= lo2) ? 1 : 0;  \
        outputBuffer[BASE + 10] = (lo > lo2) ? 1 : 0;  \
        outputBuffer[BASE + 11] = (lo >= lo2) ? 1 : 0; \
    }

[numthreads(1, 1, 1)]
void computeMain()
{
    COMPARE_GROUP(bool, false, true, 0)
    COMPARE_GROUP(int8_t, int8_t(-1), int8_t(0), 12)
    COMPARE_GROUP(uint8_t, uint8_t(0), uint8_t(0xFF), 24)
    COMPARE_GROUP(int16_t, int16_t(-1), int16_t(0), 36)
    COMPARE_GROUP(uint16_t, uint16_t(0), uint16_t(0xFFFF), 48)
    COMPARE_GROUP(int, -1, 0, 60)
    COMPARE_GROUP(uint, 0U, 0xFFFFFFFFU, 72)
    COMPARE_GROUP(int64_t, -1ll, 0ll, 84)
    COMPARE_GROUP(uint64_t, 0ull, 0xFFFFFFFFFFFFFFFFull, 96)
    COMPARE_GROUP(intptr_t, intptr_t(-1), intptr_t(0), 108)
    COMPARE_GROUP(uintptr_t, uintptr_t(0), uintptr_t(-1), 120)
}

// FOLD-NOT: Op{{[SU](Less|Greater)Than|I(Not)?Equal|Logical(Not)?Equal}}

// CHECK: type: int32_t
// bool
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// int8_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// uint8_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// int16_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// uint16_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// int
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// uint
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// int64_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// uint64_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// intptr_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// uintptr_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1{{$}}
