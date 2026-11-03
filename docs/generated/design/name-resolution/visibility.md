---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T00:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: a39bda949a6aee96678d87904efb315ace2ed593bf5e94a26786facff04438e6
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Visibility

This document covers Slang's declaration visibility rules: which
`public` / `private` / `internal` keyword each decl carries, the
defaults per language version, and where in the resolution pipeline
visibility filtering happens. The intended reader is a developer
adding or modifying a visibility-related diagnostic, a language
designer reasoning about cross-module access, or anyone wondering why
a declaration in another module is or is not reachable.

Lookup itself is described in [lookup.md](lookup.md); overload-
resolution-time visibility filtering is described in
[overload-resolution.md](overload-resolution.md). This page is the
single source for *what* counts as visible.

## Source

Visibility modifiers are declared in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)
(lines 60-82). The `DeclVisibility` enum that the rest of the
compiler reasons about is in
[slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
(lines 2133-2139). The classification helper `getDeclVisibility` and
the per-module default-visibility setup live in
[slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp);
the visibility filter applied to lookup results and the
`isDeclVisibleFromScope` predicate live in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp);
the overload-time check sits in
[slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp);
per-decl visibility validation is in
[slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp).

## Concepts

- `VisibilityModifier` (abstract,
  [slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)
  line 60) — base class for visibility keywords. The three concrete
  subclasses are `PublicModifier` (line 66), `PrivateModifier`
  (line 72), and `InternalModifier` (line 78). Each is an empty
  marker class.
- `DeclVisibility`
  ([slang-ast-support-types.h](../../../../source/slang/slang-ast-support-types.h)
  lines 2133-2139) — internal `enum class` with values `Private`,
  `Internal`, `Public`, and the alias `Default = Internal`. The numeric
  order is `Private < Internal < Public`; `Math::Min` over visibility
  values is used throughout to compute the effective visibility of a
  composite (e.g. a parameterized type).
- `ModuleDecl::defaultVisibility`
  ([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) line
  893) — `DeclVisibility` field that records the default visibility
  applied to members of the module that carry no explicit modifier.
  Initialized to `DeclVisibility::Internal` at declaration time and
  overridden during semantic checking as described below.
- `SlangLanguageVersion languageVersion`
  ([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) line
  891) — the per-module language-version field. A `module` declaration
  carries no version of its own. In
  `slang-compile-request.cpp`, the translation unit's module takes the
  version from a `#language` source directive when a primary file has one
  (the first directive wins; a conflicting one is diagnosed), and
  otherwise from the compile request's language-version option (lines
  504-552). The parser then upgrades a legacy value to 2025 through
  `maybeUpgradeLanguageVersionFromLegacy`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line
  1271) at three sites: when a visibility modifier is parsed (line 1310),
  on an `__include` declaration (line 1442), and on a `module`
  declaration (line 1481). The
  legacy version constant `SLANG_LANGUAGE_VERSION_LEGACY` is
  `2018` in [slang.h](../../../../include/slang.h) (line 5916 at
  `source_commit`); the same enum defines `SLANG_LANGUAGE_VERSION_2025`,
  `SLANG_LANGUAGE_VERSION_2026`, `SLANG_LANGUAGE_VERSION_LATEST` (an
  alias for 2026), `SLANG_LANGUAGE_VERSION_202C`
  (2027; also named `SLANG_LANGUAGE_VERSION_NEXT`, the development
  version), and `SLANG_LANGUAGE_VERSION_DEFAULT` as
  `SLANG_LANGUAGE_VERSION_LEGACY` (line 5930) — so the documented
  language default is still 2018. The misspelled
  `SLANG_LANGAUGE_VERSION_DEFAULT` (line 5929) is retained beside it,
  marked in the header as deprecated and kept for source compatibility.
  `SlangGlobalSessionDesc::minLanguageVersion` (line 5951)
  defaults to `SLANG_LANGUAGE_VERSION_2025`. The visibility defaults
  below depend only on the module's resolved `languageVersion`; the
  session-level `minLanguageVersion` field records a preferred floor
  and is not consulted by the visibility-classification path in the
  watched sources.
