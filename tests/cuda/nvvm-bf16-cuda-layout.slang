//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -matrix-layout-row-major -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -matrix-layout-row-major -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -matrix-layout-row-major -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Explicit CUDA queries describe the prelude ABI; unqualified sizeof/alignof uses Natural layout.
// BF4 is a component struct aligned to two bytes. Keep native BF2, Half padding and ordinary
// integer/float vectors as neighboring controls. Matrices use the explicit row-major option.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

typedef BFloat16 V_bf1;
struct W_bf1 { uint16_t prefix; V_bf1 value; uint16_t suffix; };
struct A_bf1 { W_bf1 values[3]; uint16_t tail; };
typedef vector<BFloat16,2> V_bf2;
struct W_bf2 { uint16_t prefix; V_bf2 value; uint16_t suffix; };
struct A_bf2 { W_bf2 values[3]; uint16_t tail; };
typedef vector<BFloat16,3> V_bf3;
struct W_bf3 { uint16_t prefix; V_bf3 value; uint16_t suffix; };
struct A_bf3 { W_bf3 values[3]; uint16_t tail; };
typedef vector<BFloat16,4> V_bf4;
struct W_bf4 { uint16_t prefix; V_bf4 value; uint16_t suffix; };
struct A_bf4 { W_bf4 values[3]; uint16_t tail; };
typedef half2 V_half2;
struct W_half2 { uint16_t prefix; V_half2 value; uint16_t suffix; };
struct A_half2 { W_half2 values[3]; uint16_t tail; };
typedef half3 V_half3;
struct W_half3 { uint16_t prefix; V_half3 value; uint16_t suffix; };
struct A_half3 { W_half3 values[3]; uint16_t tail; };
typedef half4 V_half4;
struct W_half4 { uint16_t prefix; V_half4 value; uint16_t suffix; };
struct A_half4 { W_half4 values[3]; uint16_t tail; };
typedef uint16_t2 V_ushort2;
struct W_ushort2 { uint16_t prefix; V_ushort2 value; uint16_t suffix; };
struct A_ushort2 { W_ushort2 values[3]; uint16_t tail; };
typedef uint16_t3 V_ushort3;
struct W_ushort3 { uint16_t prefix; V_ushort3 value; uint16_t suffix; };
struct A_ushort3 { W_ushort3 values[3]; uint16_t tail; };
typedef uint16_t4 V_ushort4;
struct W_ushort4 { uint16_t prefix; V_ushort4 value; uint16_t suffix; };
struct A_ushort4 { W_ushort4 values[3]; uint16_t tail; };
typedef float3 V_float3;
struct W_float3 { uint16_t prefix; V_float3 value; uint16_t suffix; };
struct A_float3 { W_float3 values[3]; uint16_t tail; };
typedef float4 V_float4;
struct W_float4 { uint16_t prefix; V_float4 value; uint16_t suffix; };
struct A_float4 { W_float4 values[3]; uint16_t tail; };
typedef matrix<BFloat16,2,4> V_bfm24;
struct W_bfm24 { uint16_t prefix; V_bfm24 value; uint16_t suffix; };
struct A_bfm24 { W_bfm24 values[3]; uint16_t tail; };
typedef matrix<BFloat16,4,2> V_bfm42;
struct W_bfm42 { uint16_t prefix; V_bfm42 value; uint16_t suffix; };
struct A_bfm42 { W_bfm42 values[3]; uint16_t tail; };
typedef matrix<BFloat16,3,4> V_bfm34;
struct W_bfm34 { uint16_t prefix; V_bfm34 value; uint16_t suffix; };
struct A_bfm34 { W_bfm34 values[3]; uint16_t tail; };
typedef matrix<BFloat16,4,3> V_bfm43;
struct W_bfm43 { uint16_t prefix; V_bfm43 value; uint16_t suffix; };
struct A_bfm43 { W_bfm43 values[3]; uint16_t tail; };

