// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

[host]
void main()
{
    print(42);
}

int ordinaryMain()
{
    return 0;
}

int legacyMain(int argc, NativeString* argv)
{
    printf("legacy %d\n", argc);
    return 0;
}
