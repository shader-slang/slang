//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// for-loop nested, with 1 write behind an if

import common;
void logic(out some IFoo v1)
{
    // CHECK: error 41404
    for (int i = 0; i < buffer[0]; i++)
    {
        for (int ii = 0; ii < buffer[1]; ii++)
        {
            if (buffer[2] == 0)
            {
                v1 = StructForIFoo1(1);
            }
        }
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}