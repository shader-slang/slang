//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_4
// CHECK: OpEntryPoint
[shader("vertex")]
void main(struct {} a) {}