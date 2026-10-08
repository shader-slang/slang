//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main() {
    float x;
    x = 1.0ff;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'ff' on floating-point literal

    x = 1.0hh;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'hh' on floating-point literal

    x = 1.0ll;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'll' on floating-point literal

    x = 1.0fff;
//CHECK:^^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^^ invalid suffix 'fff' on floating-point literal

    x = 1.0hl;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'hl' on floating-point literal

    x = 1.0lh;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'lh' on floating-point literal

    x = 1.0fhf;
//CHECK:^^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^^ invalid suffix 'fhf' on floating-point literal

    x = 1.0d;
//CHECK:^^^^ invalid suffix on floating-point literal
//CHECK:^^^^ invalid suffix 'd' on floating-point literal

    x = 1.0z;
//CHECK:^^^^ invalid suffix on floating-point literal
//CHECK:^^^^ invalid suffix 'z' on floating-point literal

    x = 0x1p0g;
//CHECK:^^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^^ invalid suffix 'g' on floating-point literal

    x = 1.0hF;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'hF' on floating-point literal

    x = 1.0Lf;
//CHECK:^^^^^ invalid suffix on floating-point literal
//CHECK:^^^^^ invalid suffix 'Lf' on floating-point literal

}
