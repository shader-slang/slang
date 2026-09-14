//TEST:INTERPRET(filecheck=CHECK):

// An array whose element type has no HostVM layout (`String[N]`) is itself unrepresentable,
// so a runtime value of it must be diagnosed rather than allocated a 0-byte slot (issue
// #13017). This mirrors interpreter-runtime-string-struct.slang but pins the array type's
// `~type:IRInst` rendering, which nothing else covers.

bool cmpF(float a, float b)
{
    return a < b;
}

void main()
{
    String a[2] = {"x", "y"};
    String b[2] = {"p", "q"};
    String r[2] = cmpF(1.0, 2.0) ? a : b;
    //CHECK: result code = 1
    //CHECK-DAG: error[E52014]
    //CHECK-DAG: type 'array<string,2>' is not representable by the Slang interpreter
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    printf("%s\n", r[0]);
}
