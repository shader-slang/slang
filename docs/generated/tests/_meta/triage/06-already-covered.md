# 06 — Entries already covered by an existing finding

Thirteen of the 21 entries correspond to compiler defects that already have a
YAML under `docs/generated/tests/_meta/findings/`. No new finding is needed;
they are listed so the triage is complete and so a reader can see which
finding to follow.

| Expected-failure entry                                                                | Existing finding                                             |
| ------------------------------------------------------------------------------------- | ------------------------------------------------------------ |
| `conformance/expressions-operators/conditional-does-not-short-circuit.slang`          | `slangi-vm-ternary-short-circuits-against-spec`              |
| `conformance/expressions-member-access/matrix-mij-zero-based.slang`                   | `slangi-vm-matrix-mij-swizzle-working-set-oob`               |
| `conformance/types-extension/enum-extension-static-members.slang`                     | `slangi-vm-global-var-bytecode-unsupported`                  |
| `conformance/types-vector-and-matrix/vector-binary-scalar-broadcast.slang`            | `slangi-vm-vector-scalar-binary-op-wrong-values`             |
| `conformance/types-vector-and-matrix/vector-swizzle-assign.slang`                     | `slangi-vm-swizzle-assign-bytecode-unimplemented`            |
| `design/ast-reference/statements/switchstmt-case-decl-used-in-later-case.slang (cpu)` | `switch-case-decl-used-in-later-case-invalid-cpp-emit`       |
| `design/ast-reference/expressions/countof-static-array.slang`                         | `countof-on-array-returns-element-size`                      |
| `design/ir-reference/misc/countof-fixed-size-array.slang (cpu)`                       | `countof-local-array-returns-bytewidth`                      |
| `design/ast-reference/expressions/new-expr-constructor-args.slang.1`                  | `new-expr-with-constructor-args-internal-error`              |
| `design/ast-reference/expressions/new-expr-constructor-args.slang.2`                  | (same)                                                       |
| `design/ast-reference/declarations/refaccessor-property.slang.1`                      | `declarations-refaccessor-spirv-invalid-funcall-return-type` |
| `design/ast-reference/declarations/refaccessor-property.slang.2`                      | `declarations-refaccessor-metal-unknown-addressspace-abort`  |
| `design/target-pipelines/metal/append-buffer-params-carry-buffer-slots.slang`         | `metal-append-buffer-params-missing-binding-slot`            |
| `design/ast-reference/values/builtinoperationintval-enum-operands-fold.slang`         | `enum-cast-in-generic-array-bound-rejected`                  |

Two observations worth carrying forward rather than acting on now.

**A cluster of five is one subsystem.** The `slangi-vm-*` findings are all the
bytecode interpreter: short-circuit evaluation, swizzle assignment, global
vars, matrix `_mij` access, and vector/scalar binary ops. They are filed
separately and that is probably right for tracking, but anyone picking up VM
work should read them together rather than one at a time.

**The last row is a loose match.** `builtinoperationintval-enum-operands-fold`
was matched to `enum-cast-in-generic-array-bound-rejected` by keyword, not by
reading both. The link should be confirmed before relying on it; if it does not
hold, that entry belongs in the untriaged pile with 07.
