struct test {
 int a;
 int b;
 bool c;
 int value() { return c ? b : a; }
}

struct globals {
    int testcount;

    [__unsafeInlineEarly]
    __init(int testcount) {
        this.testcount = testcount;
    }
}

RWStructuredBuffer<int> results;

static const globals globalsbuffer[1] = { globals(2) };

static test tests[2] = { { 5, 2, true }, { 10, 20, false } };

[shader("compute")]
[numthreads(1, 1, 1)]
void printMain()
{
    int a = 0;

    // [ForceUnroll]
    for( uint nTestIndex = 0; nTestIndex  < globalsbuffer[0].testcount; nTestIndex++)
    {
      a += tests[nTestIndex].value();
    }
    results[0] = a;
}
