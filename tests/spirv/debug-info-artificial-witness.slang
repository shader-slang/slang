//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g0
//TEST:COMPILE: tests/spirv/debug-info-artificial-witness.slang -g2 -o tests/spirv/debug-info-artificial-witness.slang-module
//TEST:SIMPLE_EX(filecheck=CHECK): tests/spirv/debug-info-artificial-witness.slang-module -entry main -stage compute -target spirv-asm -g2 -O0 -validate-ir

// Check that synthesized method witnesses are artificial while their authored callees and
// direct interface implementations remain unflagged, including generic methods and accessors.
//CHECK-DAG: [[WRAPPER_NAME:%[0-9]+]] = OpString "Reader.$__syn_get"
//CHECK-DAG: [[USER_NAME:%[0-9]+]] = OpString "Reader.get"
//CHECK-DAG: [[DIRECT_NAME:%[0-9]+]] = OpString "DirectReader.get"
//CHECK-DAG: [[GENERIC_WRAPPER_NAME:%[0-9]+]] = OpString "GenericReader.$__syn_get"
//CHECK-DAG: [[GENERIC_USER_NAME:%[0-9]+]] = OpString "GenericReader.get"
//CHECK-DAG: [[GETTER_NAME:%[0-9]+]] = OpString "AuthoredCell.value.get"
//CHECK-DAG: [[SETTER_NAME:%[0-9]+]] = OpString "AuthoredCell.value.set"
//CHECK: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//CHECK-DAG: DebugFunction [[WRAPPER_NAME]] {{.*}} [[WRAPPER_NAME]] [[ARTIFICIAL]]
//CHECK-DAG: DebugFunction [[USER_NAME]] {{.*}} [[USER_NAME]] %uint_0
//CHECK-DAG: DebugFunction [[DIRECT_NAME]] {{.*}} [[DIRECT_NAME]] %uint_0
//CHECK-DAG: DebugFunction [[GENERIC_WRAPPER_NAME]] {{.*}} [[GENERIC_WRAPPER_NAME]] [[ARTIFICIAL]]
//CHECK-DAG: DebugFunction [[GENERIC_USER_NAME]] {{.*}} [[GENERIC_USER_NAME]] %uint_0
//CHECK-DAG: DebugFunction [[GETTER_NAME]] {{.*}} [[GETTER_NAME]] %uint_0
//CHECK-DAG: DebugFunction [[SETTER_NAME]] {{.*}} [[SETTER_NAME]] %uint_0

// Check the executable function markers without debug information. Generated field accessors
// are unnamed and forced inline, so their witness-table entries identify the functions here.
//IR: [nameHint("Reader.get")]
//IR-NEXT: func
//IR: [nameHint("Reader.$__syn_get")]
//IR-NEXT: [Artificial]
//IR-NEXT: func
//IR: [nameHint("DirectReader.get")]
//IR-NEXT: func
//IR: [nameHint("GenericReader.get")]
//IR-NEXT: func
//IR: [nameHint("GenericReader.$__syn_get")]
//IR-NEXT: [Artificial]
//IR-NEXT: func
//IR: [export("{{.*}}4Cell{{.*}}Ag{{.*}}")]
//IR-NEXT: [Artificial]
//IR-NEXT: func [[GETTER:%[a-zA-Z0-9_]+]]
//IR: [export("{{.*}}4Cell{{.*}}Asp{{.*}}")]
//IR-NEXT: [Artificial]
//IR-NEXT: func [[SETTER:%[a-zA-Z0-9_]+]]
//IR: witness_table_entry(%ICellx5Fvaluex5Fget,[[GETTER]])
//IR: witness_table_entry(%ICellx5Fvaluex5Fset,[[SETTER]])
//IR: [nameHint("AuthoredCell.value.get")]
//IR-NEXT: func
//IR: [nameHint("AuthoredCell.value.set")]
//IR-NEXT: func

RWStructuredBuffer<int> data;

interface IRead
{
    int get();
}

struct Reader : IRead
{
    int value;
    int get(int extra = 0) { return value + extra; }
}

struct DirectReader : IRead
{
    int value;
    int get() { return value; }
}

interface IGenericRead
{
    int get<T>(T input);
}

struct GenericReader : IGenericRead
{
    int get<T>(T input, int extra = 0) { return data[0] + extra; }
}

interface ICell
{
    property value : int { get; set; }
}

struct Cell : ICell
{
    int value;
}

struct AuthoredCell : ICell
{
    int storage;
    property value : int
    {
        get { return storage; }
        set(newValue) { storage = newValue; }
    }
}

int read<T : IRead>(T value) { return value.get(); }
int genericRead<T : IGenericRead>(T value, int input) { return value.get(input); }
int update<T : ICell>(T value)
{
    value.value = value.value + 1;
    return value.value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Reader reader = {int(tid.x)};
    DirectReader direct = {int(tid.x)};
    GenericReader generic = {};
    Cell cell = {int(tid.x)};
    AuthoredCell authored = {int(tid.x)};
    data[tid.x] = read(reader) + read(direct) + genericRead(generic, int(tid.x))
        + update(cell) + update(authored);
}
