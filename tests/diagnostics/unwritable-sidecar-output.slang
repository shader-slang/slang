//TEST:SIMPLE(filecheck=DEPFILE): -target hlsl -entry main -stage compute -depfile no-such-dir-13294/deps.d
//TEST:SIMPLE(filecheck=REFLECTION): -target hlsl -entry main -stage compute -reflection-json no-such-dir-13294/reflection.json

[shader("compute")]
void main() {}

// DEPFILE: result code = -1
// DEPFILE: E00004{{.*}}no-such-dir-13294/deps.d

// REFLECTION: result code = -1
// REFLECTION: E52004{{.*}}no-such-dir-13294/reflection.json
