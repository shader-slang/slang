//TEST:LANG_SERVER(filecheck=CHECK):

// A parameter-name inlay hint spells the parameter's effective passing mode, so each reference
// access, including the legacy `const __ref` spelling of a read-only reference, gets its own label.

void takeRef(__ref int refArg) {}
void takeReadOnly(__ref_readonly int readOnlyArg) {}
void takeConstRef(const __ref int constRefArg) {}
void takeWriteOnly(__ref_writeonly int writeOnlyArg) {}
void takeOut(out int outArg) { outArg = 0; }
void takeIn(int inArg) {}

void useInlayHints()
{
    int a = 0;
    takeRef(a);
    takeReadOnly(a);
    takeConstRef(a);
    takeWriteOnly(a);
    takeOut(a);
    takeIn(a);
}

//CONFIG:slang.inlayHints.parameterNames=true
//INLAY

// CHECK: inlay hints:
// CHECK-NEXT: ref refArg:
// CHECK-NEXT: ref readonly readOnlyArg:
// CHECK-NEXT: ref readonly constRefArg:
// CHECK-NEXT: ref writeonly writeOnlyArg:
// CHECK-NEXT: out outArg:
// CHECK-NEXT: inArg:
