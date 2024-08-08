//TEST:SIMPLE: -target spirv

void write<params T : __BuiltinFloatingPointType>(expand each T value)
{
}

[numthreads(1,1,1)]
void computeMain()
{
    write(1.0, 2.0, 3.0, 4.0);
}