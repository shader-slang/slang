//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g2 -validate-ir

struct Pair<T> { T x; T y; }
class Box { uint value; __init() {} }

[noinline]
uint readReference(__ref uint reference)
{
    reference += 1;
    return reference;
}

[noinline]
double twice(double input)
{
    return input * 2.0;
}

[host]
void main()
{
    uint scalar = 3;
    Pair<uint> pair = { 1, 2 };
    let immutable = pair;
    Box object = new Box();
    object.value = scalar;
    RuntimeArray<uint> values = RuntimeArray<uint>::alloc(2);
    values[0] = scalar;
    String text = "hello";
    double wide = 16777217.0LF;
    print(readReference(scalar));
    print(immutable.x);
    print(object.value);
    print(values[0]);
    print(text);
    print(twice(wide));
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[PAIR:%[A-Za-z0-9_]+]] = OpStructType
// CHECK-DAG: [[BOX:%[A-Za-z0-9_]+]] = OpClassType
// CHECK-DAG: [[ARRAY:%[A-Za-z0-9_]+]] = OpRuntimeArrayType
// CHECK-DAG: [[STRING:%[A-Za-z0-9_]+]] = OpStringType
// CHECK-DAG: [[DOUBLE:%[A-Za-z0-9_]+]] = OpFloat64Type
// CHECK-DAG: [[REF:%[A-Za-z0-9_]+]] = OpRefType
// CHECK: Section("FUNC")
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugVariable %strPair{{(_[0-9]+)?}} [[PAIR]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugVariable %strImmutable [[PAIR]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugVariable %strObject [[BOX]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugVariable %strValues [[ARRAY]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugVariable %strText{{(_[0-9]+)?}} [[STRING]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugVariable %strWide [[DOUBLE]]
// CHECK-DAG: [[REFERENCE:%[A-Za-z0-9_]+]] = OpDebugVariable %strReference [[REF]]
// CHECK-DAG: OpDebugValue [[REFERENCE]] {{%[A-Za-z0-9_]+}}
