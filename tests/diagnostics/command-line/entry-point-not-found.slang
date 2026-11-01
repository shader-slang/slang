// entry-point-not-found.slang

// An unknown `-entry` name is reported once, as E38000 with the name and a
// location, not also as an empty "undefined identifier" from the lookup.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry mian
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target cuda -entry mian

// CHECK-NOT: E30015
// CHECK-NOT: undefined identifier
// CHECK: error[E38000]: no function found matching entry point name 'mian'
// CHECK-NOT: E30015
// CHECK-NOT: undefined identifier

[numthreads(64, 1, 1)]
[shader("compute")]
void main(uint3 tid: SV_DispatchThreadID, uniform uint* o)
{
    o[tid.x] = tid.x;
}
