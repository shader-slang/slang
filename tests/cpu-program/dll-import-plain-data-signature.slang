//TEST:EXECUTABLE:

// A `[DllImport]`/`[DllExport]` signature accepts plain-old-data types: scalars, vectors, structs,
// tuples, and fixed-size arrays of them, raw pointers to such values (including self-referential
// structs), `out` parameters that point at such values, and `NativeString`. A `String` is accepted
// only as an input parameter, which the import borrows as `const char*` and the export copies into
// a new `String`. `dll-signature-managed-types.slang` checks that every other use of a managed
// type is rejected.

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

// The import passes `Pair` by value through its function pointer, so the export must also take it
// by value rather than as the borrow that ordinary C++ functions get for aggregate parameters.
[DllExport]
export float sumPair(Pair value)
{
    return float(value.first) + value.second[0] + value.second[1];
}

[DllImport("", "sumPair")]
float sumPairImported(Pair value);

// Tuples reach the DLL passes before `lowerTuples` and are classified by their element types.
[DllExport]
export float sumTuple(Tuple<int, float> value)
{
    return float(value._0) + value._1;
}

[DllImport("", "sumTuple")]
float sumTupleImported(Tuple<int, float> value);

// `next` refers back to `ListNode`, so classifying the type revisits it, and the C++ emitter reaches
// `ListNode*` from the signature before it reaches `ListNode`.
struct ListNode
{
    int value;
    ListNode* next;
}

[DllExport]
export int sumList(ListNode* head)
{
    int sum = 0;
    for (ListNode* node = head; node != nullptr; node = node->next)
        sum += node->value;
    return sum;
}

[DllImport("", "sumList")]
int sumListImported(ListNode* head);

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
    printf("%g\n", sumPairImported(pair));
    printf("%g\n", sumTupleImported(makeTuple(2, 0.5f)));

    ListNode second = { 5, nullptr };
    ListNode first = { 3, &second };
    printf("%d\n", sumListImported(&first));

    int length = 0;
    measureImported("hello", length);
    printf("%d\n", length);
    return 0;
}
