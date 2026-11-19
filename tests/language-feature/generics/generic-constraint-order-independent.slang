//TEST:INTERPRET(filecheck=CHECK):

// A generic constraint may depend on another constraint declared later in the source. IR lowering
// must represent the complete checked constraint set instead of depending on source order.
interface IStorage<Element>
    where Element : __BuiltinFloatingPointType
{
    Element load();
}

struct FloatStorage : IStorage<float>
{
    float load()
    {
        return 12.0;
    }
}

Element loadStorage<Element, Storage>(Storage storage)
    where Storage : IStorage<Element>
    where Element : __BuiltinFloatingPointType
{
    return storage.load();
}

// The caller deliberately declares the same constraints in the opposite order. Each generic keeps
// its own source-order signature, so specializing `loadStorage` must preserve its argument roles.
Element forwardLoad<Element, Storage>(Storage storage)
    where Element : __BuiltinFloatingPointType
    where Storage : IStorage<Element>
{
    return loadStorage<Element, Storage>(storage);
}

void main()
{
    printf("%d\n", int(forwardLoad<float, FloatStorage>(FloatStorage())));
    // CHECK: 12
}
