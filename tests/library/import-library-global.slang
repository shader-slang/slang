//TEST:SIMPLE(filecheck=CHECK): -o tests/library/import-library-global.slang-module -target spirv -embed-downstream-ir -dump-ir

// CHECK: [availableInDownstreamIR(6 : Int)]
// CHECK: [export("_SR27import_2Dxlibrary_2Dxglobal11internalgetp0pi")]
// CHECK: [nameHint("internalget")]
// CHECK: func %internalget       : Func(Int)
// CHECK: [availableInDownstreamIR(6 : Int)]
// CHECK: [public]
// CHECK: [export("_SR27import_2Dxlibrary_2Dxglobal3getp0pi")]
// CHECK: [nameHint("get")]
// CHECK: func %get       : Func(Int)
// CHECK: EmbeddedDownstreamIR(6 : Int,
// CHECK: OpCapability Linkage
// CHECK: OpDecorate %myGlobalVar LinkageAttributes "_SV11myGlobalVar" Import
// CHECK: OpDecorate %internalget LinkageAttributes "_SR27import_2Dxlibrary_2Dxglobal11internalgetp0pi" Export
// CHECK: OpDecorate %get LinkageAttributes "_SR27import_2Dxlibrary_2Dxglobal3getp0pi" Export

module "import-library-global";

extern static int myGlobalVar;

public int get()
{
    return internalget();
}

internal int internalget()
{
    return myGlobalVar;
}