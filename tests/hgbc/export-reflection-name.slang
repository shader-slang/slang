//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

import semantic_export_api;

[host]
void main()
{
    var counter = ImportedCounter(3);
    counter.increment(7);
    print(counter.value);
}

// An imported method has no entry-point wrapper. Its export must use the declared API name,
// not the qualified display hint ImportedCounter.increment or an obfuscated linkage name.
// CHECK-DAG: [[INCREMENT:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 9 "increment"
// CHECK-DAG: OpExport Method [[INCREMENT]]
