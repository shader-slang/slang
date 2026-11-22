//TEST:EXECUTABLE:

// A `[DllImport]`/`[DllExport]` signature accepts plain-old-data types: scalars, vectors, structs
// and fixed-size arrays of them, `out` parameters that point at such values, and `NativeString`.
// A `String` is accepted only as an input parameter, which the import borrows as `const char*`
// and the export copies into a new `String`. `dll-signature-managed-types.slang` checks that every
// other use of a managed type is rejected.

struct Pair
{
    int first;
    float second[2];
}

[DllExport]
export Pair makePair(int first, float2 second)
{
    Pair result;
    result.first = first;
    result.second[0] = second.x;
    result.second[1] = second.y;
    return result;
}

[DllImport("", "makePair")]
Pair makePairImported(int first, float2 second);

[DllExport]
export void measure(String text, out int length)
{
    length = text.length;
}

[DllImport("", "measure")]
void measureImported(String text, out int length);

export __extern_cpp int main()
{
    Pair pair = makePairImported(7, float2(0.5, 0.25));
    printf("%d %g %g\n", pair.first, pair.second[0], pair.second[1]);

    int length = 0;
    measureImported("hello", length);
    printf("%d\n", length);
    return 0;
}
