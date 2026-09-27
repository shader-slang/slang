// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3 -Xslang -warnings-disable -Xslang 31227
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -warnings-disable -Xslang 31227
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -warnings-disable -Xslang 31227

// The imported source uses constexpr variables, which Slang treats as const (E31227).
// Suppress only that known compatibility warning in this output-checking fixture.
import complex.tiled_brass_material;

// Qualify the imported mini-LUT policy with synthetic data, independently of the analytic graph.
// Roughness stays in [0,1]; cosine queries include the helper's explicit endpoint clamps.
// Row-major T[r,i]=(32+8*r+4*i+r*i)/512, except T[15,15]=1.
//TEST_INPUT: set lut_globals.mini_microfacet_ggx_energy = ubuffer(data=[0.0625 0.0703125 0.078125 0.0859375 0.09375 0.1015625 0.109375 0.1171875 0.125 0.1328125 0.140625 0.1484375 0.15625 0.1640625 0.171875 0.1796875 0.078125 0.087890625 0.09765625 0.107421875 0.1171875 0.126953125 0.13671875 0.146484375 0.15625 0.166015625 0.17578125 0.185546875 0.1953125 0.205078125 0.21484375 0.224609375 0.09375 0.10546875 0.1171875 0.12890625 0.140625 0.15234375 0.1640625 0.17578125 0.1875 0.19921875 0.2109375 0.22265625 0.234375 0.24609375 0.2578125 0.26953125 0.109375 0.123046875 0.13671875 0.150390625 0.1640625 0.177734375 0.19140625 0.205078125 0.21875 0.232421875 0.24609375 0.259765625 0.2734375 0.287109375 0.30078125 0.314453125 0.125 0.140625 0.15625 0.171875 0.1875 0.203125 0.21875 0.234375 0.25 0.265625 0.28125 0.296875 0.3125 0.328125 0.34375 0.359375 0.140625 0.158203125 0.17578125 0.193359375 0.2109375 0.228515625 0.24609375 0.263671875 0.28125 0.298828125 0.31640625 0.333984375 0.3515625 0.369140625 0.38671875 0.404296875 0.15625 0.17578125 0.1953125 0.21484375 0.234375 0.25390625 0.2734375 0.29296875 0.3125 0.33203125 0.3515625 0.37109375 0.390625 0.41015625 0.4296875 0.44921875 0.171875 0.193359375 0.21484375 0.236328125 0.2578125 0.279296875 0.30078125 0.322265625 0.34375 0.365234375 0.38671875 0.408203125 0.4296875 0.451171875 0.47265625 0.494140625 0.1875 0.2109375 0.234375 0.2578125 0.28125 0.3046875 0.328125 0.3515625 0.375 0.3984375 0.421875 0.4453125 0.46875 0.4921875 0.515625 0.5390625 0.203125 0.228515625 0.25390625 0.279296875 0.3046875 0.330078125 0.35546875 0.380859375 0.40625 0.431640625 0.45703125 0.482421875 0.5078125 0.533203125 0.55859375 0.583984375 0.21875 0.24609375 0.2734375 0.30078125 0.328125 0.35546875 0.3828125 0.41015625 0.4375 0.46484375 0.4921875 0.51953125 0.546875 0.57421875 0.6015625 0.62890625 0.234375 0.263671875 0.29296875 0.322265625 0.3515625 0.380859375 0.41015625 0.439453125 0.46875 0.498046875 0.52734375 0.556640625 0.5859375 0.615234375 0.64453125 0.673828125 0.25 0.28125 0.3125 0.34375 0.375 0.40625 0.4375 0.46875 0.5 0.53125 0.5625 0.59375 0.625 0.65625 0.6875 0.71875 0.265625 0.298828125 0.33203125 0.365234375 0.3984375 0.431640625 0.46484375 0.498046875 0.53125 0.564453125 0.59765625 0.630859375 0.6640625 0.697265625 0.73046875 0.763671875 0.28125 0.31640625 0.3515625 0.38671875 0.421875 0.45703125 0.4921875 0.52734375 0.5625 0.59765625 0.6328125 0.66796875 0.703125 0.73828125 0.7734375 0.80859375 0.296875 0.333984375 0.37109375 0.408203125 0.4453125 0.482421875 0.51953125 0.556640625 0.59375 0.630859375 0.66796875 0.705078125 0.7421875 0.779296875 0.81640625 1.0],stride=4)
//TEST_INPUT:ubuffer(data=[0.0 0.0 1.0 0.0 0.0 1.0 1.0 1.0 0.5 -0.125 0.5 1.125 0.125 0.375 0.375 0.125 0.625 0.625 0.875 0.875 0.9990234375 1.0 0.984375 1.0],stride=8):name=queries
StructuredBuffer<float2> queries;
// Expected energy uses rational interpolation of the asymmetric grid at squared cosine knots.
// Compensation is independently 1+F*E/max(1/100,1-E), for F=(1/4,1/2,3/4).
// Cases 0-3 are corners, 4-5 cosine clamps, 6-9 interiors, and 10-11 either side of the
// compensation denominator floor. The fixed 2e-5 budget includes source Float32 rounding.
//TEST_INPUT:ubuffer(data=[0.0625 1.0166666666666666 1.0333333333333334 1.05 0.296875 1.1055555555555556 1.211111111111111 1.3166666666666667 0.1796875 1.0547619047619048 1.1095238095238096 1.1642857142857144 1.0 26.0 51.0 76.0 0.1796875 1.0547619047619048 1.1095238095238096 1.1642857142857144 0.5166015625 1.2671717171717172 1.5343434343434343 1.8015151515151515 0.19710661235608554 1.0613738434608653 1.1227476869217303 1.1841215303825956 0.249725341796875 1.0832113077079519 1.1664226154159041 1.249633923123856 0.51862766431725549 1.2693484990063204 1.5386969980126408 1.8080454970189612 0.73684902848868539 1.7000249935016896 2.4000499870033791 3.1000749805050685 0.99719619750976562 25.929904937744141 50.859809875488281 75.789714813232422 0.95513916015625 6.3227891156462581 11.645578231292516 16.968367346938777],stride=16):name=expected
StructuredBuffer<float4> expected;
//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=results
RWStructuredBuffer<uint> results;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint completed = 0;
    for (uint index = 0; index < 12; ++index)
    {
        float2 query = queries[index];
        float energy = mtlx::tabulated_energy_emiss_eval_mini_microfacet(query.x, query.y);
        float3 scale = mtlx::MxTurquinLutMicrofacetCompensation::reflection_scale(
            query.x, query.y, float3(0.25, 0.5, 0.75));
        float4 actual = float4(energy, scale);
        float4 wanted = expected[index];
        uint errors = 0;
        for (uint channel = 0; channel < 4; ++channel)
        {
            if (!isfinite(actual[channel]) || abs(actual[channel] - wanted[channel]) > 0.00002)
                errors |= 1u << channel;
        }
        results[index] = errors;
        ++completed;
    }
    results[12] = completed;
    // The queried index comes from the runtime input buffer. Observing the knot bits separately
    // distinguishes its 1e-6 floor, whose effect on interpolated energy is below our float budget.
    int knotIndex = int(queries[0].x);
    results[13] = asuint(mtlx::mini_microfacet_get_cosine(knotIndex));
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 12{{$}}
// CHECK-NEXT: 897988541{{$}}
// CHECK-NEXT: 3735928559{{$}}
