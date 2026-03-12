//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_4
// CHECK: error[E30102]:
// CHECK: :[[# @LINE+2]]:
[shader("vertex")]
void main(struct {} a) {}
