> TODO

Expression Type Conversions
===========================

**Grammar:**

> **`'('`** _`type-expr`_ **`')'`**) _`base-expr`_

A _cast expression_ converts a value (_`base-expr`_) to the desired type (_`type-expr`_). This is also known
as an explicit type conversion. See [Expression Type Conversions](expressions-conversions.md) for details.

```hlsl
(int) 1.0f
```

A cast expression can perform both built-in type conversions and invoke any single-argument initializers of the target type.

As a compatibility feature for older code, Slang supports using a cast where the base expression is an integer
literal zero and the target type is a user-defined structure type:

```hlsl
MyStruct s = (MyStruct) 0;
```

The semantics of such a cast are equivalent to initialization from an empty initializer list:

```hlsl
MyStruct s = {};
```

cases:

- cast to void. But see also https://github.com/shader-slang/slang/issues/11315
- passing a void value to `return` statement is allowed for `void`-typed functions