- `IgnoreForLookupModifier`
  ([slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)
  line 279) — a separate modifier on an `InheritanceDecl` that makes
  member lookup skip that inheritance facet (the enum tag type). It is not part of the `Public` / `Internal` / `Private`
  classification but commonly mistaken for one; see
  "Interaction with `IgnoreForLookupModifier`" below.

## Rules

### Per-keyword semantics

The three keyword classifications map onto the `DeclVisibility`
levels:

- `public`: visible from any module that has imported the declaring
  module.
- `internal`: visible only inside the declaring module
  (including all of its files).
- `private`: visible only inside the declaring aggregate type
  (`struct`, `class`, `interface`, ...). A namespace is not a legal
  container for `private`: a namespace member is still a global decl
  by `isGlobalDecl`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  lines 1596-1604, true when the parent, looking through generics, is a
  namespace-like decl or a `FileDecl`), so `private` on it is rejected at the
  declaration site with `invalid-use-of-private-visibility` (see
  "Edge cases and failure modes" below).

The mapping is implemented in `getDeclVisibility`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 23217-23281 at `source_commit`): the function walks
`decl->modifiers` and returns the first `VisibilityModifier` it finds
(lines 23241-23249).

`getDeclVisibility` also implements three structural fall-throughs:

- For an `AccessorDecl` or `EnumCaseDecl`, visibility is inherited
  from the enclosing decl (the parent-walk skips them, lines
  23230-23237); if the walk runs out of parents the result is `Public`
  (lines 23238-23239).
- For a `GenericDecl`, visibility is taken from its `inner` decl
  (lines 23228-23229).
- For a generic parameter (`isGenericParam` / `GenericTypeConstraintDecl`)
  whose parent is a `GenericDecl`, visibility is the visibility of that
  generic's inner decl; any other parent, or a generic with no inner
  decl, falls back to `Default` (lines 23219-23227).

If no explicit modifier is present, the fall-back depends on where the
decl sits and on the module's language version, in this order:

1. **Slang 2026 and later: an unmodified member of an aggregate
   inherits the aggregate's effective visibility** (lines 23250-23260).
   So in

   ```slang
   internal struct S { void f(); }
   ```

   `f` is `internal` rather than picking up the module default. The rule
   is keyed off the aggregate's *effective* visibility — the result of a
   recursive `getDeclVisibility(parentAggTypeDecl)` — so it composes
   transitively through nested aggregates. The aggregate is found by
   `getParentAggTypeDecl` in
   [slang-syntax.cpp](../../../../source/slang/slang-syntax.cpp) (line
   1247), which walks every ancestor, so a local declared inside a
   member function also inherits it. It applies only when the
   module's `languageVersion >= SLANG_LANGUAGE_VERSION_2026`, so
   existing 2025 and legacy code keeps the older behaviour.
2. **A member of an interface** inherits the interface's visibility
   (lines 23261-23265); this is the rule the 2026 aggregate rule above
   was written to mirror.
3. **Otherwise** the visibility is the module's default (see below).

### Defaults by language version

`ModuleDecl::defaultVisibility` controls the implicit visibility for
members of the module that have no explicit modifier. The default
is computed in `getDeclVisibility`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 23266-23272):

```cpp
defaultVis = parentModule->languageVersion == SLANG_LANGUAGE_VERSION_LEGACY
                 ? DeclVisibility::Public
                 : parentModule->defaultVisibility;
```

The legacy language treats every unannotated decl as `public` — the
language predates the visibility system entirely, and existing code
relies on that behaviour. Modern versions default to `internal`
unless the module is marked `public` at the top:
`checkModule` flips the module-wide default to `public` when it
finds a `PublicModifier` on the `ModuleDecl`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 5504-5507, in `SemanticsDeclVisitorBase::checkModule`).

A `NamespaceDecl` with no explicit visibility modifier, and with no
enclosing aggregate or interface to inherit from, is `Public`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 23276-23279).

