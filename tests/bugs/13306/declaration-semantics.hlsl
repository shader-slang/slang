//TEST:SIMPLE_EX(filecheck=REMOVED): tests/bugs/13306/declaration-semantics.hlsl -target hlsl -entry main -no-codegen -DTRY_REMOVED
//TEST:SIMPLE_EX(filecheck=REMOVED): tests/bugs/13306/declaration-semantics.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_REMOVED
//TEST:SIMPLE_EX(filecheck=DEPRECATED): tests/bugs/13306/declaration-semantics.hlsl -target hlsl -entry main -no-codegen -DTRY_DEPRECATED
//TEST:SIMPLE_EX(filecheck=DEPRECATED): tests/bugs/13306/declaration-semantics.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_DEPRECATED
//TEST:SIMPLE_EX(filecheck=WRITEONLY): tests/bugs/13306/declaration-semantics.hlsl -target hlsl -entry main -no-codegen -DTRY_WRITEONLY
//TEST:SIMPLE_EX(filecheck=WRITEONLY): tests/bugs/13306/declaration-semantics.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_WRITEONLY
//TEST:SIMPLE_EX(filecheck=WRITEONLY_NUMERIC): tests/bugs/13306/declaration-semantics.hlsl -target hlsl -entry main -no-codegen -DTRY_WRITEONLY_NUMERIC
//TEST:SIMPLE_EX(filecheck=WRITEONLY_NUMERIC): tests/bugs/13306/declaration-semantics.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_WRITEONLY_NUMERIC
//TEST:SIMPLE_EX(filecheck=READONLY_NUMERIC): tests/bugs/13306/declaration-semantics.hlsl -target hlsl -entry main -no-codegen -DTRY_READONLY_NUMERIC
//TEST:SIMPLE_EX(filecheck=READONLY_NUMERIC): tests/bugs/13306/declaration-semantics.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_READONLY_NUMERIC
//TEST:SIMPLE_EX(filecheck=READONLY_BUFFER): tests/bugs/13306/declaration-semantics.hlsl -target hlsl -entry main -no-codegen -DTRY_READONLY_BUFFER
//TEST:SIMPLE_EX(filecheck=READONLY_BUFFER): tests/bugs/13306/declaration-semantics.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_READONLY_BUFFER

// Usage checks must read the parameter's declaration attributes after lookup finds its shadow.
// Usage checks must still diagnose removed and deprecated declarations when a mutable copy exists.
// Qualifier checking must still reject reads from write-only parameters and writes to `readonly`
// parameters, whether lowering creates mutable storage or aliases the parameter value.
#if defined(TRY_REMOVED)
[RemovedSince(-1, "removed parameter")]
uniform uint input;
#elif defined(TRY_DEPRECATED)
[deprecated("deprecated parameter")]
uniform uint input;
#elif defined(TRY_WRITEONLY)
writeonly RWStructuredBuffer<uint> input;
#elif defined(TRY_WRITEONLY_NUMERIC)
writeonly uniform uint input;
#elif defined(TRY_READONLY_NUMERIC)
readonly uniform uint input;
#elif defined(TRY_READONLY_BUFFER)
readonly cbuffer Settings
{
    uint value;
};
#endif
RWStructuredBuffer<uint> output;

// REMOVED-NOT: $uniformParameter_
// REMOVED: error[E31207]: use of removed declaration
// REMOVED: input has been removed since language version '-1': removed parameter
// REMOVED-NOT: error[
// REMOVED-NOT: $uniformParameter_

// DEPRECATED-NOT: error[
// DEPRECATED-NOT: $uniformParameter_
// DEPRECATED: warning[E31200]: use of deprecated declaration
// DEPRECATED: input has been deprecated: deprecated parameter
// DEPRECATED-NOT: error[
// DEPRECATED-NOT: $uniformParameter_

// WRITEONLY: error[E30119]: cannot read from writeonly
// WRITEONLY-NOT: error[

// WRITEONLY_NUMERIC: error[E30119]: cannot read from writeonly
// WRITEONLY_NUMERIC-NOT: error[

// READONLY_NUMERIC: error[E30011]: left of '=' is not an l-value
// READONLY_NUMERIC-NOT: error[

// READONLY_BUFFER: error[E30011]: left of '=' is not an l-value
// READONLY_BUFFER-NOT: error[

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
#if defined(TRY_WRITEONLY)
    // Qualifier checking must reject initializing a local from a write-only resource parameter.
    RWStructuredBuffer<uint> copy = input;
    output[0] = copy[0];
#elif defined(TRY_WRITEONLY_NUMERIC)
    // Qualifier checking must reject initializing a local from a write-only numeric parameter,
    // even when compatibility creates mutable storage for that parameter.
    uint copy = input;
    output[0] = copy;
#elif defined(TRY_READONLY_NUMERIC)
    // The compatibility option permits ordinary parameter writes, but qualifier checking must
    // still reject this write because the programmer explicitly declared `input` as `readonly`.
    input = 1;
    output[0] = input;
#elif defined(TRY_READONLY_BUFFER)
    // Lookup finds `value` through the legacy buffer's transparent shadow. Qualifier checking
    // must preserve the enclosing buffer's `readonly` restriction when checking this field write.
    value = 1;
    output[0] = value;
#else
    output[0] = input;
#endif
}
