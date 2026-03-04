//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct DefaultStruct_base
{
    int data0;
    __init()
    {
        data0 = 2;
    }
};

struct DefaultStruct1 : DefaultStruct_base
{
    int data1 = 1;
};

struct DefaultStruct2 : DefaultStruct_base
{

};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DefaultStruct1 s1 = {};
//CHECK:                ^ cannot use initializer list

    DefaultStruct1 s2 = {1};
//CHECK:                ^ cannot use initializer list

    DefaultStruct1 s3 = {1, 2};
//CHECK:                ^ cannot use initializer list

    DefaultStruct2 s4 = {};
//CHECK:                ^ cannot use initializer list

    DefaultStruct2 s5 = {1};
//CHECK:                ^ cannot use initializer list
}
