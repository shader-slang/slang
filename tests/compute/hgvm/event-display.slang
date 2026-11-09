// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:INTERPRET(hgvm,filecheck=COMPACT): -trace-all -color never
//TEST:INTERPRET(hgvm,filecheck=FULL): -trace-all -color never -full-names

class DisplayValue
{
    uint value;
    __init(uint value) { this.value = value; }
}

[host]
void main()
{
    let value = new DisplayValue(42);
    print(value.value);
}

// COMPACT: heap-alloc {{.*}}type=DisplayValue{{$}}
// COMPACT: 42
// COMPACT: heap-free {{.*}}type=DisplayValue{{$}}
// FULL: heap-alloc {{.*}}type=DisplayValue module=
// FULL: 42
// FULL: heap-free {{.*}}type=DisplayValue module=
