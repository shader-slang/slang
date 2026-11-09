// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
import trace_metadata_types;

[host]
void main()
{
    let box = new Tracing::Box<int>(42);
    print(box.value);
}
