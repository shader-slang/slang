//TEST_DISABLED:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile cs_4_0 -entry CSScatterVertexTriIDIndexID -entry CSScatterIndexTriIDIndexID
//--------------------------------------------------------------------------------------
// File: TessellatorCS40_ScatterIDCS.hlsl
//
// The CS to scatter vertex ID and triangle ID
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
StructuredBuffer<uint2> InputScanned : register(t0);
RWStructuredBuffer<uint2> TriIDIndexIDOut : register(u0);

cbuffer cbCS : register(b1)
{
    uint4 g_param;
}

[numthreads(128, 1, 1)]
void CSScatterVertexTriIDIndexID( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_param.x)
    {
        uint start = InputScanned[DTid.x-1].x;
        uint end = InputScanned[DTid.x].x;

        for ( uint i = start; i < end; ++i ) 
        {
            TriIDIndexIDOut[i] = uint2(DTid.x, i - start);
        }
    }
}

[numthreads(128, 1, 1)]
void CSScatterIndexTriIDIndexID( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x < g_param.x)
    {
        uint start = InputScanned[DTid.x-1].y;
        uint end = InputScanned[DTid.x].y;

        for ( uint i = start; i < end; ++i ) 
        {
            TriIDIndexIDOut[i] = uint2(DTid.x, i - start);
        }
    }
}
