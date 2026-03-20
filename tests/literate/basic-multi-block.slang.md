# Basic Multi-Block Literate Test

This test validates that `.slang.md` files with multiple code blocks
compile and run correctly.

```slang
//TEST:INTERPRET(filecheck=CHECK):

int helper(int x)
{
    return x * 2;
}
```

The `helper` function was defined above. Non-slang code blocks should
be ignored:

```python
# This Python block should be completely ignored by the compiler
def not_slang():
    pass
```

An untagged code block is also treated as Slang source:

```
int anotherHelper(int x)
{
    return x + 10;
}
```

Now we use both helpers from a `main` function in a third block:

```slang
int main()
{
    // CHECK: 52
    printf("%d\n", helper(21) + anotherHelper(0));
    return 0;
}
```
