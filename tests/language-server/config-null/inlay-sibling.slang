//TEST:LANG_SERVER(filecheck=CHECK):

// Each inlay-hint setting arrives as its own `workspace/didChangeConfiguration` key. A message that
// does not mention a setting leaves it unchanged, and a null value restores its default (off).

int scale(int count)
{
    return count * 2;
}

void useInlayHints()
{
    let doubled = scale(3);
}

//CONFIG:slang.inlayHints.deducedTypes=true
//CONFIG:slang.inlayHints.parameterNames=true
//INLAY
//CONFIG:slang.inlayHints.deducedTypes=null
//INLAY
//CONFIG:slang.inlayHints.parameterNames=false
//INLAY

// CHECK: config: slang.inlayHints.parameterNames=true
// CHECK: inlay hints:
// CHECK-NEXT: : int
// CHECK-NEXT: count:
// CHECK: config: slang.inlayHints.deducedTypes=null
// CHECK: inlay hints:
// CHECK-NOT: : int
// CHECK: count:
// CHECK: config: slang.inlayHints.parameterNames=false
// CHECK: inlay hints:
// CHECK-NOT: count:
