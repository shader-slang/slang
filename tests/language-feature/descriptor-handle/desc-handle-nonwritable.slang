//TEST:SIMPLE(filecheck=SPVDH): -target spirv -capability spvDescriptorHeapEXT

// SPVDH-DAG: OpDecorate {{.*}} BuiltIn ResourceHeapEXT
// SPVDH-DAG: OpBufferPointerEXT {{.*}}; NonWritable 

uniform StructuredBuffer<int>.Handle t;
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain()
{
    output[0] = t[0];
}