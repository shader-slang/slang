// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
[shader("compute")]
[numthreads(1, 1, 1)]
void probe<T>() {}

// The closure is used only as a specialization argument, never as a runtime value.
[host]
public void forward<T>(T value)
{
    dispatch(probe<T>, uint3(1, 1, 1))();
}

[host]
public void make<T : __BuiltinArithmeticType>()
{
    forward(() => T(1));
}
