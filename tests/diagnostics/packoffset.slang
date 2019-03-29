// packoffset.slang
//DIAGNOSTIC_TEST:SIMPLE:-target hlsl

// use of `packoffset` (not supported):
cbuffer B
{
	float4 x : packoffset(c0);
}

void main()
{}