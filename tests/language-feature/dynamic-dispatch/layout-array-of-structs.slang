// Arrays of structs as fields in interface implementations require the
// marshalling to handle array-of-composite layout: each struct element
// must be packed field-by-field in array order within the AnyValue.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Vertex
{
    float x;
    float y;
    float weight;
}

interface IShape
{
    float area();
    float totalWeight();
}

struct Triangle : IShape
{
    Vertex verts[3];

    float area()
    {
        float ax = verts[1].x - verts[0].x;
        float ay = verts[1].y - verts[0].y;
        float bx = verts[2].x - verts[0].x;
        float by = verts[2].y - verts[0].y;
        return abs(ax * by - ay * bx) * 0.5;
    }

    float totalWeight()
    {
        return verts[0].weight + verts[1].weight + verts[2].weight;
    }
}

struct Segment : IShape
{
    Vertex endpoints[2];
    float thickness;

    float area()
    {
        float dx = endpoints[1].x - endpoints[0].x;
        float dy = endpoints[1].y - endpoints[0].y;
        float len = sqrt(dx * dx + dy * dy);
        return len * thickness;
    }

    float totalWeight()
    {
        return endpoints[0].weight + endpoints[1].weight;
    }
}

IShape makeShape(int id)
{
    if (id == 0)
    {
        Triangle t;
        t.verts[0] = { 0.0, 0.0, 1.0 };
        t.verts[1] = { 4.0, 0.0, 2.0 };
        t.verts[2] = { 0.0, 3.0, 3.0 };
        return t;
    }
    else
    {
        Segment s;
        s.endpoints[0] = { 0.0, 0.0, 5.0 };
        s.endpoints[1] = { 3.0, 4.0, 7.0 };
        s.thickness = 2.0;
        return s;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Triangle: area = 0.5 * |4*3 - 0*0| = 6.0
    var tri = makeShape(id);
    outputBuffer[0] = tri.area();
    // CHECK: 6.0
    outputBuffer[1] = tri.totalWeight();
    // CHECK: 6.0

    // Segment: length = sqrt(9+16) = 5.0, area = 5.0 * 2.0 = 10.0
    var seg = makeShape(id + 1);
    outputBuffer[2] = seg.area();
    // CHECK: 10.0
    outputBuffer[3] = seg.totalWeight();
    // CHECK: 12.0
}
