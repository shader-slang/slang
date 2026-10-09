// Digit separators do not change where an integer literal ends: an integer followed by
// a `.x` swizzle, or a `0_`-prefixed decimal, keep their meaning.

//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all
void main()
{
    printf("%d\n", 1_0.x);
    // CHECK: 10
    printf("%d\n", 1_.x);
    // CHECK-NEXT: 1
    printf("%d\n", 0_7);
    // CHECK-NEXT: 7
    printf("%d\n", 0x_FF);
    // CHECK-NEXT: 255
    printf("%d\n", 0x1e_5);
    // CHECK-NEXT: 485
    printf("%d\n", 1__0);
    // CHECK-NEXT: 10
}
