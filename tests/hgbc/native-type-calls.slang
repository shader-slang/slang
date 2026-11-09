//TEST:SIMPLE(filecheck=CODE): -target hgbc -entry main -validate-ir
//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

// HGBC supports these types directly. Neither a String argument/result nor a reference
// parameter should force an otherwise out-of-line helper into its caller.
[noinline]
String echo(String value)
{
    return value;
}

[noinline]
void increment(__ref uint value)
{
    ++value;
}

[host]
void main()
{
    uint value = hgbc_asm { result : $$uint = OpIntLit 7; };
    print(echo("native string call"));
    increment(value);
    print(value);
}

// CODE: Section("TYPE")
// CODE-DAG: [[STRING:%[A-Za-z0-9_]+]] = OpStringType
// CODE-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CODE-DAG: [[REF:%[A-Za-z0-9_]+]] = OpRefType [[UINT]]
// CODE: Section("FUNC")
// CODE: [[CALL:%[A-Za-z0-9_]+]] : [[STRING]] = OpCall [[ECHO:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}}
// CODE: OpPrint [[STRING]] [[CALL]]
// CODE: OpCall [[INCREMENT:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}}
// CODE: OpPrint [[UINT]]

// CHECK: standard output = {
// CHECK-NEXT: native string call
// CHECK-NEXT: 8
