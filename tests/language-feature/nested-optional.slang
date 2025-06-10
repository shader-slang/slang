//TEST:INTERPRET(filecheck=CHECK):

Optional<Optional<int>> getNone() { return none; }

void main()
{
    Optional<Optional<Optional<int>>> val = Optional<Optional<int>>(5);
    Optional<Optional<Optional<int>>> defaultVal1 = none;
    Optional<Optional<Optional<int>>> defaultVal2 = getNone();

    // CHECK: 8
    printf("%d\n", sizeof(val));

    // CHECK: success
    if (defaultVal1.hasValue == defaultVal2.hasValue)
    {
        printf("success\n");
    }
    else
    {
        printf("failure\n");
    }

    // CHECK: value: 5
    if (let x = val)
    {
        if (let y = x)
        {
            if (let z = y)
            {
                printf("value: %d\n", z);
            }
        }
    }
}