A function-local `VarDecl` in a free function has no enclosing aggregate
or interface, so it takes the module
default like any other unmodified decl, and `checkVisibility` runs on
it along with every other `VarDeclBase`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
line 3297 in `checkVarDeclCommon`; a local reaches that path through the
`ensureDecl(..., DefinitionChecked)` in
`SemanticsStmtVisitor::checkDeclAtDeclarationPoint`, which
`visitDeclStmt` calls; [slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp)
line 75). In a module declared
`public module M;` that makes a local `public`, so the
container-level cap below rejects a local whose type is `internal`
with `use-of-less-visible-type` (30604) even though the name never
escapes the function:

```slang
public module M;
internal struct Counter { internal int n; }
void f() { Counter c; }   // error 30604
```

The field is annotated `internal` on purpose. Leave it bare and, in a
module below language version 2026, the
same public-module default that makes `f`'s local `public` also makes
`n` `public` (from 2026 a bare field inherits `Counter`'s visibility
instead), which puts a `public` member inside an `internal` struct
and draws a second, unrelated error — `E30601`
(`decl-cannot-have-higher-visibility`, *"'n' cannot have a higher
visibility than 'Counter'."*) — on the field line. Spelling the field
`internal` keeps the snippet about the function-local rule it is meant
to illustrate.

### Where visibility is filtered

```mermaid
flowchart TB
  raw["raw LookupResult"]
  item["for each item"]
  pred["isDeclVisibleFromScope"]
  keep["keep"]
  drop["drop"]
  result["filtered LookupResult"]
  cand["overload candidate, separate path"]
  ovld["TryCheckOverloadCandidateVisibility"]
  verdict["candidate dropped, or DeclIsNotVisible in ForReal mode"]
  raw -->|"filterLookupResultByVisibility"| item
  item --> pred
  pred -->|"visible"| keep
  pred -->|"not visible"| drop
  keep --> result
  cand --> ovld
  ovld -->|"same isDeclVisibleFromScope predicate"| verdict
```

Visibility is consulted at two distinct points:

1. **Lookup boundary.** Most lookup call sites in the checker pass
   their result through `filterLookupResultByVisibilityAndDiagnose`
   ([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
   lines 1324-1346), which delegates the filtering itself to
   `filterLookupResultByVisibility` (lines 1311-1322; it returns its
   input unchanged when the visitor has no `m_outerScope`). If lookup
   returned candidates but all are
   filtered out, the function emits diagnostic
   `decl-is-not-visible` (`Diagnostics::DeclIsNotVisible`,
   [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua) 30600) and reports the *first* offending
   decl. In language-server mode it intentionally returns the
   unfiltered result so completion can keep operating.
2. **Overload resolution.** `TryCheckOverloadCandidateVisibility`
   ([slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)
   lines 266-288) is the last stage of `TryCheckOverloadCandidate`
   (line 1438, after the arity, fixity, type, direction and constraint
   stages, setting `Status::VisibilityChecked`) and is repeated in
   `CompleteOverloadCandidate` (line 1635). It accepts the candidate
   unchanged when the context has no `sourceScope` or the item has no
   decl-ref. Otherwise, when the predicate fails it drops the
   candidate, and only in `ForReal` mode also emits the same
   `DeclIsNotVisible` diagnostic.

Both call sites delegate to the same predicate,
`SemanticsVisitor::isDeclVisibleFromScope(declRef, scope)`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
lines 1181-1309). The predicate computes the decl's
`DeclVisibility` and dispatches:

- `Public` — always visible.
- `Internal` — visible iff `getModuleDecl(decl)` equals
  `getModuleDecl(scope)`; that is, the requesting scope is part of
  the same module.
- `Private` — visible iff one of the requesting scope's parents is
  the enclosing aggregate (or namespace) that owns the decl, or the
  scope is inside an `ExtensionDecl` whose target type matches.
- Anything else, including an `Internal` decl from another module —
  not visible (the closing `return false`). `Default` is an alias of
  `Internal`, so it takes the `Internal` branch.

The same predicate is also used outside the two filters above: by the
"did you mean" suggestion search (`findClosestInScopeName`, line 5533),
by interface-requirement collection for member-access diagnostics
(`doesInterfaceDeclareUsableRequirement`, line 9127, and
`collectVisibleInterfacesDeclaringRequirement`, line 9179), and by
`doesNameResolveToDecl` (line 9200).

The private-access check walks the requesting scope's parent chain
looking for the parent aggregate (lines 1195-1209). When the
candidate decl lives in an `ExtensionDecl`, the predicate also
resolves the extension's target type and checks for type equality
with the enclosing aggregate of the requesting scope
(lines 1224-1306, via the local `ContainerTargetTypeResolver`). This
lets `extension S { private foo() {...} }`
work when called from inside `S` itself or from another extension on
`S` — even a generic extension that specializes to `S`.

That specialization step is guarded on the *candidate's* own
container being an `ExtensionDecl` (lines 1284-1289), so it does not run in
the other direction. A `private` member declared in the body of a
generic type `G<T>` is compared as the type's own default decl-ref
`G<T>` against the requesting extension's target type, and that
comparison fails: such a member is reachable from neither
`extension G<int>` nor `extension<T> G<T>`. Only a `private` declared
in an `extension` crosses instantiations.

### Container-level cap

`SemanticsVisitor::getTypeVisibility`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
lines 1172-1179) computes the visibility of a `Type` by taking the
minimum of the underlying decl's visibility and the visibilities of
its declaration-reference generic arguments — the recursion only
descends into an argument that is itself a `DeclRefType`. This is what makes
`HashMap<String, InternalKey>` effectively `internal` even if
`HashMap` is `public`.

