// This test checks the SPIR-V output when compiling a library containing generics with embedded downstream IR.

//TEST:SIMPLE(filecheck=CHECK): -o tests/library/export-library-generics-test.slang-module -target spirv -embed-downstream-ir -profile lib_6_6 -incomplete-library -dump-ir -verbose-paths

module "export-library-generics";

public cbuffer Constants {
    public float x;
    public float y;
}

interface MyInterface
{
    int myMethod(int a);
}

struct MyType : MyInterface
{
    int myMethod(int a)
    {
        return a * 3;
    }
}

int genericFunc<T: MyInterface>(T arg)
{
    return arg.myMethod(3);
}

public int normalFuncUsesGeneric(int a)
{
    MyType obj;
    return genericFunc(obj);
}

public int normalFunc(int a, float b)
{
    return a - floor(b);
}

// CHECK:$
// CHECK:let  %29        : CapabilitySet = capabilityConjunction(1 : Int, 3 : Int, 64 : Int)
// CHECK:let  %28        : CapabilitySet = capabilityConjunction(1 : Int, 3 : Int, 63 : Int)
// CHECK:let  %27        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 76 : Int)
// CHECK:let  %26        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 75 : Int)
// CHECK:let  %25        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 74 : Int)
// CHECK:let  %24        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 73 : Int)
// CHECK:let  %23        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 72 : Int)
// CHECK:let  %22        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 71 : Int)
// CHECK:let  %21        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 70 : Int)
// CHECK:let  %20        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 69 : Int)
// CHECK:let  %19        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 68 : Int)
// CHECK:let  %18        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 67 : Int)
// CHECK:let  %17        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 66 : Int)
// CHECK:let  %16        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 65 : Int)
// CHECK:let  %15        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 64 : Int)
// CHECK:let  %14        : CapabilitySet = capabilityConjunction(1 : Int, 2 : Int, 63 : Int)
// CHECK:[public]
// CHECK:[export("_STR29export_2Dxlibrary_2Dxgenerics30SLANG_ParameterGroup_Constants")]
// CHECK:[nameHint("SLANG_ParameterGroup_Constants")]
// CHECK:struct %SLANGx5FParameterGroupx5FConstants      : Type
// CHECK:{
// CHECK:        field(%x, Float)
// CHECK:        field(%y, Float)
// CHECK:}
// CHECK:[public]
// CHECK:[export("_SVR29export_2Dxlibrary_2Dxgenerics30SLANG_ParameterGroup_Constants1x")]
// CHECK:[nameHint("x")]
// CHECK:let  %x : _     = key
// CHECK:[public]
// CHECK:[export("_SVR29export_2Dxlibrary_2Dxgenerics30SLANG_ParameterGroup_Constants1y")]
// CHECK:[nameHint("y")]
// CHECK:let  %y : _     = key
// CHECK:let  %6 : witness_table_t(%x5Fx5FBuiltinFloatingPointType)      = thisTypeWitness
// CHECK:let  %126       : _     = interface_req_entry(%127, witness_table_t(%x5Fx5FBuiltinRealType))
// CHECK:let  %128       : _     = interface_req_entry(%7, witness_table_t(%IFloat))
// CHECK:let  %129       : _     = interface_req_entry(%130, Func(this_type(%x5Fx5FBuiltinFloatingPointType)))
// CHECK:[availableInDownstreamIR(6 : Int)]
// CHECK:[treatAsDifferentiableDecoration]
// CHECK:[constructor(true)]
// CHECK:[method]
// CHECK:[public]
// CHECK:[export("_SR29export_2Dxlibrary_2Dxgenerics30SLANG_ParameterGroup_ConstantsR8_24xinitp2pi_fi_f")]
// CHECK:[nameHint("SLANG_ParameterGroup_Constants.$init")]
// CHECK:func %SLANGx5FParameterGroupx5FConstantsx5Fx24init      : Func(%SLANGx5FParameterGroupx5FConstants, Float, Float)
// CHECK:EmbeddedDownstreamIR(6 : Int,
// CHECK:; SPIR-V
// CHECK:; Version: 1.5
// CHECK:; Generator: Khronos; 40
// CHECK:; Bound: 61
// CHECK:; Schema: 0
// CHECK:               OpCapability Linkage
// CHECK:               OpCapability Shader
// CHECK:         %44 = OpExtInstImport "GLSL.std.450"
// CHECK:               OpMemoryModel Logical GLSL450
// CHECK:               OpSource Slang 1
// CHECK:               OpName %SLANG_ParameterGroup_Constants "SLANG_ParameterGroup_Constants"
// CHECK:               OpMemberName %SLANG_ParameterGroup_Constants 0 "x"
// CHECK:               OpMemberName %SLANG_ParameterGroup_Constants 1 "y"
// CHECK:               OpName %x "x"
// CHECK:               OpName %y "y"
// CHECK:               OpName %SLANG_ParameterGroup_Constants__init "SLANG_ParameterGroup_Constants.$init"
