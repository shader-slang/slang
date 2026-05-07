//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation

// Companion to mesh-output-on-vertex.slang, exercising the same rule
// applied via the entry-point return type rather than a parameter
// (issue #9430, mesh-side analogue).

struct V {
    float4 pos : SV_Position;
};

[shader("vertex")]
OutputVertices<V, 3> main()
//diag:              ^^^^ type cannot be used as entry-point varying parameter or return type
//diag:              ^^^^ type 'OutputVertices<V, 3>' cannot be used as output return type of entry point 'main' because mesh output types are only valid on mesh shader entry points
{
    return {};
}
