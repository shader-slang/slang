// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g0

[host]
void main()
{
    print("debug-disabled");
}

// CHECK-NOT: OpDebug
// CHECK: OpPrint
// CHECK-NOT: OpDebug
