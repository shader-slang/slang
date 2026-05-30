## Configure CMake and Build

Slang is already built in debug configuration, so you should be able to run targets
like `slangc`, `slang-test`, `slangi` etc. right away.

If you made some changes and need to rebuild Slang, follow these steps:

1. Configure cmake with `cmake --preset default`.
2. Run `cmake --workflow --preset debug` to build.

Detailed build instructions can be found in docs/building.md

## Formatting

DO THIS BEFORE COMMITTING YOUR CHANGES:
RUN `./extras/formatting.sh` to format your changes first!!
Your PR needs to be formatted according to our coding style.

The formatting script requires these tools:

- **clang-format** 17-18 (for C++ files)
- **gersemi** 0.21-0.22 (for CMake files)
- **prettier** 3+ (for YAML/JSON/Markdown files)
- **shfmt** 3+ (for shell scripts)

If you need to install these tools locally:

**Ubuntu/Debian:**

```bash
sudo apt-get install clang-format-17 npm
python3 -m pip install gersemi==0.21.0
sudo npm install -g prettier@3
wget https://github.com/mvdan/sh/releases/download/v3.10.0/shfmt_v3.10.0_linux_amd64 -O /tmp/shfmt
sudo install /tmp/shfmt /usr/local/bin/shfmt
```

Note: If pip install fails with externally-managed-environment error, use `--break-system-packages` flag or create a virtual environment.

**macOS (Homebrew):**

```bash
brew install clang-format gersemi prettier shfmt
```

You can also use `./extras/formatting.sh --check-only` to verify formatting without modifying files.

## Labeling your PR

All PRs needs to be labeled as either "pr: non-breaking" or "pr: breaking".
Add the "pr: breaking" label to your PR if you are introducing public API changes that breaks ABI compabibility,
or you are introducing changes to the Slang language that will cause the compiler to error out on existing Slang code.
It is rare for a PR to be a breaking change.

## Problem-solving methodology

Follow the principled path, not the minimal-edit-distance path: fix root causes (usually upstream
in an IR pass, lowering, or the AST/IR representation), not symptoms in emit/codegen. Question every
change — if you cannot name a test that fails without it, it probably should not exist. Do not mask
malformed AST/IR with guards or special cases; make the representation correct so consumers stay
simple. When data is conceptually an unordered key→value set (e.g. witness-table / interface
requirement entries), address it by role/key, never by position/index. Keep a scratch log
throughout the task recording the problem, how issues cascade (one fix exposing the next), the fix
for each and why it is principled (with a code trace), and rejected alternatives; distill that log
into the PR description.

## PR description format

Write every PR description in this four-part format:

1. **Motivation** — the problem, with a concrete example / motivating test case.
2. **Proposed solution** — the approach and why it is principled.
3. **Change summary** — the files/areas touched and what each does.
4. **Process report** — explain every change with a logical reason. For a change addressing a
   cascading issue, describe the issue (with its motivating test case) and justify the fix with a
   code trace (the exact functions/insts involved), explaining why it is necessary and principled
   rather than a workaround.

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

Or you can craft your test to run with `slangi` (byte-code interpreter), such as:

```
//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    //CHECK: hello!
    printf("hello!");
}
```

If you are working on a GPU specific feature, don't try to run the test locally, just leave your PR to the CI for verification.
