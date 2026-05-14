//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -O0 -skip-spirv-validation -warnings-disable 38052

// Companion to stream-output-on-vertex.slang, exercising the same
// rule applied via the entry-point return type rather than a
// parameter (issue #9430).

[shader("vertex")]
LineStream<int> main()
//diag:         ^^^^ type cannot be used as entry-point varying parameter or return type
//diag:         ^^^^ type 'LineStream<int>' cannot be used as output return type of entry point 'main' because stream output types are only valid on a geometry shader entry point
{
    return {};
}
