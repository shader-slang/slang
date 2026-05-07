//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation

// Regression test for #9430 (companion to stream-output-on-vertex).
//
// Mesh output types are only meaningful on mesh shader entry points.

struct V {
    float4 pos : SV_Position;
};

[shader("vertex")]
void main(out OutputVertices<V, 3> verts)
//diag:                            ^^^^^ type cannot be used as entry-point varying parameter or return type
//diag:                            ^^^^^ type 'OutputVertices<V, 3>' cannot be used as output parameter 'verts' of entry point 'main' because mesh output types are only valid on mesh shader entry points
{
}
