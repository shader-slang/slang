//TEST:SIMPLE(filecheck=CHK): -target spirv -entry computeMain

extern static const int flag;

struct None
{
    int a;
    int b;

    __init()
    {
        a = 0;
        b = 0;
    }
}

struct Simple
{
    int value;
    Simple *next;

    __init(int v, Simple* ptr)
    {
        value = v;
        //CHK-DAG: warning 41020: exiting constructor without initializing field 'next'
    }
}

struct Conditional
{
    int field;
    int another_field;

    __init()
    {
        field = 0;
        if (flag == 0)
            //CHK-DAG: warning 41020: exiting constructor without initializing field 'another_field'
            return;

        another_field = 0;
    }
};

struct Partial
{
    int inline = 0;
    int body = 0;

    __init(int x)
    {
        body = x;
    }
}

// Warnings here should be a bit different
struct Implicit
{
    //CHK-DAG: warning 41021: default initializer for 'Implicit' will not initialize field 'a'
    int a;
    int b = 1;
    //CHK-DAG: warning 41021: default initializer for 'Implicit' will not initialize field 'c'
    int c;
    int d = 1 + 1;
}

int using_implicit(int x)
{
    Implicit impl;
    impl.a = x;
    return impl.c;
}

struct UnusedImplicit
{
    // Shouldn't warn here, since its never used
    int a;
    int b = 1;
    int c;
    int d = 1 + 1;
}

struct Pass
{
    int x = 0;
    int y = 0;
}

//CHK-NOT: warning 41020
//CHK-NOT: warning 41021

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}

