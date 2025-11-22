extern static const int ARRAY_SIZE;

struct TestStruct 
{ 
    int a;
    float b;
    int dynamicArray[ARRAY_SIZE];
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
    for(int i = 0; i < ARRAY_SIZE; ++i)
        s.dynamicArray[i] = 10 + i;
    s.c = 3;
    s.d = 4.0f;
    buffer[0] = s;
}