The recursion itself lives in the static helper `_getTypeVisibility`
(lines 1145-1170), which threads a
`ShortDictionary<Type*, DeclVisibility>` memo through the walk;
`getTypeVisibility` is the thin entry point that creates that
dictionary per query. Only completed results are cached.
The reason is that generic arguments frequently repeat: in
`Pair<T, T>` both arguments point at the same canonical type DAG, so
the second edge should reuse the visibility already collected through
the first instead of traversing the whole DAG again.

`SemanticsVisitor::checkVisibility`
([slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)
lines 2373-2438) enforces the converse: a decl cannot reference a
type that is *less* visible than itself. Violations produce
diagnostic `use-of-less-visible-type`
(`Diagnostics::UseOfLessVisibleType`, code 30604). The same
function also enforces that a decl's visibility cannot exceed that of
the nearest enclosing `AggTypeDeclBase`. The search starts at the decl
itself, so an aggregate declaration is compared against itself and the
cap effectively bites on non-aggregate members. Violations produce
`decl-cannot-have-higher-visibility` (code 30601). The function returns
early for an `AccessorDecl` and for any decl that is not a `VarDeclBase`,
`CallableDecl`, `PropertyDecl`, `AggTypeDeclBase` or `TypeDefDecl`
(lines 2373-2406).

### Interaction with `extern` and `export`

The `extern` and `export` modifiers are distinct from the
`public` / `internal` / `private` classification and do not change a
decl's `DeclVisibility`; instead they affect lookup or cross-module
reachability directly:

- `ExternModifier`
  ([slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)
  line 111) and `ExtensionExternVarModifier` (line 262) mark `extern`
  decls. `DeclPassesLookupMask`
  ([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp)
  lines 40-53) always drops `ExtensionExternVarModifier` decls from
  lookup, and drops an `ExternModifier` member whose parent is an
  `ExtensionDecl`, so those `extern` members are hidden from lookup
  regardless of their visibility keyword. The user-level spelling is
  an `extern` member of an `extension`, and the resulting failure is
  an absence rather than a rejection:

  ```slang
  extension S { extern int extra; }        // error 31143 here
  int test(S s) { return s.extra; }        // error 30027 here
  ```

  The declaration itself is reported as
  `missing-original-defintion-of-extern-decl` (31143), and the use
  site as `no-member-of-name-in-type` (30027, "member not found") —
  not the `decl-is-not-visible` (30600) that a visibility rejection
  would produce.
