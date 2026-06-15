# Prompt: docs/generated/tests/conformance/types-interface/

See [`_common.md`](../../../../generated/tests/_meta/prompts/_common.md).

## Target

Bundle at `docs/generated/tests/conformance/types-interface/`,
anchored to
[`docs/language-reference/types-interface.md`](../../../../language-reference/types-interface.md).

## Source doc structure

The document covers:

1. **Syntax** — grammar productions for interface declarations and associated type declarations.
2. **Description** — normative claims about what interfaces consist of:
   - Method requirements (with and without body = default)
   - Constructor requirements
   - Property/subscript/call-operator requirements
   - Associated named types (satisfied by typealias or nested struct)
   - Static const data members (int or bool only)
   - Method compatibility rules (parameter / return type covariance)
   - Visibility rules (lower is OK, higher is not)
   - Interface inheritance
3. **Interface-Conforming Variants** (existential types) — experimental section.
4. **Example** — full `IBase`/`ITest`/`ConcreteInt32`/`ConcreteInt16` worked example.
5. **Memory Layout and Dispatch Mechanism** — explicitly non-normative.

## Claim-extraction strategy

Normative claims are extracted from the **Description** and **Example** sections.
The **Syntax** section is normative grammar — claims about allowed member kinds (method,
constructor, property, subscript, call-op, associated type, static const) derive from it.
The **Memory Layout** section is explicitly marked as non-normative and informational;
no claims are extracted from it. The **Interface-Conforming Variants** section is marked
experimental; claims there are extracted for interface-typed variable declaration but
implementation-detail remarks are not tested.

## High-value claims

- **Interface declaration with all requirement types** (the doc's ITest example)
- **Method requirements** (with and without default body)
- **Override modifier** for default methods
- **Property requirements** satisfied by accessors or plain variable
- **Constructor requirements** (`__init`)
- **Subscript and call-operator requirements**
- **Associated type requirements** (via typealias and nested struct)
- **Static const members** (type restriction: int or bool only; E30306 for float)
- **Interface inheritance** (child interface adds to parent)
- **Method compatibility** (covariant return, contravariant parameter)
- **Visibility rule** (implementation may be lower but not higher)
- **Generic parameterized interface** (Remark 2)
- **Interface-typed function parameters** get compile-time specialization
- **Negative: missing requirement → E38100**
- **Negative: wrong return type → E38106**
- **Negative: float static const in interface → E30306**

## What NOT to test here

- Memory layout of interface-conforming variants — explicitly non-normative
- Dynamic dispatch implementation details (type-tag switch) — non-normative
- Interface variant restrictions (no opaque/non-copyable/unsized data members) —
  experimental; the constraints can't be exercised without GPU textures on CPU
- Deprecated default-initialization of interface variants (Remark 2 in the
  Interface-Conforming Variants section) — deprecated behavior; no test needed
