//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry geoMain -stage geometry -O0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=PRESENT): -target spirv -entry geoMain -stage geometry -O0 -emit-spirv-directly

// Third companion to numthreads-on-non-entry-point.slang, for the stage attributes that reach the
// same `dstID` execution-mode path: `[instance]` here, and `[domain]` / `[outputcontrolpoints]` in
// the tessellation tests. A geometry entry point is used because `Invocations` requires the Geometry
// capability, so this attribute cannot be exercised on the compute or fragment companions.
//
// On a merely-*called* function the attribute may not contribute an execution mode: SPIR-V requires
// the first operand of `OpExecutionMode` to be an entry point.

// CHECK-NOT: OpExecutionMode %helper

// `-O0` keeps `helper` from being inlined, so the CHECK-NOT above is able to fail if the mode comes
// back. Assert the callee is still emitted and that the entry point kept its own mode.
// PRESENT-DAG: %helper = OpFunction
// PRESENT-DAG: OpExecutionMode %geoMain Invocations 2

struct GSOut
{
    float4 pos : SV_Position;
};

[instance(4)]
float4 helper(float4 p)
{
    return p;
}

[shader("geometry")]
[instance(2)]
[maxvertexcount(3)]
void geoMain(triangle float4 pos[3], inout TriangleStream<GSOut> outStream)
{
    GSOut o;
    o.pos = helper(pos[0]);
    outStream.Append(o);
}
