//TEST:INTERPRET(filecheck=CHECK):

// Each constraint below needs a witness declared later in the source. This exercises more than a
// single forward reference: IR lowering must predeclare the complete constraint environment before
// it assigns parameter types for `loadHolder`.
interface IStorage<Element>
    where Element : __BuiltinFloatingPointType
{
    Element load();
}

interface IHolder<Element, Storage>
    where Storage : IStorage<Element>
    where Element : __BuiltinFloatingPointType
{
    Element load();
}

struct FloatStorage : IStorage<float>
{
    float load()
    {
        return 13.0;
    }
}

struct FloatHolder : IHolder<float, FloatStorage>
{
    float load()
    {
        return 13.0;
    }
}

Element loadHolder<Element, Storage, Holder>(Holder holder)
    where Holder : IHolder<Element, Storage>
    where Storage : IStorage<Element>
    where Element : __BuiltinFloatingPointType
{
    return holder.load();
}

void main()
{
    printf("%d\n", int(loadHolder<float, FloatStorage, FloatHolder>(FloatHolder())));
    // CHECK: 13
}
