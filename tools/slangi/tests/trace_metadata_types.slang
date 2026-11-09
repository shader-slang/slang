// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

namespace Tracing
{
    class Box<T>
    {
        T value;
        __init(T initialValue) { value = initialValue; }
    }
}
