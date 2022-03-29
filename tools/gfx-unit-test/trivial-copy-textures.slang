// trivial-copy-textures.slang

typedef uint4 Element;

// Copy the contents of "src" into "dst". These are for 2D textures containing UINT data.
[shader("compute")]
[numthreads(1,1,1)]
void resourceViewTest(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform uint width,
    uniform uint height,
    uniform uint depth,
    uniform RWTexture3D<Element> resourceView,
    uniform RWStructuredBuffer<Element> testResults)
{
    uint tid = sv_dispatchThreadID.x;

    uint tmp = tid;
    uint x = tmp % width; tmp /= width;
    uint y = tmp % height; tmp /= height;
    uint z = tmp;

    uint3 coord = uint3(x, y, z);

    // Read from resourceView
    Element elem = resourceView[coord];

    // Write to resourceView (if possible)
    resourceView[coord] = Element(1);

    // Write something to testResults
    testResults[tid] = elem;
}

#if 0
typedef uint4 Element;

interface ITestCase
{
    Element doTest(uint3 coord);
}

// 1D + array, 2D + array + multisample + multisample array, cube + array

struct UAV_3D : ITestCase
{
    uniform RWTexture3D<Element> resourceView,

    Element doTest(uint3 coord)
    {
        // Read from resourceView
        Element elem = resourceView[coord];

        // Write to resourceView (if possible)
        resourceView[coord] = Element(1);

        return elem;
    }
}

struct UAV_2D : ITestCase
{
    uniform RWTexture2D<Element> resourceView,

    Element doTest(uint3 coord)
    {
        // Read from resourceView
        Element elem = resourceView[coord.xy];

        // Write to resourceView (if possible)
        resourceView[coord.xy] = Element(1);

        return elem;
    }
}

struct SRV_3D : ITestCase
{
    uniform Texture3D<Element> resourceView,

    Element doTest(uint3 coord)
    {
        // Read from resourceView
        Element elem = resourceView[coord];

        return elem;
    }
}

// Copy the contents of "src" into "dst". These are for 2D textures containing UINT data.
[shader("compute")]
[numthreads(1,1,1)]
void resourceViewTest<T:ITestCase>(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform uint width,
    uniform uint height,
    uniform uint depth,
    uniform T testCase,
    uniform RWStructuredBuffer<Element> testResults)
{
    uint tid = sv_dispatchThreadID.x;
    
    uint tmp = tid;
    uint x = tmp % width; tmp /= width;
    uint y = tmp % height; tmp /= height;
    uint z = tmp;

    uint3 coord = uint3(x, y, z);
    Element elem = testCase.doTest(coord);

    // Write something to testResults
    testResults[tid] = elem;
}
#endif
