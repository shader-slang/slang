//TEST:COMPILE: tests/modules/type-traits-module.slang -o tests/modules/type-traits-module.slang-module
//TEST:COMPILE(filecheck=CHECK): tests/modules/type-traits-module-test.slang -target hlsl

import type_traits_module;

RWStructuredBuffer<int> outputBuffer;

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain()
{
    int arr[TypeTrait<half,5, 4, 3>.Uint4AlignedK] = {};

    // CHECK: 8
    outputBuffer[0] = arr.getCount();
    for (int i = 0; i < arr.getCount(); i++)
    {
        outputBuffer[i + 1] = arr[i];
    }
}
