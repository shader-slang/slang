//TEST:SIMPLE(filecheck=METAL): -target metallib -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage vertex -entry vertexMain

//METAL-DAG: position
//METALLIB: @vertexMain

//WGSL-DAG: @builtin(position)
//WGSL-DAG: @vertex

// Vertex Shader

float4 vertexMain()
{
	return float4(1,1,1,1);
}
