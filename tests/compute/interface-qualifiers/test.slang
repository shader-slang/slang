//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

interface IFoo
{
}

struct Meh
{
    int a;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    Meh* bad2;
    some IFoo bad1;
    
    buffer[0] = 0;
}