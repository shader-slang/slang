// An OptiX direct call emits the physical arguments that survive type legalization. A payload
// whose only field is itself empty therefore contributes no caller argument or callee parameter,
// just like a directly empty struct. This also verifies that CUDA does not depend on a shallow
// zero-field padding heuristic.

//TEST:SIMPLE(filecheck=CALLER): -stage raygeneration -entry rgen -target cuda
//TEST:SIMPLE(filecheck=CALLEE): -stage callable -entry callableMain -target cuda

struct Empty
{
}

struct TransitivelyEmpty
{
    Empty value;
}

[shader("callable")]
void callableMain(inout TransitivelyEmpty data)
{
}

[shader("raygeneration")]
void rgen()
{
    TransitivelyEmpty data;
    CallShader(0, data);
}

// CALLER-NOT: _slang_dummy
// CALLER-LABEL: void __raygen__rgen()
// CALLER: optixDirectCall<void>(0U);

// CALLEE-NOT: _slang_dummy
// CALLEE: void __direct_callable__callableMain()
