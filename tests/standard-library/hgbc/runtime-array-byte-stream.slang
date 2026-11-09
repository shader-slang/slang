// TEST:SIMPLE(filecheck=CHECK): -target hgbc

import slang.runtime.io.byte_stream;

[host]
void main()
{
    ByteStream input = openByteStream("input.bin", ByteStreamMode::read);
    RuntimeArray<uint> inputValues = RuntimeArray<uint>::alloc(4);
    readByteStreamArray(input, inputValues);
    closeByteStream(input);

    ByteStream output = openByteStream("output.bin", ByteStreamMode::write);
    writeByteStreamArray(output, inputValues);
    closeByteStream(output);
}

// CHECK: OpByteStreamRead RuntimeArray
// CHECK: OpByteStreamWrite RuntimeArray
