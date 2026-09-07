// Regression for https://github.com/shader-slang/slang/issues/12926.
//
// The extension-first checking pass evaluates `Hints : __EnumType` (via the extension in
// ext.slang) before `Hints`'s own declaration is traversed; the enum's synthesized `__EnumType`
// conformance must still be found by the later `is_set<T : __EnumType>` call.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

module test_enum_conformance_12926;

__include enums;
__include ext;

public bool is_set<T : __EnumType>(T value, T flag)
{
    return (__slang_noop_cast<T.__Tag>(value) & __slang_noop_cast<T.__Tag>(flag)) ==
           __slang_noop_cast<T.__Tag>(flag);
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Hints hints = Hints::b;
    outputBuffer[0] = is_set(hints, Hints::b) ? 1u : 0u;
    // CHECK: 1
}