- `HLSLExportModifier` (line 123) is the bare `export` keyword modifier
  and marks a decl for linkage;
  it records linkage intent and does not by itself raise a decl's
  `DeclVisibility`.
- `ExportedModifier` (line 153) on an `import` controls transitive
  cross-module reachability: `isModuleReachableViaExportedImports`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  lines 9535-9562) only follows imports marked with `ExportedModifier`,
  so an `__exported import` re-exports the imported module while a
  plain `import` does not.

The non-transitivity of a plain `import` also has to be enforced on the
*scope chain*, not just on the import graph. When a module is imported,
`importModuleIntoScope` splices the imported module's scopes onto the
importing scope's sibling chain — and a transitively imported foreign
module's `FileDecl` can land on that chain too. The predicate
`isOwnModuleOrIncludedFileScope`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
lines 326-342) decides which of those siblings belong to a module's own
re-export surface: the module's own scope, or a `FileDecl` whose
`parentDecl` is that same module (an `__include`d file). The
`parentDecl == moduleDecl` conjunct is load-bearing — a foreign
module's `FileDecl` points at *that* module, so it is dropped no matter
how it arrived on the chain. Dropping the conjunct would re-export
those foreign files and silently make plain `import` transitive (see
shader-slang/slang#11443). It also drops `using`-spliced namespace
siblings, so a primary file's `using namespace Foo;` does not leak
through an `import`. The predicate is declared in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h) line
88, and is applied by `SemanticsVisitor::importModuleIntoScope`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
line 18700, in the function starting at 18663) and again in
`ComponentType::_getOrCreateScopeForLegacyLookup`
([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp)
line 3993), the scope behind name-based type and entry-point lookup
through the API.

### Generic parameters, accessors, and synthesized members

`getDeclVisibility` collapses the visibility of generic parameters
into the visibility of the generic's `inner` decl, so a generic
parameter is never independently more or less visible than the decl
it parameterizes. Similarly, `AccessorDecl` and `EnumCaseDecl` get
their visibility from their parent.

Synthesized members (interface-requirement witnesses, derivative
implementations, ...) are in most cases assigned a visibility at their
synthesis site rather than relying on the module default; where that
assignment is conditional, the synthesized decl falls through to the
normal effective/default classification described above. The requirement-
satisfaction synthesizers call
`addVisibilityModifier(synthesized, Math::Min(parentVisibility, requirementVisibility))`
when the satisfied requirement carries an explicit visibility
modifier
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 7618-7621 for a synthesized method, 8663-8666 for a property,
and 9084-9087 for a subscript), so a synthesized member is never more
visible than either its parent or the requirement it satisfies.
Synthesized differential types and fields propagate the parent
member's visibility directly via
`addVisibilityModifier(decl, getDeclVisibility(parent))`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 4202-4203, 4226-4227, 4285-4286 and 4351-4352); the synthesized
differential struct and its type alias in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
lines 877-878 and 911 do the same. `addVisibilityModifier` itself is a
`SemanticsVisitor` method at
[slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
line 4040; it adds a `PublicModifier`, `InternalModifier` or
`PrivateModifier` and does nothing for any other value. A synthesized
default constructor or member-initialization constructor takes the
struct's own visibility: `createCtor` takes a `ctorVisibility` argument
(line 3339, applied at line 3368) and its callers pass
`getDeclVisibility(structDecl)` (lines 21747 and 21877); a member whose
visibility is lower than the constructor's is left out of the
constructor's parameter list (line 21656).

### Synthesized extensions: two visibilities, not one

Synthesis that produces an `extension` rather than a plain member needs
two visibility decisions, because an extension is hoisted to module
scope where `private` is not a legal visibility at all. Consider a
private member function annotated `[Differentiable]`: the derivative is
synthesized as an extension on the function-as-type, but that extension
cannot itself be `private`.

`getSynthesizedExtensionVisibility`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
lines 9508-9533) resolves this by returning a
`SynthesizedExtensionVisibility` pair (lines 9470-9474) with separate
`extensionVisibility` and `memberVisibility` fields. `Public` and
`Internal` targets map both fields to themselves; a `Private` target
maps the extension to `Internal` — module-visible, so synthesis can
attach to it — while keeping the synthesized members `Private` so the
callable's API surface is unchanged. Any other value is
`SLANG_UNEXPECTED`; `Default` is an alias of `Internal`, so it takes the
`Internal` case.

