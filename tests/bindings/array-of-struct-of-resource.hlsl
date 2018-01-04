//TEST:COMPARE_HLSL:-no-mangle -target dxbc-assembly -profile ps_5_1 -entry main

// Let's first confirm that Spire can reproduce what the
// HLSL compiler would already do in the simple case (when
// all shader parameters are actually used).

float4 use(Texture2D t, SamplerState s) { return t.Sample(s, 0.0); }

#ifdef __SLANG__

struct Test
{
	Texture2D a;
	Texture2D b;
};

Test test[2];
SamplerState s;

float4 main() : SV_Target
{
	return use(test[0].a,s)
		 + use(test[0].b,s)
		 + use(test[1].a,s)
		 + use(test[1].b,s);
}

#else

Texture2D test_a[2];
Texture2D test_b[2];
SamplerState s;

float4 main() : SV_Target
{
	return use(test_a[0],s)
		 + use(test_b[0],s)
		 + use(test_a[1],s)
		 + use(test_b[1],s);
}

#endif
