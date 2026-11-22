//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target host-cpp -entry main -stage compute

// A `[DllImport]` or `[DllExport]` signature rejects managed types, whose lifetime native code
// cannot honor: a `String` anywhere except a by-value input parameter, and any class, resource,
// or aggregate or pointer that contains a managed type. `dll-import-plain-data-signature.slang`
// covers the accepted forms.

struct Named
{
    String name;
    int id;
}

class Node
{
    int value;
}

// CHECK-DAG: 'exportString' cannot use 'string' as a result;
[DllExport]
export String exportString()
{
    return "x";
}

// CHECK-DAG: 'exportOutString' cannot use 'string' as a by-reference parameter;
[DllExport]
export void exportOutString(out String text)
{
    text = "x";
}

// CHECK-DAG: 'exportNamed' cannot use 'Named' as a parameter;
[DllExport]
export int exportNamed(Named value)
{
    return value.id;
}

// CHECK-DAG: 'importNamedArray' cannot use 'array<Named,2>' as a parameter;
[DllImport("native", "importNamedArray")]
int importNamedArray(Named values[2]);

// CHECK-DAG: 'importClass' cannot use 'Node' as a parameter;
[DllImport("native", "importClass")]
void importClass(Node node);

// CHECK-DAG: 'importInOutString' cannot use 'string' as a by-reference parameter;
[DllImport("native", "importInOutString")]
void importInOutString(inout String text);

// CHECK-DAG: 'importStringPointer' cannot use '{{.*}}' as a parameter;
[DllImport("native", "importStringPointer")]
void importStringPointer(String* text);

// CHECK-DAG: 'importBuffer' cannot use '{{.*}}StructuredBuffer{{.*}}' as a parameter;
[DllImport("native", "importBuffer")]
void importBuffer(RWStructuredBuffer<int> buffer);

RWStructuredBuffer<int> gBuffer;

export __extern_cpp int main()
{
    Named values[2];
    values[0].id = 0;
    values[1].id = 1;
    String text = "a";
    importNamedArray(values);
    importClass(new Node());
    importInOutString(text);
    importStringPointer(&text);
    importBuffer(gBuffer);
    return 0;
}
