//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

// Host `format` accepts the fmt-style `[[fill]align][sign][#][0][width][.precision][type]`
// subset, `String` supports `==` and `!=`, and byte streams report their size as `uint` or, for
// streams larger than 4 GiB, as an exact `double`.

import slang.runtime;

[noinline]
void checkStrings(String a, String b, String c)
{
    assert(a == b);
    assert(!(a != b));
    assert(a != c);
    assert(a != "abc");
    assert(format("{}-{}", 1, 2) == "1-2");
}

[noinline]
void printFormats(int negative, uint value, float pi, String word, bool flag)
{
    print(format("[{:5d}] [{:<5d}] [{:^7d}] [{:*>6}]", negative, negative, negative, value));
    print(format("[{:06d}] [{:+d}] [{: d}]", negative, value, value));
    print(format("[{:x}] [{:X}] [{:#x}] [{:#010x}] [{:b}] [{:#o}]", value, value, value, value, 5u, 8u));
    print(format("[{:.2f}] [{:8.3f}] [{:<8.1f}] [{:+.1f}]", pi, pi, pi, pi));
    print(format("[{:.3e}] [{:.4g}] [{:.4G}] [{:.1%}]", pi, pi, pi * 1000000.0, 0.256));
    print(format("[{:>6}] [{:<6}] [{:^6}] [{:.2}]", word, word, word, word));
    print(format("[{:>6}] [{}] [{:.0f}]", flag, pi, pi));
}

[host]
void main()
{
    checkStrings(format("{}{}", "a", "b"), "ab", "ac");
    printFormats(-42, 255u, 3.14159265, "slang", true);

    ByteStream output = openByteStream("format-specs-string-equality.actual", ByteStreamMode::write);
    RuntimeArray<uint> words = RuntimeArray<uint>::alloc(3);
    for (uint i = 0; i < 3; ++i)
        words[i] = i;
    writeByteStreamArray(output, words);
    print(format("{} {:.1f}", getByteStreamSize(output), getByteStreamSizeAsDouble(output)));
    closeByteStream(output);
}

// CHECK: [  -42] [-42  ] [  -42  ] [***255]
// CHECK: [-00042] [+255] [ 255]
// CHECK: [ff] [FF] [0xff] [0x000000ff] [101] [010]
// CHECK: [3.14] [   3.142] [3.1     ] [+3.1]
// CHECK: [3.142e+00] [3.142] [3.142E+06] [25.6%]
// CHECK: [ slang] [slang ] [slang ] [sl]
// CHECK: [  true] [3.141593] [3]
// CHECK: 12 12.0
