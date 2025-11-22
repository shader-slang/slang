struct TestStruct 
{ 
    int a;
    float b;
    int c;
    float d;
}

RWStructuredBuffer<TestStruct> buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    TestStruct s;
    s.a = 1;
    s.b = 2.0f;
    s.c = 3;
    s.d = 4.0f;
    buffer[0] = s;
}