//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target host-cpp -entry main -stage compute

// A `[DllImport]` or `[DllExport]` signature rejects managed types, whose lifetime native code
// cannot honor: a `String` anywhere except a by-value input parameter, and any class, resource,
// unsized array, or aggregate or pointer that contains a managed type. Each diagnostic points at
// the function's declaration line, which follows its attribute.
// `dll-import-plain-data-signature.slang` covers the accepted forms.

struct Named
{
    String name;
    int id;
}

class Node
{
    int value;
}

// `next` refers back to `ListNode` before the managed `name` is reached, so classifying it
// revisits the type that is already being classified.
struct ListNode
{
    ListNode* next;
    String name;
}

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'exportString' cannot use 'string' as a result;
[DllExport]
export String exportString()
{
    return "x";
}

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'exportOutString' cannot use 'string' as a by-reference parameter;
[DllExport]
export void exportOutString(out String text)
{
    text = "x";
}

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'exportNamed' cannot use 'Named' as a parameter;
[DllExport]
export int exportNamed(Named value)
{
    return value.id;
}

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importNamedArray' cannot use 'array<Named,2>' as a parameter;
[DllImport("native", "importNamedArray")]
int importNamedArray(Named values[2]);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importClass' cannot use 'Node' as a parameter;
[DllImport("native", "importClass")]
void importClass(Node node);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importInOutString' cannot use 'string' as a by-reference parameter;
[DllImport("native", "importInOutString")]
void importInOutString(inout String text);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importStringPointer' cannot use '{{.*}}' as a parameter;
[DllImport("native", "importStringPointer")]
void importStringPointer(String* text);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importBuffer' cannot use '{{.*}}StructuredBuffer{{.*}}' as a parameter;
[DllImport("native", "importBuffer")]
void importBuffer(RWStructuredBuffer<int> buffer);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importNamedTuple' cannot use '{{.*}}' as a parameter;
[DllImport("native", "importNamedTuple")]
void importNamedTuple(Tuple<int, Named> value);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importUnsized' cannot use '{{.*}}' as a by-reference parameter;
[DllImport("native", "importUnsized")]
void importUnsized(inout int values[]);

// CHECK-DAG: dll-signature-managed-types.slang:[[# @LINE+3]]:
// CHECK-DAG: 'importList' cannot use '{{.*}}' as a parameter;
[DllImport("native", "importList")]
void importList(ListNode* head);

RWStructuredBuffer<int> gBuffer;

export __extern_cpp int main()
{
    Named values[2];
    values[0].id = 0;
    values[1].id = 1;
    String text = "a";
    int unsized[] = { 1, 2 };
    importNamedArray(values);
    importClass(new Node());
    importInOutString(text);
    importStringPointer(&text);
    importBuffer(gBuffer);
    importNamedTuple(makeTuple(1, values[0]));
    importUnsized(unsized);
    importList(nullptr);
    return 0;
}
