//TEST:SIMPLE(filecheck=METAL1): -target metal -stage vertex -entry vertexMain1
//TEST:SIMPLE(filecheck=METALLIB1): -target metallib -stage vertex -entry vertexMain1
//TEST:SIMPLE(filecheck=METAL2): -target metal -stage vertex -entry vertexMain2
//TEST:SIMPLE(filecheck=METALLIB2): -target metallib -stage vertex -entry vertexMain2

//METALLIB1: @vertexMain1
//METAL1-DAG: attribute(0)
//METAL1-DAG: attribute(1)
//METAL1-NOT: attribute(2)

//METALLIB2: @vertexMain2
//METAL2-DAG: attribute(0)
//METAL2-DAG: attribute(1)
//METAL2-DAG: attribute(2)

struct SharedStruct
{
    float4 position;
    float4 color;
};

struct VertexStageInput
{
	SharedStruct assembledVertex	: CoarseVertex;
};

float4 vertexMain1(VertexStageInput vertex)
{
	return vertex.assembledVertex.position;
}

struct sharedStructWrapper
{
    float2 uv;
    SharedStruct sharedData;
};
struct VertexStageInput2
{
	sharedStructWrapper assembledVertex	: CoarseVertex;
};

float4 vertexMain2(VertexStageInput2 vertex)
{
	return vertex.assembledVertex.sharedData.position;
}