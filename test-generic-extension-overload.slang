// Test case for generic method extension overload resolution issue

interface IStorage
{}
interface IBindlessStorage : IStorage
{}
struct Bindless : IBindlessStorage
{}

struct F<T:IStorage>
{
    void method<int u>(int x) {
        printf("base method with param\n");
    }
}

extension<T:IBindlessStorage> F<T>
{
    void method<int u>() {
        printf("extension method no param\n");
    }
}

struct G<T:IStorage>
{
    F<T> l;
}

extension<T:IBindlessStorage> G<T>
{
    void caller() { l.method<3>(); }
}

void main()
{
    G<Bindless> g;
    g.caller();
}