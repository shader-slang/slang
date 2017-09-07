//TEST:COMPARE_HLSL: -use-ir -target dxbc-assembly -profile ps_4_0 -entry main

// We need to allow the user to add explicit bindings to their parameters,
// and we can't go and auto-assign anything to use the same locations.

#ifdef __SPIRE__
#define R(X) /**/
#else
#define R(X) X
#endif

float4 use(float4 val) { return val; };
float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

// We'll make three textures, but explicit assign the third one
// to the slot `t0`. We expect the others to shift further along
// to "make room".
Texture2D 		ta R(: register(t1));
Texture2D 		tb R(: register(t2));
Texture2D 		tc : register(t0);


// The explicit binding may "split" the range of register available
// for automatic placement. We use a "first-fit" approach to pack
// things in:
SamplerState 	sa R(: register(s0));
SamplerState 	sb R(: register(s2));
SamplerState 	sc : register(s1);

// It's also okay to use a register that *doesn't* conflict,
// and even to make things non-contiguous. Here we bind
// the third constnat buffer to register `b9`
//
cbuffer CA R(: register(b0))
{
	float ca;
}
//
cbuffer CB R(: register(b1))
{
	float cb;
}
//
cbuffer CC : register(b9)
{
	float cc;
}

float4 main() : SV_Target
{
	// Go ahead and use everything in this case:
	return use(ta, sa) + use(ca)
		+  use(tb, sb) + use(cb)
		+  use(tc, sc) + use(cc);
}