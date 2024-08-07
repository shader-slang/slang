//TEST:SIMPLE: -target spirv

RWStructuredBuffer<float> outputBuffer;

void accept<params T>(expand each T value)
{

}

T writeSingle<T : __BuiltinFloatingPointType>(T value)
{
    static int counter = 0;
    outputBuffer[counter++] = __realCast<float>(value);
}

void write<params T : __BuiltinFloatingPointType>(expand each T value)
{
    accept(expand writeSingle(each value));
}

[numthreads(1,1,1)]
void computeMain()
{
    write(1.0, 2.0, 3.0, 4.0);
}