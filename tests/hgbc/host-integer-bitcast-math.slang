//TEST:INTERPRET(hgvm,filecheck=OUT): -api default
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

// Host shifts, bitwise operations, numeric vector casts, bitcasts, and the math functions that are
// built from them. Inputs come from [noinline] parameters so the interpreter executes the
// bytecode instead of the compiler folding constants.

void check(bool condition, String message)
{
    if (!condition)
        hgbc_asm { OpFail $message; };
}

[noinline]
void checkShifts(int negative, uint value, uint amount)
{
    // Amounts are masked to the bit width, as HLSL defines: 33 shifts by one, 32 by zero.
    check((value << amount) == 2u, "lsh by 33");
    check((value << (amount - 1u)) == 1u, "lsh by 32");
    check((0x80000000u >> amount) == 0x40000000u, "logical rsh by 33");
    check((negative >> int(amount)) == -4, "arithmetic rsh by 33");
    check((negative >> 1u) == -4, "arithmetic rsh with unsigned amount");
    check((0xfffffff8u >> 1) == 0x7ffffffcu, "logical rsh with signed amount");

    uint3 lanes = uint3(value, value << 4, 0xf0000000u);
    uint3 shifted = lanes >> 2u;
    check(shifted.x == 0u && shifted.y == 4u && shifted.z == 0x3c000000u, "vector rsh by scalar");
    uint3 masked = lanes << uint3(32u, 33u, 1u);
    check(masked.x == 1u && masked.y == 32u && masked.z == 0xe0000000u, "vector lsh masks lanes");
}

[noinline]
void checkBitwise(uint a, uint b, int c)
{
    check((a & b) == 0x0f00u, "and");
    check((a | b) == 0xfff0u, "or");
    check((a ^ b) == 0xf0f0u, "xor");
    check(~a == 0xffff00ffu, "not");
    check((~c) == -8, "signed not");
    check(-a == 0xffff0100u, "unsigned negate wraps");

    uint2 v = uint2(a, b) & 0x0ff0u;
    check(v.x == 0x0f00u && v.y == 0x0ff0u, "vector and with scalar");
    int2 w = int2(c, -c) | int2(8, 8);
    check(w.x == 15 && w.y == -7, "signed vector or");
}

[noinline]
void checkCasts(uint3 u, float3 f, bool3 b, float nan)
{
    float3 fromUint = float3(u);
    check(fromUint.x == 1.0 && fromUint.y == 2.0 && fromUint.z == 4294967296.0, "uint3 to float3");

    int3 fromFloat = int3(f);
    check(fromFloat.x == -1 && fromFloat.y == 2147483647 && fromFloat.z == 2, "float3 to int3");
    uint3 unsignedFromFloat = uint3(f);
    check(unsignedFromFloat.x == 0u && unsignedFromFloat.z == 2u, "float3 to uint3 saturates");
    check(int(nan) == 0 && uint(nan) == 0u, "NaN converts to zero");

    half3 narrow = half3(f);
    check(narrow.x == half(-1.5) && isinf(narrow.y) && narrow.z == half(2.75), "float3 to half3");

    int3 signedLanes = int3(-1, 2, 3);
    uint3 reinterpreted = uint3(signedLanes);
    check(reinterpreted.x == 0xffffffffu && reinterpreted.y == 2u, "int3 to uint3");

    float3 fromBool = float3(b);
    check(fromBool.x == 1.0 && fromBool.y == 0.0 && fromBool.z == 1.0, "bool3 to float3");

    float2x2 m = float2x2(f.x, f.z, -f.z, 0.5);
    int2x2 im = int2x2(m);
    check(im[0][0] == -1 && im[0][1] == 2 && im[1][0] == -2 && im[1][1] == 0, "float2x2 to int2x2");
}

[noinline]
void checkBitCasts(float negativeZero, uint one, float nan)
{
    check(asuint(negativeZero) == 0x80000000u, "asuint of -0.0");
    check(asfloat(one) == 1.0, "asfloat of 0x3f800000");
    check(asint(-2.0) == int(0xc0000000u), "asint of -2.0");
    check(asuint(asfloat(0x7fa00001u + one - one)) == 0x7fa00001u, "NaN payload is preserved");
    check((asuint(nan) & 0x7fffffffu) > 0x7f800000u, "asuint of NaN");

    uint3 bits = asuint(float3(negativeZero, 1.0, -2.0));
    check(bits.x == 0x80000000u && bits.y == 0x3f800000u && bits.z == 0xc0000000u, "vector asuint");
    float2 back = asfloat(int2(0x3f800000, int(0xc0000000u)));
    check(back.x == 1.0 && back.y == -2.0, "vector asfloat");
}

