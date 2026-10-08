// Khronos entry points can combine multiple logical payload parameters. An empty parameter
// must not add a dummy when another parameter already supplies physical incoming payload data.
// The ordinary helper still receives only its scalar argument after empty parameters disappear.

//TEST:SIMPLE(filecheck=GLSL): -entry main -stage miss -target glsl -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -entry main -stage callable -target glsl -DUSE_CALLABLE=1 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage miss -target spirv -DUSE_CALLABLE=0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage callable -target spirv -DUSE_CALLABLE=1 -validate-ir

struct Empty
{
}

struct Payload
{
    Empty values[2];
}

RWStructuredBuffer<uint> result;

[noinline]
[noRefInline]
void useEmpty(__ref Payload empty, uint value)
{
    result[1] = value;
}

#if USE_CALLABLE
[shader("callable")]
#else
[shader("miss")]
#endif
void main(inout Payload empty, inout uint value)
{
    value += 1;
    useEmpty(empty, value);
    result[0] = value;
}

// GLSL-NOT: _slang_dummy
// GLSL-DAG: {{rayPayloadInEXT|callableDataInEXT}}
// GLSL-DAG: void [[helper:useEmpty[A-Za-z0-9_]*]](uint {{[A-Za-z0-9_]+}})
// GLSL-NOT: _slang_dummy
// GLSL: void main()
// GLSL-NOT: _slang_dummy
// GLSL: [[helper]]({{[^,;]+}});
// GLSL-NOT: _slang_dummy

// SPIRV-NOT: _slang_dummy
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer Incoming{{RayPayload|CallableData}}KHR %{{[A-Za-z0-9_]+}}
// SPIRV-NOT: _slang_dummy
// SPIRV: = OpVariable %[[pointer]] Incoming{{RayPayload|CallableData}}KHR
// SPIRV-NOT: _slang_dummy
