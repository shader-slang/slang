// Verify that `any`/`all` resolve for scalar, vector, and matrix bool masks when slang.numerics is
// imported, and that the interface-generic mask wrappers stay reachable for generic code.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o boolean-mask-any-all-14613.hlsl -experimental-feature
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o boolean-mask-any-all-14613.spv -experimental-feature
//TEST:SIMPLE: -target wgsl -entry computeMain -stage compute -o boolean-mask-any-all-14613.wgsl -experimental-feature
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -o boolean-mask-any-all-14613.metal -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

import slang.numerics;

RWStructuredBuffer<int> outputBuffer;

bool reduceMask<M : IBooleanMask>(M mask)
{
    return any(mask) && all(mask);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    bool scalar = true;
    bool3 mixed = bool3(true, false, true);
    bool2x2 mixedMatrix = bool2x2(true, false, false, true);

    outputBuffer[0] = int(any(scalar) == scalar);
    outputBuffer[1] = int(all(scalar) == scalar);
    outputBuffer[2] = int(any(mixed) == true);
    outputBuffer[3] = int(all(mixed) == false);
    outputBuffer[4] = int(reduceMask(scalar) == scalar);
    outputBuffer[5] = int(reduceMask(mixed) == (any(mixed) && all(mixed)));
    outputBuffer[6] = int(any(mixedMatrix) == true);
    outputBuffer[7] = int(all(mixedMatrix) == false);
}
