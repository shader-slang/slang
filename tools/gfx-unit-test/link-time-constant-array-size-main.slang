extern static const int N;

struct S { int xs[N]; }

RWStructuredBuffer<S> b;

[numthreads(1, 1, 1)]
void computeMain()
{
    for(int i = 0; i < N; ++i)
        b[0].xs[i] *= N;
}
