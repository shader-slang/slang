//TEST:SIMPLE(filecheck=CHECK): -target hgbc

[host]
void main()
{
    RuntimeArray<uint> source = RuntimeArray<uint>::alloc(8);
    RuntimeArray<uint> destination = RuntimeArray<uint>::alloc(8);
    copy(destination, 2, source, 1, 4);
}

// CHECK: OpCopyArrayRange
// CHECK-NOT: OpCopyArrayRange
