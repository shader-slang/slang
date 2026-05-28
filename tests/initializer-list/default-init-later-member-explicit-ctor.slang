//TEST:SIMPLE: -stage compute -entry main

struct ExplicitDefault
{
    int value;

    __init(int value = 0)
    {
        this.value = value;
    }
}

struct UsesLaterType
{
    LaterType later;
}

struct LaterType
{
    ExplicitDefault leaf;
}

struct Outer
{
    UsesLaterType data;
}

void init(inout Outer o)
{
    o = {};
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Outer o;
    init(o);
}
