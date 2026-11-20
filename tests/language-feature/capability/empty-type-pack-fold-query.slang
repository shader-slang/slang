//TEST:SIMPLE(filecheck=EMPTY):-target hlsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=NONEMPTY,non-exhaustive):-target hlsl -whole-program -DNONEMPTY_PACK=1
//TEST:SIMPLE(filecheck=EMPTY):-target hlsl -whole-program -DSKIPPED_OPERAND=1
//DIAGNOSTIC_TEST:SIMPLE(diag=NONEMPTY,non-exhaustive):-target hlsl -whole-program -DNONEMPTY_PACK=1 -DSKIPPED_OPERAND=1
// non-exhaustive: incompatible dependency diagnostics include source notes.

[require(metal)]
interface IMetalItem
{}

struct MetalItem : IMetalItem
{}

struct Items<each T> where expand each T : IMetalItem
{
    int payload;
    static const int count = countof(T);
}

int readInt<let value : int>() { return value; }

// EMPTY: int itemCount
[require(hlsl)]
export int itemCount()
//NONEMPTY: 'itemCount' has dependencies that are not compatible on the required compilation target 'hlsl'.
{
    // Folding a type query must preserve the same element-wise constraint as ordinary use.
    // The empty pack has no element whose conformance could require Metal.
#if NONEMPTY_PACK
#if SKIPPED_OPERAND
    return readInt<true ? 1 : Items<MetalItem>.count>();
#else
    return readInt<sizeof(Items<MetalItem>)>();
#endif
#else
#if SKIPPED_OPERAND
    return readInt<true ? 1 : Items<>.count>();
#else
    return readInt<sizeof(Items<>)>();
#endif
#endif
}
