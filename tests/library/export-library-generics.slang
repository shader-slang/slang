//TEST_IGNORE_FILE:

// export-library-generics.slang

module "export-library-generics";

public cbuffer Constants {
    public float x;
    public float y;
}

interface MyInterface
{
    int myMethod(int a);
}

struct MyType : MyInterface
{
    int myMethod(int a)
    {
        return a * 3;
    }
}

int genericFunc<T: MyInterface>(T arg)
{
    return arg.myMethod(3);
}

public int normalFuncUsesGeneric(int a)
{
    MyType obj;
    return genericFunc(obj);
}

public int normalFunc(int a, float b)
{
    return a - floor(b);
}
