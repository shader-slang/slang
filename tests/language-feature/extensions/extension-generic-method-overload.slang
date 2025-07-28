//TEST:INTERPRET(filecheck=CHECK):

interface IStorage
{}
interface IBindlessStorage : IStorage
{}
struct Bindless : IBindlessStorage
{}

struct F<T:IStorage>
{
    void method<int u>(int x) {
        printf("F.method(int): %d\n", x);
    }
}

extension<T:IBindlessStorage> F<T>
{
    void method<int u>() {
        printf("F.method(): no params\n");
    }
}

struct G<T:IStorage>
{
    F<T> l;
}

extension<T:IBindlessStorage> G<T>
{
    void caller() { 
        l.method<3>(); 
    }
    void callerWithParam() {
        l.method<3>(42);
    }
}

void main()
{
    G<Bindless> g;
    
    // This should call the zero-parameter version from the extension
    //CHECK: F.method(): no params
    g.caller();
    
    // This should call the one-parameter version from the base struct  
    //CHECK: F.method(int): 42
    g.callerWithParam();
}