The related helper `getMoreRestrictiveVisibility` (lines 9476-9506)
computes the pairwise minimum of two visibilities after normalizing
`Default`, and is the shared spelling for "no more visible than either
of these two things." The extension is applied through `extendContainerDecl`
(line 9411, which calls `addVisibilityModifier` on the new extension at
line 9431) and the synthesized members through `addSynthesizedFunc` (line
15801; line 15819). Call sites include the auto-diff requirement
synthesizer, which gives its function the `memberVisibility` of the
target (lines 9842-9849), and the custom-derivative translation, which
feeds the pair `getMoreRestrictiveVisibility(primal, user-supplied
derivative)` (lines 20927-20935 forward, 21003-21006 backward), so a
derivative member is never more visible than the primal function or the
derivative function that supplies it.

### Interaction with `IgnoreForLookupModifier`

`IgnoreForLookupModifier`
([slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)
line 279) marks an `InheritanceDecl`, not an arbitrary decl. The only
producer in the watched paths is the enum tag-type inheritance decl
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
line 13330). Member lookup skips facets whose declared subtype witness
names such a decl, so the tag type does not appear as a base
([slang-lookup.cpp](../../../../source/slang/slang-lookup.cpp) line
461); this is not a general filter, and visibility rules never see
that facet.

## Edge cases and failure modes

- **`public` inside an `internal` struct.** `checkVisibility`
  rejects this: the diagnostic `decl-cannot-have-higher-visibility`
  fires at the inner decl
  ([slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)
  lines 2433-2437).
