//TEST:SIMPLE_EX: -lang glsl tests/glsl/mutable-buffer-block-member-assignment.slang -target spirv -stage compute -entry main -o mutable-buffer-block-member-assignment.spv

// A mutable GLSL buffer-block member is writable regardless of whether implicit dereference
// insertion classifies the buffer-block base expression itself as an l-value.

buffer MyBlockName
{
    int result;
} outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer.result = 1;
}
