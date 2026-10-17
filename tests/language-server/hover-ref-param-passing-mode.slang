// A reference parameter's signature prints its effective passing mode, so the legacy alias
// `const __ref` reads the same as `__ref_readonly`, and the legacy alias `__constref` reads the
// same as `__borrow`.

//TEST:LANG_SERVER(filecheck=CHECK):
int fRef(__ref int x) { return x; }
int fReadOnly(__ref_readonly int x) { return x; }
int fWriteOnly(__ref_writeonly int x) { x = 1; return 0; }
int fConstRef(const __ref int x) { return x; }
int fBorrow(__borrow int x) { return x; }
int fLegacyConstRef(__constref int x) { return x; }

void use()
{
    int a = 0;
//HOVER:17,6
    fRef(a);
//HOVER:19,6
    fReadOnly(a);
//HOVER:21,6
    fWriteOnly(a);
//HOVER:23,6
    fConstRef(a);
//HOVER:25,6
    fBorrow(a);
//HOVER:27,6
    fLegacyConstRef(a);
}

//CHECK: func fRef(ref int x) -> int
//CHECK: func fReadOnly(ref_readonly int x) -> int
//CHECK: func fWriteOnly(ref_writeonly int x) -> int
//CHECK: func fConstRef(ref_readonly int x) -> int
//CHECK: func fBorrow(borrow int x) -> int
//CHECK: func fLegacyConstRef(borrow int x) -> int
