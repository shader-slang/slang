// RN16(exact(a*b+c)); finite expected bits are independently derived, not CUDA outputs.
// NaNs compare by class; signed zeros and infinities compare exact bits.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -fp-mode -Xslang precise

//TEST_INPUT: ubuffer(data=[15361 15872 32769 15873 15361 15361 48130 16 31743 16384 64511 31743 1 14336 0 0 3 14336 0 2 32769 14336 32768 32768 1023 15360 1 1024 31743 15360 19456 31744 0 15360 32768 0 32768 15360 32768 32768 15360 15360 48128 0 31744 15360 15360 31744 64512 15360 48128 64512 31744 0 15360 32256 31744 15360 64512 32256 32309 15360 0 32256 31745 15360 0 32256], stride=16):name=cases
StructuredBuffer<uint4> cases;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

half operand(uint lane, uint component)
{
    return bit_cast<half>(uint16_t(cases[lane][component]));
}

uint observedBits(half value)
{
    uint bits = uint(bit_cast<uint16_t>(value));
    return (bits & 0x7c00u) == 0x7c00u && (bits & 0x3ffu) != 0 ? 0x7e00u : bits;
}

[noinline]
half throughHelper(half a, half b, half c)
{
    return fma(a, b, c);
}

[numthreads(32,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 17) return;
    half a = operand(lane, 0), b = operand(lane, 1), c = operand(lane, 2);
    half scalar = fma(a, b, c);
    uint errors = observedBits(scalar) == cases[lane].w ? 0u : 1u;
    errors |= observedBits(throughHelper(a, b, c)) == cases[lane].w ? 0u : 2u;
    uint i1 = (lane + 1) % 17, i2 = (lane + 2) % 17, i3 = (lane + 3) % 17;
    half4 av = half4(a, operand(i1, 0), operand(i2, 0), operand(i3, 0));
    half4 bv = half4(b, operand(i1, 1), operand(i2, 1), operand(i3, 1));
    half4 cv = half4(c, operand(i1, 2), operand(i2, 2), operand(i3, 2));
    half4 vectorResult = fma(av, bv, cv);
    errors |= observedBits(vectorResult.x) == cases[lane].w ? 0u : 4u;
    errors |= observedBits(vectorResult.y) == cases[i1].w ? 0u : 8u;
    errors |= observedBits(vectorResult.z) == cases[i2].w ? 0u : 16u;
    errors |= observedBits(vectorResult.w) == cases[i3].w ? 0u : 32u;
    outputBuffer[1 + 3 * lane] = errors;
    outputBuffer[2 + 3 * lane] = observedBits(scalar);
    outputBuffer[3 + 3 * lane] = 100 + lane;
}

// CHECK: {{^}}324508639{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}15873{{$}}
// CHECK-NEXT: {{^}}100{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}16{{$}}
// CHECK-NEXT: {{^}}101{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}31743{{$}}
// CHECK-NEXT: {{^}}102{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}103{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}104{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}32768{{$}}
// CHECK-NEXT: {{^}}105{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1024{{$}}
// CHECK-NEXT: {{^}}106{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}31744{{$}}
// CHECK-NEXT: {{^}}107{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}108{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}32768{{$}}
// CHECK-NEXT: {{^}}109{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}110{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}31744{{$}}
// CHECK-NEXT: {{^}}111{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}64512{{$}}
// CHECK-NEXT: {{^}}112{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}32256{{$}}
// CHECK-NEXT: {{^}}113{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}32256{{$}}
// CHECK-NEXT: {{^}}114{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}32256{{$}}
// CHECK-NEXT: {{^}}115{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}32256{{$}}
// CHECK-NEXT: {{^}}116{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
