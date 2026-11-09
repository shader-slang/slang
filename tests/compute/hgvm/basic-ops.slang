//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[host]
void main()
{
    // Keep the inputs opaque to the ordinary constant-folding passes so this test reaches HGVM
    // with executable arithmetic and control-flow records instead of only folded literals.
    uint left = hgbc_asm { result : $$uint = OpIntLit 7; };
    uint right = hgbc_asm { result : $$uint = OpIntLit 5; };
    uint eight = hgbc_asm { result : $$uint = OpIntLit 8; };
    uint two = hgbc_asm { result : $$uint = OpIntLit 2; };
    float nine = hgbc_asm { result : $$float = OpFloatLit 1091567616; };
    float fourPointFive = hgbc_asm { result : $$float = OpFloatLit 1083179008; };
    float zero = hgbc_asm { result : $$float = OpFloatLit 0; };

    uint sum = left + right;
    uint difference = left - right;
    uint product = left * right;
    uint integerQuotient = eight / two;
    uint unsignedRemainder = eight % right;
    int signedRemainder = -int(eight) % int(right);
    uint2 vectorRemainder = uint2(eight, left) % uint2(right, two);
    uint shifted = left << two;
    float floatQuotient = nine / fourPointFive;
    float negative = -floatQuotient;
    float power = hgbc_asm {
        result : $$float = OpPow $$float $floatQuotient $floatQuotient;
    };
    float sine = hgbc_asm { result : $$float = OpSin $zero; };
    float cosine = hgbc_asm { result : $$float = OpCos $zero; };
    float tangent = hgbc_asm { result : $$float = OpTan $zero; };
    float arcSine = asin(zero);
    float arcTangent = atan2(nine, zero);
    float squareRoot = hgbc_asm { result : $$float = OpSqrt $nine; };
    float ceiling = ceil(fourPointFive);
    float floorValue = floor(fourPointFive);
    float exponent = exp(zero);
    float baseTwoExponent = exp2(floatQuotient);
    float logarithm = log(exponent);
    float baseTwoLogarithm = log2(eight);
    half halfBaseTwoLogarithm = log2(half(eight));
    bool anyFalse = any(float3(zero, zero, zero));
    bool anyTrue = any(float3(zero, nine, zero));
    bool condition = sum > product;
    bool inverseCondition = !condition;

    print(sum);
    print(difference);
    print(product);
    print(integerQuotient);
    print(unsignedRemainder);
    print(signedRemainder == -3);
    print(vectorRemainder.x);
    print(vectorRemainder.y);
    print(shifted);
    print(floatQuotient);
    print(negative);
    print(power);
    print(sine);
    print(cosine);
    print(tangent);
    print(arcSine);
    print(arcTangent);
    print(squareRoot);
    print(ceiling);
    print(floorValue);
    print(exponent);
    print(baseTwoExponent);
    print(logarithm);
    print(baseTwoLogarithm);
    print(float(halfBaseTwoLogarithm));
    print(anyFalse);
    print(anyTrue);
    print(inverseCondition);
    print(left == right);
    print(left != right);
    print(left < right);
    print(left <= right);
    print(left >= right);

    if (condition)
    {
        print("unexpected branch");
    }
    else
    {
        print("selected false branch");
    }
    if (inverseCondition)
    {
        print("selected true branch");
    }
    else
    {
        print("unexpected second branch");
    }
    print(sum);
}

// CHECK: 12
// CHECK-NEXT: 2
// CHECK-NEXT: 35
// CHECK-NEXT: 4
// CHECK-NEXT: 3
// CHECK-NEXT: true
// CHECK-NEXT: 3
// CHECK-NEXT: 1
// CHECK-NEXT: 28
// CHECK-NEXT: 2.000000
// CHECK-NEXT: -2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.570796
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: selected false branch
// CHECK-NEXT: selected true branch
// CHECK-NEXT: 12
