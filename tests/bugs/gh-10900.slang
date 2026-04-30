//DIAGNOSTIC_TEST:SIMPLE:
interface IBar
{
}

struct Thingy<T: IBar>
{
}

interface IFoo
{
    [mutating]
    Thingy<T> foo<T: IBar>();
}

struct Foo: IFoo
{
    Thingy<T> foo<T: IBar>()
    {
        return Thingy<T>();
    }
}
