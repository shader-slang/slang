// The short-circuit reconstruction pass is excluded from CPU-via-LLVM
// targets: LLVM IR `and`/`or` do not short-circuit, so rewriting the
// short-circuit diamond there would hoist the guarded read into
// unconditional evaluation. The diamond must survive as control flow —
// branch structure, not a bitwise `and`/`or` of the operands.

//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program

export __extern_cpp int main(int argc, Ptr<int> data)
{
    if (argc > 0 && data[argc] > 0)
    {
        return 1;
    }
    return 0;
    //CHECK-NOT: and i1
    //CHECK-NOT: or i1
    //CHECK: br i1
}
