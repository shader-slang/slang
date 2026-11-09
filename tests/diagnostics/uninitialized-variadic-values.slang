//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

Tuple<expand each T> readPack<each T>(expand each T values)
{
    return makeTuple(expand each values);
}

void writePack<each T>(Tuple<expand each T> source, out expand each T values)
{
    expand (each values = each source);
}

RWStructuredBuffer<float> output;

void readUninitializedPack()
{
    Tuple<float> values;
    output[0] = readPack(expand each values)._0;
//CHECK:                ^ use of uninitialized variable
//CHECK:                ^ use of uninitialized variable 'values'
}

void readBeforeWrite()
{
    Tuple<float, uint> values;
    output[1] = values._0;
//CHECK:               ^^ use of uninitialized variable
//CHECK:               ^^ use of uninitialized variable 'values'
    writePack(makeTuple(3.0, uint(7)), expand each values);
    output[2] = values._0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    readUninitializedPack();
    readBeforeWrite();
}
