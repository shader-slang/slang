//TEST:COMPILE: tests/serialization/effective-this-param-info-module.slang -o tests/serialization/effective-this-param-info-module.slang-module
//TEST:SIMPLE: -r tests/serialization/effective-this-param-info-module.slang-module -target spirv -entry computeMain -stage compute -o effective-this-param-info.spv

// Calling both methods after loading the binary module exercises deserialization of their
// effective `this` parameter information and its use by lookup and lowering.

import "effective-this-param-info-module";

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Receiver receiver = {1};
    int original = receiver.read();
    receiver.increment();
    outputBuffer[0] = original + receiver.value;
}
