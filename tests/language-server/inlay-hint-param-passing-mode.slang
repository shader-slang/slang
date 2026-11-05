//TEST:LANG_SERVER(filecheck=CHECK):

// A parameter-name inlay hint spells the parameter's effective passing mode, so each reference
// access, including `const __ref`, the legacy alias of `__ref_readonly`, gets its own label. The
// legacy alias `__constref` is labeled the same as `__borrow`.

void takeRef(__ref int refArg) {}
void takeReadOnly(__ref_readonly int readOnlyArg) {}
void takeConstRef(const __ref int constRefArg) {}
void takeWriteOnly(__ref_writeonly int writeOnlyArg) {}
void takeOut(out int outArg) { outArg = 0; }
void takeIn(int inArg) {}
void takeBorrow(__borrow int borrowArg) {}
void takeLegacyConstRef(__constref int legacyConstRefArg) {}

void useInlayHints()
{
    int a = 0;
    takeRef(a);
    takeReadOnly(a);
    takeConstRef(a);
    takeWriteOnly(a);
    takeOut(a);
    takeIn(a);
    takeBorrow(a);
    takeLegacyConstRef(a);
}

//CONFIG:slang.inlayHints.parameterNames=true
//INLAY

// CHECK: inlay hints:
// CHECK-NEXT: ref refArg:
// CHECK-NEXT: ref_readonly readOnlyArg:
// CHECK-NEXT: ref_readonly constRefArg:
// CHECK-NEXT: ref_writeonly writeOnlyArg:
// CHECK-NEXT: out outArg:
// CHECK-NEXT: inArg:
// CHECK-NEXT: borrow borrowArg:
// CHECK-NEXT: borrow legacyConstRefArg:
