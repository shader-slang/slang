// entry-point-not-found.slang

// An unknown `-entry` name is reported once, as E38000 with the name and a
// location, not also as an empty "undefined identifier" from the lookup.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv -entry mian
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target cuda -entry mian

// CHECK-NOT: E30015
// CHECK-NOT: undefined identifier
// CHECK: error[E38000]: no function found matching entry point name 'mian'
// CHECK-NOT: E38000
// CHECK-NOT: E30015
// CHECK-NOT: undefined identifier

// When the name is more than an identifier, the lookup can fail in a part that
// E38000 does not mention (here the generic argument), so its errors are kept.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=GENERIC,exit-code=-1):-target spirv -entry genericMain<MissingType>

// GENERIC: error[E30015]: undefined identifier
// GENERIC: error[E38000]: no function found matching entry point name 'genericMain<MissingType>'

[numthreads(64, 1, 1)]
[shader("compute")]
void main(uint3 tid: SV_DispatchThreadID, uniform uint* o)
{
    o[tid.x] = tid.x;
}

[numthreads(64, 1, 1)]
[shader("compute")]
void genericMain<T : __BuiltinFloatingPointType>(uint3 tid: SV_DispatchThreadID, uniform T* o)
{
    o[tid.x] = T(tid.x);
}
