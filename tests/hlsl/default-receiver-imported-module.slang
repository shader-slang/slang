//TEST:COMPILE: tests/hlsl/default-receiver-module-lib.hlsl -o tests/hlsl/default-receiver-module-lib.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// The language of the call site decides whether a call can pass a copy of an immutable receiver
// argument to an `inout` `this` parameter (issue #13964). This file is Slang code, so its calls to
// a default method and an interface requirement written in HLSL still require a mutable receiver.
// The HLSL module is precompiled because `import` resolves only `.slang` source files.

import default_receiver_module_lib;

StructuredBuffer<Helper> gHelpers;

int getThroughRequirement<T : IGetter>(T getter)
{
    let immutableGetter = getter;
    return immutableGetter.get();
//CHECK:                      ^ mutating method cannot be called on immutable value
//CHECK:                      ^ mutating method 'get' cannot be called on an immutable value
}

int getThroughElement()
{
    return gHelpers[0].get();
//CHECK:                  ^ no overload for 'get' applicable to arguments of type ()
}

// The candidate note names the method of the precompiled module, which has no source location.
//CHECK: candidate: func Helper.get() -> int
