interface IStorage
{}
interface IBindlessStorage : IStorage
{}
struct Bindless : IBindlessStorage
{}

struct F<T:IStorage>
{
    void method<int u>(int x) {
        printf("l::method(int);\n");
    }
}

extension<T:IBindlessStorage> F<T>
{
    void method<int u>() {
        printf("l::method();\n");
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