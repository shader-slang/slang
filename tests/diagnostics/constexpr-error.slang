//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Failure to pass compile-time-constant data
// where it is expected.
//
// In this case, the place where compile-time-constant
// data is expected is the texel offset parameter to
// the `Texture2D.Sample` operation.

Texture2D t;
SamplerState s;

cbuffer U
{
	float2 uv;
	uint2 offset;
};

float4 main() : SV_Target
{
    float4 result = 0.0f;

    // Okay, immediate constant
    result += t.Sample(s, uv, int2(0,0));

    // Error: data passed through cbuffer isn't compile-time constant
    result += t.Sample(s, uv, offset);
//CHECK:                      ^^^^^^ expected a compile-time constant

    // Error: data computed via conditional isn't compile-time cosntant
    uint ii = 0;
    if(uv.x > 0.0f)
    {
    	ii = 1;
    }
    result += t.Sample(s, uv, int2(ii));
//CHECK:                          ^ expected a compile-time constant

    // Error: data computed in loop isn't compile-time constant
    // (and loop isn't unroll-able).
    //
    // Historically the for-clause produced two diagnostics here -- one at
    // `jj` in the condition and a duplicate at `jj++`.  The duplicate was a
    // backward-propagation cascade: marking the `int2(jj)` argument as
    // requiring constexpr also marked every feeder in the loop chain,
    // including the `jj++` loop-back add.  After #11018 the feeder
    // diagnostic is suppressed because it points at loop syntax rather
    // than at the actual constexpr demand; only the condition-site and
    // demand-site diagnostics remain.
    for(uint jj = 0; jj < uv.y; jj++)
//CHECK:             ^^ expected a compile-time constant
    {
    	result += t.Sample(s, uv, int2(jj));
//CHECK:                           ^ expected a compile-time constant
    }




    return result;
}
