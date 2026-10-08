//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

extension uint2
{
    __implicit_conversion(0)
    __init(int2 value)
    {
        this = value.x == 0 ? uint2(17u) : uint2(23u);
    }
}

void mutateConvertedValue(inout uint2 value)
{
    value = uint2(1u);
}

void rejectUserDefinedConversionWriteback()
{
    int2 value = int2(0);

    // Writable-argument lowering only knows how to reverse builtin numeric casts. Treating this
    // user-defined initializer as the builtin int-to-uint cast would silently discard its body.
    mutateConvertedValue(value);
//CHECK:                 ^^^^^ argument must be l-value
//CHECK:                 ^^^^^ argument passed to parameter '0' must be l-value.
//CHECK:                 ^^^^^ argument was implicitly cast from 'vector<int,2>' to 'vector<uint,2>', and Slang does not support using an implicit cast as an l-value for this usage
}
