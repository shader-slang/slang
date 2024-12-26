//TEST(compute):SIMPLE(filecheck=CHECK): -target spirv

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint8_t4_packed packedU = 0U;
    int8_t4_packed packedS = 0xFU;
    uint val1 = 0xFU;

    //
    // Implicit conversions between the packed types are not OK, they must be explicit.
    //

    // CHECK: warning 30081: implicit conversion from 'int8_t4_packed' to 'uint8_t4_packed' is not recommended
    packedU = packedS;
    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'int8_t4_packed' is not recommended
    packedS = packedU;


    // Implicit casting from 32 bit literals are OK.
    packedU = 32U;
    packedS = 32;

    // Implicit casting from 64 bit literals are not OK.
    // CHECK: warning 30081: implicit conversion from 'uint64_t' to 'uint8_t4_packed' is not recommended
    packedU = 0xFFFFFFFFFFULL;
    // CHECK: warning 30081: implicit conversion from 'int64_t' to 'uint8_t4_packed' is not recommended
    packedU = 0xFFFFFFFFFFLL;

    //
    // Explicit casting from other builtin integer types are OK.
    //
    packedU = uint8_t4_packed(val1);
    packedU = uint8_t4_packed(uint16_t(123));
    val1 = uint(packedS);

    //
    // Implicit casting from other builtin integer types are not OK.
    //

    // CHECK: warning 30081: implicit conversion from 'uint' to 'uint8_t4_packed' is not recommended
    packedU = val1;
    // CHECK: warning 30081: implicit conversion from 'uint' to 'int8_t4_packed' is not recommended
    packedS = val1;
    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'uint' is not recommended
    val1 = packedU;
    // CHECK: warning 30081: implicit conversion from 'int8_t4_packed' to 'uint' is not recommended
    val1 = packedS;

    // CHECK: warning 30081: implicit conversion from 'uint8_t' to 'uint8_t4_packed' is not recommended
    packedU = uint8_t(1);
    // CHECK: warning 30081: implicit conversion from 'int64_t' to 'int8_t4_packed' is not recommended
    packedS = int64_t(1);
    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'uint64_t' is not recommended
    uint64_t val2 = packedU;
    // CHECK: warning 30081: implicit conversion from 'int8_t4_packed' to 'int16_t' is not recommended
    int16_t val3 = packedS;

    //
    // Arithmetic and logical (bitwise) operations are not supported on packed types,
    // but overload to integer types will be made and unrecommended conversion warnings
    // should be thrown out.
    //

    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'int' is not recommended
    packedU = uint8_t4_packed(packedU + 32);
    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'int' is not recommended
    packedU = uint8_t4_packed(packedU / 2);
    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'int' is not recommended
    packedU = uint8_t4_packed(packedU | 0xF);
    // CHECK: warning 30081: implicit conversion from 'uint8_t4_packed' to 'int' is not recommended
    packedU = uint8_t4_packed(packedU & 0x3);
}
