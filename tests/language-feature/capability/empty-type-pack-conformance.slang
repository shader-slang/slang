//TEST:SIMPLE(filecheck=EMPTY): -target hlsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(filecheck=NONEMPTY): -target hlsl -whole-program -DNONEMPTY_PACK=1

// An empty element-wise constraint carries no target requirement. A nonempty pack must
// still propagate the capability required by its element's conformance.
//EMPTY: return int(0);
//NONEMPTY: dependencies not compatible on target

[require(metal)]
interface IMetalItem
{}

struct MetalItem : IMetalItem
{}

interface IItemList
{
    static const int count;
}

struct Items<each T> : IItemList
    where expand each T : IMetalItem
{
    static const int count = countof(T);
}

struct ReadCount<T : IItemList>
{
    static const int value = T.count;
}

[require(hlsl)]
export int itemCount()
{
#if NONEMPTY_PACK
    return ReadCount<Items<MetalItem>>.value;
#else
    return ReadCount<Items<>>.value;
#endif
}
