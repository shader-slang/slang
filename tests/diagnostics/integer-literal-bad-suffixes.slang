//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main() {
    int x;
    x = 0LUL;
//CHECK:^^^^ invalid suffix on integer literal
//CHECK:^^^^ invalid suffix 'LUL' on integer literal

    x = 0LLL;
//CHECK:^^^^ invalid suffix on integer literal
//CHECK:^^^^ invalid suffix 'LLL' on integer literal

    x = 0LLLLL;
//CHECK:^^^^^^ invalid suffix on integer literal
//CHECK:^^^^^^ invalid suffix 'LLLLL' on integer literal

    x = 0UU;
//CHECK:^^^ invalid suffix on integer literal
//CHECK:^^^ invalid suffix 'UU' on integer literal

    x = 0ULU;
//CHECK:^^^^ invalid suffix on integer literal
//CHECK:^^^^ invalid suffix 'ULU' on integer literal

    x = 0LZ;
//CHECK:^^^ invalid suffix on integer literal
//CHECK:^^^ invalid suffix 'LZ' on integer literal

    x = 0ZL;
//CHECK:^^^ invalid suffix on integer literal
//CHECK:^^^ invalid suffix 'ZL' on integer literal

    x = 0ZZ;
//CHECK:^^^ invalid suffix on integer literal
//CHECK:^^^ invalid suffix 'ZZ' on integer literal

    x = 0Ll;
//CHECK:^^^ invalid suffix on integer literal
//CHECK:^^^ invalid suffix 'Ll' on integer literal

    x = 0A;
//CHECK:^^ invalid suffix on integer literal
//CHECK:^^ invalid suffix 'A' on integer literal
}
