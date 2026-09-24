//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// The host supplies exact IEEE binary64 words independently of source literal parsing.
// Each lane compares runtime-loaded data to its source literal, so constant folding cannot
// turn the oracle into a comparison of two identically misprinted source constants.
//TEST_INPUT:ubuffer(data=[4194304 1055916032 4294967295 1054867455 1 1054867456 4294967295 1071644671 1 1071644672 4294967295 1072693247 1 1072693248 4294967295 1089470463 1 1089470464 1 1048576 1 0 4194304 3203399680 4294967295 3202351103 1 3202351104 4294967295 3219128319 1 3219128320 4294967295 3220176895 1 3220176896 4294967295 3236954111 1 3236954112 1 2148532224 1 2147483648], stride=8):name=inputBits
StructuredBuffer<uint2> inputBits;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 22) return;
    uint2 bits = inputBits[lane];
    double actual = asdouble(bits.x, bits.y);
    bool ok = false;
    switch (lane)
    {
    case 0: ok = actual == 0x1.0000000400000p-16l; break;
    case 1: ok = actual == 0x1.fffffffffffffp-18l; break;
    case 2: ok = actual == 0x1.0000000000001p-17l; break;
    case 3: ok = actual == 0x1.fffffffffffffp-2l; break;
    case 4: ok = actual == 0x1.0000000000001p-1l; break;
    case 5: ok = actual == 0x1.fffffffffffffp-1l; break;
    case 6: ok = actual == 0x1.0000000000001p+0l; break;
    case 7: ok = actual == 0x1.fffffffffffffp+15l; break;
    case 8: ok = actual == 0x1.0000000000001p+16l; break;
    case 9: ok = actual == 0x1.0000000000001p-1022l; break;
    case 10: ok = actual == 0x0.0000000000001p-1022l; break;
    case 11: ok = actual == -0x1.0000000400000p-16l; break;
    case 12: ok = actual == -0x1.fffffffffffffp-18l; break;
    case 13: ok = actual == -0x1.0000000000001p-17l; break;
    case 14: ok = actual == -0x1.fffffffffffffp-2l; break;
    case 15: ok = actual == -0x1.0000000000001p-1l; break;
    case 16: ok = actual == -0x1.fffffffffffffp-1l; break;
    case 17: ok = actual == -0x1.0000000000001p+0l; break;
    case 18: ok = actual == -0x1.fffffffffffffp+15l; break;
    case 19: ok = actual == -0x1.0000000000001p+16l; break;
    case 20: ok = actual == -0x1.0000000000001p-1022l; break;
    case 21: ok = actual == -0x0.0000000000001p-1022l; break;
    }
    outputBuffer[lane] = ok ? 8000 + lane : 0;
}

// CHECK: 8000
// CHECK: 8001
// CHECK: 8002
// CHECK: 8003
// CHECK: 8004
// CHECK: 8005
// CHECK: 8006
// CHECK: 8007
// CHECK: 8008
// CHECK: 8009
// CHECK: 8010
// CHECK: 8011
// CHECK: 8012
// CHECK: 8013
// CHECK: 8014
// CHECK: 8015
// CHECK: 8016
// CHECK: 8017
// CHECK: 8018
// CHECK: 8019
// CHECK: 8020
// CHECK: 8021
