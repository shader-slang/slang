//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-invert-y
//TEST:SIMPLE(filecheck=POINT): -target spirv -stage mesh -entry pointMain

// CHECK-DAG: OpExecutionMode %main OutputLinesEXT
// CHECK: %[[CONST:[0-9]+]] = OpConstantComposite %v4float %float_0 %float_n1 %float_0 %float_0
// CHECK: OpStore {{.*}} %[[CONST]]

// POINT-DAG: OpExecutionMode %pointMain OutputPoints

struct PS_IN
{
	float4 vPositionPs : SV_Position;
};

[ shader("mesh") ]
[ outputtopology( "line" ) ]
[ numthreads( 64, 1, 1 ) ]
void main(
	uint nThreadId : SV_DispatchThreadID,
	uint nGroupThreadId : SV_GroupThreadID,
	uint nGroupId : SV_GroupID,
	out vertices PS_IN outputVerts[ 128 ],
	out indices uint2 outputIB[ 64  ] )
{
	SetMeshOutputCounts( 128, 64 );

    outputVerts[ 2 * nGroupThreadId ].vPositionPs = float4( 0, 1, 0, 0 );
}

[ shader("mesh") ]
[ outputtopology( "point" ) ]
[ numthreads( 1, 1, 1 ) ]
void pointMain(
	out vertices PS_IN outputVerts[ 1 ],
	out indices uint outputIB[ 1 ] )
{
	SetMeshOutputCounts( 1, 1 );

    outputVerts[ 0 ].vPositionPs = float4( 0, 1, 0, 0 );
    outputIB[ 0 ] = 0;
}
