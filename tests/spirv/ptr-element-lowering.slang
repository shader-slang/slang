//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// CHECK: OpEntryPoint

struct SubElement
{
    float4 arr[12];
}
struct Element
{
    int *x;
    float4 v;
    int4x4 m1;
    int4x4 m2;
    SubElement subElems;
}

struct Scene
{
    Element* buffer;
}
uniform Scene* scene;

void f(int* x, float4 v, SubElement elems)
{
    *x = (int)(v.x + elems.arr[0].y);
}

[numthreads(1,1,1)]
void computeMain()
{
    var s = scene[0];
    var b = s.buffer[0];
    f(b.x, b.v, b.subElems);
}
