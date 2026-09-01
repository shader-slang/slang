//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main() {
    float x;
    x = 0x1p;
//CHECK:^^^^ invalid floating-point number
//CHECK:^^^^ invalid floating-point number '0x1p' on floating-point literal

    x = 0x1p+;
//CHECK:^^^^^ invalid floating-point number
//CHECK:^^^^^ invalid floating-point number '0x1p+' on floating-point literal

    x = 0x123p-ab;
//CHECK:^^^^^^^^^ invalid floating-point number
//CHECK:^^^^^^^^^ invalid floating-point number '0x123p-' on floating-point literal

    x = 0.123e-abc;
//CHECK:^^^^^^^^^^ invalid floating-point number
//CHECK:^^^^^^^^^^ invalid floating-point number '0.123e-' on floating-point literal
}