[numthreads(1, 1, 1)]
void computeMain()
{
    // bf1: value, wrapped record, and three-record array holder.
    outputBuffer[0] = uint(__sizeOf<V_bf1>());
    outputBuffer[1] = uint(__alignOf<V_bf1>());
    outputBuffer[2] = uint(__sizeOf<W_bf1>());
    outputBuffer[3] = uint(__alignOf<W_bf1>());
    outputBuffer[4] = uint(__sizeOf<A_bf1>());
    outputBuffer[5] = uint(__alignOf<A_bf1>());
    // bf2: value, wrapped record, and three-record array holder.
    outputBuffer[6] = uint(__sizeOf<V_bf2>());
    outputBuffer[7] = uint(__alignOf<V_bf2>());
    outputBuffer[8] = uint(__sizeOf<W_bf2>());
    outputBuffer[9] = uint(__alignOf<W_bf2>());
    outputBuffer[10] = uint(__sizeOf<A_bf2>());
    outputBuffer[11] = uint(__alignOf<A_bf2>());
    // bf3: value, wrapped record, and three-record array holder.
    outputBuffer[12] = uint(__sizeOf<V_bf3>());
    outputBuffer[13] = uint(__alignOf<V_bf3>());
    outputBuffer[14] = uint(__sizeOf<W_bf3>());
    outputBuffer[15] = uint(__alignOf<W_bf3>());
    outputBuffer[16] = uint(__sizeOf<A_bf3>());
    outputBuffer[17] = uint(__alignOf<A_bf3>());
    // bf4: value, wrapped record, and three-record array holder.
    outputBuffer[18] = uint(__sizeOf<V_bf4>());
    outputBuffer[19] = uint(__alignOf<V_bf4>());
    outputBuffer[20] = uint(__sizeOf<W_bf4>());
    outputBuffer[21] = uint(__alignOf<W_bf4>());
    outputBuffer[22] = uint(__sizeOf<A_bf4>());
    outputBuffer[23] = uint(__alignOf<A_bf4>());
    // half2: value, wrapped record, and three-record array holder.
    outputBuffer[24] = uint(__sizeOf<V_half2>());
    outputBuffer[25] = uint(__alignOf<V_half2>());
    outputBuffer[26] = uint(__sizeOf<W_half2>());
    outputBuffer[27] = uint(__alignOf<W_half2>());
    outputBuffer[28] = uint(__sizeOf<A_half2>());
    outputBuffer[29] = uint(__alignOf<A_half2>());
    // half3: value, wrapped record, and three-record array holder.
    outputBuffer[30] = uint(__sizeOf<V_half3>());
    outputBuffer[31] = uint(__alignOf<V_half3>());
    outputBuffer[32] = uint(__sizeOf<W_half3>());
    outputBuffer[33] = uint(__alignOf<W_half3>());
    outputBuffer[34] = uint(__sizeOf<A_half3>());
    outputBuffer[35] = uint(__alignOf<A_half3>());
    // half4: value, wrapped record, and three-record array holder.
    outputBuffer[36] = uint(__sizeOf<V_half4>());
    outputBuffer[37] = uint(__alignOf<V_half4>());
    outputBuffer[38] = uint(__sizeOf<W_half4>());
    outputBuffer[39] = uint(__alignOf<W_half4>());
    outputBuffer[40] = uint(__sizeOf<A_half4>());
    outputBuffer[41] = uint(__alignOf<A_half4>());
    // ushort2: value, wrapped record, and three-record array holder.
    outputBuffer[42] = uint(__sizeOf<V_ushort2>());
    outputBuffer[43] = uint(__alignOf<V_ushort2>());
    outputBuffer[44] = uint(__sizeOf<W_ushort2>());
    outputBuffer[45] = uint(__alignOf<W_ushort2>());
    outputBuffer[46] = uint(__sizeOf<A_ushort2>());
    outputBuffer[47] = uint(__alignOf<A_ushort2>());
    // ushort3: value, wrapped record, and three-record array holder.
    outputBuffer[48] = uint(__sizeOf<V_ushort3>());
    outputBuffer[49] = uint(__alignOf<V_ushort3>());
    outputBuffer[50] = uint(__sizeOf<W_ushort3>());
    outputBuffer[51] = uint(__alignOf<W_ushort3>());
    outputBuffer[52] = uint(__sizeOf<A_ushort3>());
    outputBuffer[53] = uint(__alignOf<A_ushort3>());
    // ushort4: value, wrapped record, and three-record array holder.
    outputBuffer[54] = uint(__sizeOf<V_ushort4>());
    outputBuffer[55] = uint(__alignOf<V_ushort4>());
    outputBuffer[56] = uint(__sizeOf<W_ushort4>());
    outputBuffer[57] = uint(__alignOf<W_ushort4>());
    outputBuffer[58] = uint(__sizeOf<A_ushort4>());
    outputBuffer[59] = uint(__alignOf<A_ushort4>());
    // float3: value, wrapped record, and three-record array holder.
    outputBuffer[60] = uint(__sizeOf<V_float3>());
    outputBuffer[61] = uint(__alignOf<V_float3>());
    outputBuffer[62] = uint(__sizeOf<W_float3>());
    outputBuffer[63] = uint(__alignOf<W_float3>());
    outputBuffer[64] = uint(__sizeOf<A_float3>());
    outputBuffer[65] = uint(__alignOf<A_float3>());
    // float4: value, wrapped record, and three-record array holder.
    outputBuffer[66] = uint(__sizeOf<V_float4>());
    outputBuffer[67] = uint(__alignOf<V_float4>());
    outputBuffer[68] = uint(__sizeOf<W_float4>());
    outputBuffer[69] = uint(__alignOf<W_float4>());
    outputBuffer[70] = uint(__sizeOf<A_float4>());
    outputBuffer[71] = uint(__alignOf<A_float4>());
    // bfm24: value, wrapped record, and three-record array holder.
    outputBuffer[72] = uint(__sizeOf<V_bfm24>());
    outputBuffer[73] = uint(__alignOf<V_bfm24>());
    outputBuffer[74] = uint(__sizeOf<W_bfm24>());
    outputBuffer[75] = uint(__alignOf<W_bfm24>());
    outputBuffer[76] = uint(__sizeOf<A_bfm24>());
    outputBuffer[77] = uint(__alignOf<A_bfm24>());
    // bfm42: value, wrapped record, and three-record array holder.
    outputBuffer[78] = uint(__sizeOf<V_bfm42>());
    outputBuffer[79] = uint(__alignOf<V_bfm42>());
    outputBuffer[80] = uint(__sizeOf<W_bfm42>());
    outputBuffer[81] = uint(__alignOf<W_bfm42>());
    outputBuffer[82] = uint(__sizeOf<A_bfm42>());
    outputBuffer[83] = uint(__alignOf<A_bfm42>());
    // bfm34: value, wrapped record, and three-record array holder.
    outputBuffer[84] = uint(__sizeOf<V_bfm34>());
    outputBuffer[85] = uint(__alignOf<V_bfm34>());
    outputBuffer[86] = uint(__sizeOf<W_bfm34>());
    outputBuffer[87] = uint(__alignOf<W_bfm34>());
    outputBuffer[88] = uint(__sizeOf<A_bfm34>());
    outputBuffer[89] = uint(__alignOf<A_bfm34>());
    // bfm43: value, wrapped record, and three-record array holder.
    outputBuffer[90] = uint(__sizeOf<V_bfm43>());
    outputBuffer[91] = uint(__alignOf<V_bfm43>());
    outputBuffer[92] = uint(__sizeOf<W_bfm43>());
    outputBuffer[93] = uint(__alignOf<W_bfm43>());
    outputBuffer[94] = uint(__sizeOf<A_bfm43>());
    outputBuffer[95] = uint(__alignOf<A_bfm43>());
}

//CHECK: type: uint32_t
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}6{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}20{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}12{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}40{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}6{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}10{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}32{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}12{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}38{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}12{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}40{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}52{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}52{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}12{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}40{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}6{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}10{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}32{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}24{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}80{{$}}
//CHECK-NEXT: {{^}}8{{$}}
//CHECK-NEXT: {{^}}12{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}20{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}64{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}48{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}160{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}20{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}62{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}16{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}24{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}76{{$}}
//CHECK-NEXT: {{^}}4{{$}}
//CHECK-NEXT: {{^}}24{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}28{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}86{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}24{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}28{{$}}
//CHECK-NEXT: {{^}}2{{$}}
//CHECK-NEXT: {{^}}86{{$}}
//CHECK-NEXT: {{^}}2{{$}}
