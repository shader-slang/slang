//TEST:SIMPLE(filecheck=METAL): -target metallib -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain

//METAL-DAG: position
//METALLIB: @vertexMain

// Vertex Shader

float4 vertexMain()
{
	return float4(1,1,1,1);
}