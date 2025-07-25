interface IStorage
{}
interface IBindlessStorage : IStorage
{}
struct Bindless : IBindlessStorage
{}

struct F<T:IStorage>
{
    void method<int u>(int x) {
        printf("F::method(int) with u=%d\n", u);
    }
}

extension<T:IBindlessStorage> F<T>
{
    void method<int u>() {
        printf("F::method() extension with u=%d\n", u);
    }
}

struct G<T:IStorage>
{
    F<T> l;
}

extension<T:IBindlessStorage> G<T>
{
    void caller() { 
        // This should call the extension method (no parameters)
        // but compiler thinks it needs 1 parameter
        l.method<3>(); 
    }
}

void main()
{
    G<Bindless> g;
    g.caller();
}