[noinline]
void checkMath(float x, float negativeZero, float inf, float nan, int intMin, half h)
{
    check(asuint(abs(negativeZero)) == 0u, "abs(-0.0) is +0");
    check(abs(-x) == x && abs(-3) == 3 && abs(intMin) == intMin, "scalar abs");
    check(abs(7u) == 7u, "unsigned abs");
    float3 absolute = abs(float3(-x, x, negativeZero));
    check(absolute.x == x && absolute.y == x && asuint(absolute.z) == 0u, "vector abs");
    check(all(abs(int2(-4, 5)) == int2(4, 5)), "integer vector abs");

    check(clamp(x, 0.0, 1.0) == 1.0 && clamp(-x, -1.0, 1.0) == -1.0, "clamp");
    check(clamp(5, 0, 3) == 3 && all(clamp(float2(x, -x), -1.0, 1.0) == float2(1.0, -1.0)), "clamp");
    check(clamp(nan, 0.0, 1.0) == 0.0, "clamp of NaN returns the lower bound");
    check(saturate(x) == 1.0 && saturate(-x) == 0.0, "saturate");

    float3 c = cross(float3(1.0, 0.0, 0.0), float3(0.0, x - 1.0, 0.0));
    check(c.x == 0.0 && c.y == 0.0 && c.z == 1.0, "cross");
    check(length(float3(x, 0.0, 0.0)) == x && length(float2(3.0, 4.0)) == 5.0, "length");
    check(distance(float2(1.0, 1.0), float2(4.0, 5.0)) == 5.0, "distance");
    float3 n = normalize(float3(0.0, 0.0, x));
    check(n.x == 0.0 && n.z == 1.0, "normalize");
    check(isnan(normalize(float3(0.0, 0.0, 0.0)).x), "normalize of zero is NaN");
    check(rcp(x) == 0.5 && rsqrt(4.0 * x * x) == 0.25, "rcp and rsqrt");
    check(all(rcp(float2(x, 4.0)) == float2(0.5, 0.25)), "vector rcp");
    check(frac(-1.25) == 0.75 && all(frac(float2(2.5, -x)) == float2(0.5, 0.0)), "frac");

    check(all(bool3(true, x > 1.0, true)) && !all(float2(x, 0.0)), "all");
    check(all(int3(1, 2, 3)) && !all(uint2(0u, 1u)), "integer all");

    check(isnan(nan) && !isnan(inf) && !isnan(x), "isnan");
    check(isinf(inf) && isinf(-inf) && !isinf(nan) && !isinf(x), "isinf");
    check(isfinite(x) && !isfinite(inf) && !isfinite(nan), "isfinite");
    bool3 nanLanes = isnan(float3(nan, x, inf));
    bool3 infLanes = isinf(float3(nan, x, -inf));
    bool3 finiteLanes = isfinite(float3(nan, x, inf));
    check(nanLanes.x && !nanLanes.y && !nanLanes.z, "vector isnan");
    check(!infLanes.x && !infLanes.y && infLanes.z, "vector isinf");
    check(!finiteLanes.x && finiteLanes.y && !finiteLanes.z, "vector isfinite");
    check(isinf(h * half(1000.0)) && isfinite(h) && !isnan(h), "half classification");
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    float floatZero = float(zero);
    float inf = 1.0 / floatZero;
    float nan = floatZero / floatZero;

    checkShifts(-8, zero + 1u, zero + 33u);
    checkBitwise(zero + 0xff00u, zero + 0x0ff0u, int(zero) + 7);
    checkCasts(
        uint3(zero + 1u, zero + 2u, 0xffffffffu + zero),
        float3(floatZero - 1.5, floatZero + 3e9, floatZero + 2.75),
        bool3(true, zero != 0u, true),
        nan);
    checkBitCasts(-floatZero, zero + 0x3f800000u, nan);
    checkMath(floatZero + 2.0, -floatZero, inf, nan, int(0x80000000u + zero), half(floatZero + 100.0));
    print("done");
}

// CHECK-DAG: OpRsh
// CHECK-DAG: OpBitAnd
// CHECK-DAG: OpBitOr
// CHECK-DAG: OpBitXor
// CHECK-DAG: OpBitNot
// CHECK-DAG: OpAbs
// CHECK-DAG: OpBitCast
// CHECK-DAG: OpScalarTypeCast
// OUT: done
