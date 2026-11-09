//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

public struct Box<T> { T value; }
[host]
void main(Box<float> box) { print(box.value); }

// CHECK-DAG: [[VALUE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 5 "value"
// CHECK-DAG: OpStructType [[VALUE]]
