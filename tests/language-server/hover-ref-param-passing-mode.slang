// A reference parameter's signature prints its effective passing mode, so the legacy spelling
// `const __ref` reads the same as `__ref_readonly`.

//TEST:LANG_SERVER(filecheck=CHECK):
int fRef(__ref int x) { return x; }
int fReadOnly(__ref_readonly int x) { return x; }
int fWriteOnly(__ref_writeonly int x) { x = 1; return 0; }
int fConstRef(const __ref int x) { return x; }

void use()
{
    int a = 0;
//HOVER:14,6
    fRef(a);
//HOVER:16,6
    fReadOnly(a);
//HOVER:18,6
    fWriteOnly(a);
//HOVER:20,6
    fConstRef(a);
}

//CHECK: func fRef(ref int x) -> int
//CHECK: func fReadOnly(ref readonly int x) -> int
//CHECK: func fWriteOnly(ref writeonly int x) -> int
//CHECK: func fConstRef(ref readonly int x) -> int
