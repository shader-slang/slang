## Configure CMake and Build

Slang is already built in debug configuration, so you should be able to run targets
like `slangc`, `slang-test`, `slangi` etc. right away.

If you made some changes and need to rebuild Slang, follow these steps:

1. Configure cmake with `cmake --preset default`.
2. Run `cmake --workflow --preset debug` to build.

Detailed build instructions can be found in docs/building.md

## Formatting

DO THIS BEFORE COMMITING YOUR CHANGES:
    RUN `./extras/formatting.sh` to format your changes first!!
Your PR needs to be formatted according to our coding style.

## Labeling your PR

All PRs needs to be labeled as either "pr: non-breaking" or "pr: breaking".
Add the "pr: breaking" label to  your PR if you are introducing public API changes that breaks ABI compabibility,
or you are introducing changes to the Slang language that will cause the compiler to error out on existing Slang code.
It is rare for a PR to be a breaking change.

## Debugging

If you encounter a bug related to a problematic instruction, it is often useful to trace the location where the instruction is created.
You can use the `extras/insttrace.py` script to do this. For example, during debugging you find that an instruction with `_debugUID=1234`
is wrong, you can run the following command to trace the callstack where the instruction is created:

```bash
# From workspace root:
python3 ./extras/insttrace.py 1234 ./build/Debug/bin/slangc tests/my-test.slang -target spirv
```

## Testing

Your PR should include a regression test for the bug you are fixing.
Normally, these tests present as a `.slang` file under `tests/` directory.
You will need to run your test with `slang-test tests/path/to/your-new-test.slang`.
You will need to build the `slang-test` target first.
Note that your execution environment does not have a GPU, so you can't run any tests that requires a GPU locally, for example,
you won't be able to run a shader test using D3D12, Vulkan, Metal or WGSL.

If the changes you are making is not specific to a particular GPU target, you can craft your test case to run on the CPU
by writing the following as the first line of your test shader:

```
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
```
See `tests/language-feature/lambda/lambda-0.slang` for a full example.

Or you can craft your test to run with `slangi`  (byte-code interpreter), such as:

```
//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    //CHECK: hello!
    printf("hello!");
}
```

If you are working on a GPU specific feature, don't try to run the test locally, just leave your PR to the CI for verification.