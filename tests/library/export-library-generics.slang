// This test checks the SPIR-V output when compiling a library containing generics with embedded downstream IR.

//TEST:SIMPLE(filecheck=CHECK): -o tests/library/export-library-generics-test.slang-module -target spirv -embed-downstream-ir -profile lib_6_6 -incomplete-library -dump-ir -verbose-paths

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

// CHECK:_SR29export_2Dxlibrary_2Dxgenerics10normalFuncp2pi_ii_fi