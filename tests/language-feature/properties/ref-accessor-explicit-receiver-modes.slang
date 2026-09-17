//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

static int backing = 0;

struct Container
{
    int unused;

    property int nonmutatingValue
    {
        [nonmutating] ref { return backing; }
    }

    property int constRefValue
    {
        [constref] ref { return backing; }
    }
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let nonmutatingContainer = Container();
    nonmutatingContainer.nonmutatingValue = 7;

    // A const-reference receiver must be addressable, but it is not mutating.
    var constRefContainer = Container();
    constRefContainer.constRefValue += 2;
    outputBuffer[0] = backing;
}

// CHECK: 9
