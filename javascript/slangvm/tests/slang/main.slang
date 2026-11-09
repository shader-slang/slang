[host]
int add(int a, int b) { return a + b; }

[host]
half roundHalf(float value) { return half(value); }

[host]
int printAndAdd(int a, int b)
{
    print("entered");
    return a + b;
}

[host]
int then() { return 17; }
