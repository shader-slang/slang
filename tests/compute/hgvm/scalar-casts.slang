//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[host]
void main()
{
    // Inline literals keep the inputs opaque to constant folding while the ordinary Slang casts
    // below exercise compiler lowering to the common HGBC OpScalarTypeCast representation.
    uint seven = hgbc_asm { result : $$uint = OpIntLit 7; };
    uint twoHundredFiftySix = hgbc_asm { result : $$uint = OpIntLit 256; };
    uint allBits = hgbc_asm { result : $$uint = OpIntLit 4294967295; };
    int minusThree = hgbc_asm { result : $$int = OpIntLit 4294967293; };
    int minusOne = hgbc_asm { result : $$int = OpIntLit 4294967295; };
    float threePointSevenFive = hgbc_asm { result : $$float = OpFloatLit 1081081856; };
    bool falseValue = hgbc_asm { result : $$bool = OpBoolLit false; };

    float sevenFloat = hgbc_asm {
        result : $$float = OpScalarTypeCast $$uint $seven;
    };
    print(sevenFloat);
    print(float(minusThree));
    print(int(threePointSevenFive));
    print(uint(threePointSevenFive));
    print(bool(twoHundredFiftySix));
    print(int(falseValue));
    print(uint(minusThree));
    print(int(allBits) == minusOne);
}

// CHECK: 7.000000
// CHECK-NEXT: -3.000000
// CHECK-NEXT: 3
// CHECK-NEXT: 3
// CHECK-NEXT: true
// CHECK-NEXT: 0
// CHECK-NEXT: 4294967293
// CHECK-NEXT: true
