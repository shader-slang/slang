//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes how an aggregate hit-attribute struct is fetched from OptiX's eight 32-bit attribute slots, recursing through vectors, matrices and arrays in declaration order and bit-casting each float slot with __int_as_float.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `emitOptiXAttributeFetch`
// (slang-ir-legalize-varying-params.cpp:2058). Custom intersection attributes
// arrive in OptiX as up to eight untyped 32-bit slots read one at a time with
// `optixGetAttribute_N`, so an aggregate attribute parameter has to be
// reassembled slot by slot. The fetch walks the type recursively — struct
// fields, then vector elements, then matrix rows (each row itself a vector),
// then array elements — consuming one slot per scalar in declaration order,
// and bit-casts a float slot with `__int_as_float` while an integer slot is
// used as-is. This attribute struct is chosen to be exactly eight slots wide
// and to mix all three aggregate shapes, so CHECK can assert the full slot
// assignment: `uv` takes 0-1, `frame` takes 2-5 in row order, and the uint
// array takes 6-7 with no cast.
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry chit -stage closesthit

struct Attribs
{
    float2 uv;
    float2x2 frame;
    uint ids[2];
}
struct Payload
{
    float4 color;
}

[shader("closesthit")]
void chit(inout Payload p, in Attribs attr)
{
    p.color = float4(attr.uv, attr.frame[0][1], float(attr.ids[1]));
}

// The float2 takes slots 0-1 and is rebuilt with make_float2.
//CUDA: float _S{{[0-9]+}} = __int_as_float(optixGetAttribute_0());
//CUDA: float _S{{[0-9]+}} = __int_as_float(optixGetAttribute_1());
//CUDA: float2  _S{{[0-9]+}} = make_float2 (
// The 2x2 matrix takes slots 2-5, one row (a float2) at a time.
//CUDA: float _S{{[0-9]+}} = __int_as_float(optixGetAttribute_2());
//CUDA: float _S{{[0-9]+}} = __int_as_float(optixGetAttribute_3());
//CUDA: float _S{{[0-9]+}} = __int_as_float(optixGetAttribute_4());
//CUDA: float _S{{[0-9]+}} = __int_as_float(optixGetAttribute_5());
//CUDA: Matrix<float, 2, 2>  _S{{[0-9]+}} = makeMatrix<float, 2, 2> (
// The uint array takes slots 6-7 and needs no bit-cast.
//CUDA: uint _S{{[0-9]+}} = optixGetAttribute_6();
//CUDA: uint _S{{[0-9]+}} = optixGetAttribute_7();
