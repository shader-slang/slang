// Regression test for issue #12049: specializing a generic entry point via
// `-specialize <type>` used to fail with `error[E38029]: type argument doesn't
// conform to interface` when the required conformance was provided by an
// `extension` declared in the primary (command-line) source file.
//
// The entry-point specialization boundary
// (`EntryPoint::_validateSpecializationArgsImpl`) checks the specialization
// argument's conformance in a `SharedSemanticsContext` scoped to the entry
// point's owning module. Before the fix, that context was unscoped
// (m_module == nullptr), so extension-lookup fell back to the linkage's
// `loadedModulesList` -- which never contains the primary translation unit --
// and the `float : IDenorm` / `uint32_t : IDenorm` witnesses were invisible.
//
// The conformance itself resolves fine everywhere else (in-body generic calls,
// manual wrapper entry points, or when the extensions live in an imported
// module); it failed only at this boundary. A successful emit -- i.e. the
// entry point makes it to valid SPIR-V without E38029 -- proves the witness is
// now found.

//TEST:SIMPLE(filecheck=CHECK_FLOAT): -target spirv-asm -entry shader -specialize float
//TEST:SIMPLE(filecheck=CHECK_UINT): -target spirv-asm -entry shader -specialize uint32_t

interface IDenorm
{
    static vector<This, 4> denorm(vector<float32_t, 4>);
}

extension<T : __BuiltinIntegerType & IRangedValue> T : IDenorm
{
    static vector<T, 4> denorm(vector<float32_t, 4> pixel)
    {
        pixel = round(pixel * __realCast<float32_t>(T.maxValue));
        return vector<T, 4>(pixel);
    }
}

extension<T : __BuiltinFloatingPointType> T : IDenorm
{
    static vector<T, 4> denorm(vector<float32_t, 4> pixel)
    {
        return vector<T, 4>(pixel);
    }
}

// CHECK_FLOAT: OpEntryPoint
// CHECK_FLOAT: OpName %shader "shader"
// CHECK_UINT: OpEntryPoint
// CHECK_UINT: OpName %shader "shader"

[shader("compute")]
[numthreads(64, 1, 1)]
void shader<T : IDenorm>(uint3 threadId: SV_DispatchThreadID)
{
    T.denorm(float4(1.0));
}
