// Regression for https://github.com/shader-slang/slang/issues/12926.
//
// An `enum`'s conformance to the built-in `__EnumType` interface is synthesized
// late, when the enum is driven to `ReadyForLookup`. When a module is assembled
// from `__include` fragments, `checkModule` drives every `extension` to
// `ReadyForLookup` in a dedicated pass that runs before the enum's own
// declaration is checked. Resolving the `extension EnumBox<Hints>` target below
// therefore checks `Hints : __EnumType` before that conformance exists, which
// used to cache a non-conforming result that survived (the inheritance cache is
// only invalidated by extension-epoch bumps). The later `is_set(hints, ...)`
// call then read the stale negative and failed with E38029, even though `Hints`
// genuinely conforms.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

module test_enum_conformance_12926;

__include enums;
__include ext;

// A generic helper constrained on the built-in `__EnumType` interface -- the same
// shape as the `is_set` utility in the bug report.
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