- **`private` on a decl that is not a member of a type.** The
  diagnostic `invalid-use-of-private-visibility`
  ([slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
  2625, code 30603) fires at the declaration site
  ([slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)
  lines 2236-2265) for three shapes, not just the top-level one:
  a module- or file-scope decl, a member of a `namespace` (both are
  `isGlobalDecl`), and an interface requirement. The message says
  "is not a member of a type", which is the accurate statement of
  the rule — a container is not enough, it has to be a type.
- **Visibility modifier on a node that does not accept one.** The
  diagnostic `invalid-visibility-modifier-on-type-of-decl`
  ([slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
  line 2639, code 36005) fires when the user marks a namespace
  `internal` (lines 2266-2273 of
  [slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)).
  `private namespace X` does *not*
  reach it: the `isGlobalDecl` test above runs first and returns, so
  a namespace — top-level or nested — is reported as 30603 instead.
  (A repeated modifier such as `public public ...` is instead caught
  by the conflict-group check, which emits `duplicate-modifier`
  (`Diagnostics::DuplicateModifier`, code 31202)
  from
  [slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)
  line 2575.)
- **Less-visible type in a more-visible signature.** A `public
  func foo(x: InternalT)` produces `use-of-less-visible-type`
  (code 30604).
- **A name found by lookup but filtered out.** `DeclIsNotVisible`
  (code 30600) is emitted by
  `filterLookupResultByVisibilityAndDiagnose` when the original
  `LookupResult` was non-empty but the filter removed every
  candidate; the diagnostic names the first removed decl. Language-
  server mode returns the unfiltered result so completion still
  shows the candidate.
- **Cross-language-version import.** A legacy-language module
  imports a modern-language module: the modern module's decls are
  still classified by their *own* `defaultVisibility`. The legacy
  caller's scope is checked by `getModuleDecl(scope)` for
  `Internal`-level access — being in a different module makes
  `internal` decls invisible, regardless of the caller's language
  version. The legacy module's *own* decls are seen as `public` by
  any caller because the legacy default is `public`.
- **`extension` on a generic type.** A `private` member declared in
  an `extension` is reachable from another extension only if the
  target types are equal or `applyExtensionToType` successfully
  specializes the candidate's generic extension to the requesting
  scope's container type
  ([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)
  lines 1281-1289). Distinct concrete specializations such as
  `extension G<int>` and `extension G<float>` do not see each
  other's `private` members. A `private` member of the generic
  type's *own* body gets no specialization and is rejected with
  `DeclIsNotVisible` from any extension on the type.
- **Synthesized derivative members.** When auto-diff synthesizes a
  derivative as an `extension` on a function-as-type whose owner is
  itself a struct member, `isDeclVisibleFromScope` resolves the
  parent aggregate recursively so that the derivative inherits the
  ordinary member's visibility scope (lines 1239-1258).
- **Re-exporting a non-exported decl through an alias.** A `using`
  declaration in Slang only brings a *namespace-like* container into
  scope — a namespace or a module, since modules are namespace-like —
  `SemanticsDeclScopeWiringVisitor::visitUsingDecl`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  lines 18960-19022) requires its argument to resolve to a
  `NamespaceDeclBase` and otherwise emits `ExpectedANamespace`
  ([slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
  line 1635, code 30061) — so `using SomeStruct;` is
  rejected with "expected a namespace" rather than aliasing the
  struct. A `using` does not re-export an individual `internal`
  member, so there is no `using`-specific visibility rejection in
  the watched paths. The
  source-backed rejection path for a derivative that exposes a less-visible
  decl is `validatePublicCallableOperandVisibility`
  ([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
  lines 9593-9647). It is called from the header visitors for a
  `FuncAliasDecl` (`visitCallableDecl`, line 16873) and for a
  `SynthesizedFuncDecl` (`visitFunctionDeclBase`, line 16882), and it
  checks only `Public` decls. For the alias branch (line 9606), or for each
  operand of the synthesized function (line 9621), it rejects a target
  that is not `Public`, or whose module is reachable only through a plain
  (non `__exported`) `import`
  (`isDeclReachableViaExportedImports`), with
  `public-custom-derivative-uses-non-exported-import`
  (`Diagnostics::PublicCustomDerivativeUsesNonExportedImport`, code
  31162, `slang-diagnostics.lua` line 3189).

  A `FuncAliasDecl` is never written by hand: it is synthesized by
  `translateFwdDerivativeAttributeToAD2` (line 20909) for a
  `[ForwardDerivative(...)]` attribute, which builds an extension on
  the primal-function-as-type and gives it an alias member naming the
  supplied derivative (line 20954). The backward translation
  (`translateBwdDerivativeAttributeToAD2`, line 20990) builds
  `SynthesizedFuncDecl` members instead. The rejected shape is therefore
  a `public` primal whose derivative comes from a plainly imported
  module:

  ```slang
  import "helper";                    // not `__exported import`

  [ForwardDerivative(helper_fwd)]     // error 31162 here
  public float f(float x) { return x * x; }
  ```
- **`IgnoreForLookupModifier`.** Not a general filter; it only makes
  lookup skip an enum's tag-type inheritance facet (see above).

## See also

- [scopes.md](scopes.md) — the scope chain that
  `isDeclVisibleFromScope` walks.
- [lookup.md](lookup.md) — the lookup algorithm whose results are
  visibility-filtered.
- [overload-resolution.md](overload-resolution.md) — overload
  candidate filtering, which calls
  `TryCheckOverloadCandidateVisibility`.
- [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
  — the semantic-checking phase that runs the visibility filters.
- [../ast-reference/modifiers.md](../ast-reference/modifiers.md) —
  per-class reference for every modifier, including the visibility
  family.
- [../ast-reference/declarations.md](../ast-reference/declarations.md)
  — per-class reference for `ModuleDecl`, `NamespaceDecl`, and the
  aggregate-type decls.
- [../glossary.md](../glossary.md) — entries for
  `visibility modifier`, `decl-ref`, `name resolution`.
