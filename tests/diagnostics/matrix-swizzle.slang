//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

int doSomething(int a)
{
    int2x3 m1 = int2x3(0, 1, 2, 3, 4, 5);
    int3x2 m2 = int3x2(0, 1, 2, 3, 4, 5);

    int c = m1._14; // Out of bounds
//diag:        ^ member not found
//diag:        ^ '_14' is not a member of 'matrix<int,2,3>'.
    c = m1._32;
//diag:    ^ member not found
//diag:    ^ '_32' is not a member of 'matrix<int,2,3>'.
    c = m2._m22;
//diag:    ^ member not found
//diag:    ^ '_m22' is not a member of 'matrix<int,3,2>'.
    c = m2._;       // unfinished
//diag:    ^ member not found
//diag:    ^ '_' is not a member of 'matrix<int,3,2>'.
    c = m2._m;
//diag:    ^ member not found
//diag:    ^ '_m' is not a member of 'matrix<int,3,2>'.
    c = m2._1;
//diag:    ^ member not found
//diag:    ^ '_1' is not a member of 'matrix<int,3,2>'.
    c = m2._m1;
//diag:    ^ member not found
//diag:    ^ '_m1' is not a member of 'matrix<int,3,2>'.
    c = m2._m12_;
//diag:    ^ member not found
//diag:    ^ '_m12_' is not a member of 'matrix<int,3,2>'.
    int2 c2 = m1._m11_11;   // Mixing of 1 and 0-indexing
//diag:          ^ member not found
//diag:          ^ '_m11_11' is not a member of 'matrix<int,2,3>'.
    c = m1._11_11_11_11_11; // More than 4 elements
//diag:    ^ member not found
//diag:    ^ '_11_11_11_11_11' is not a member of 'matrix<int,2,3>'.
    c = m1.x;       // Invalid character
//diag:    ^ member not found
//diag:    ^ 'x' is not a member of 'matrix<int,2,3>'.
    c = m1._x;
//diag:    ^ member not found
//diag:    ^ '_x' is not a member of 'matrix<int,2,3>'.
    c = m1.x123;
//diag:    ^ member not found
//diag:    ^ 'x123' is not a member of 'matrix<int,2,3>'.

    return m1._11;
}

