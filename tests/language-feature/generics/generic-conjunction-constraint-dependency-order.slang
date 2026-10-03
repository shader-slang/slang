//TEST:INTERPRET(filecheck=CHECK):

interface IStorage<Element>
    where Element : __BuiltinFloatingPointType
{
    Element load();
}

interface IMarker
{
    int marker();
}

struct CombinedStorage : IStorage<float>, IMarker
{
    float load()
    {
        return 1.0;
    }

    int marker()
    {
        return 2;
    }
}

Element loadCombined<Element, Storage>(Storage storage)
    where Storage : IStorage<Element> & IMarker
    where Element : __BuiltinFloatingPointType
{
    return storage.load() + Element(storage.marker());
}

// Calling between opposite source orders checks that each flattened conjunction witness remains
// paired with its source-order parameter when the generic is specialized.
Element forwardCombined<Element, Storage>(Storage storage)
    where Element : __BuiltinFloatingPointType
    where Storage : IStorage<Element> & IMarker
{
    return loadCombined<Element, Storage>(storage);
}

void main()
{
    printf("%d\n", int(forwardCombined<float, CombinedStorage>(CombinedStorage())));
    // CHECK: 3
}
