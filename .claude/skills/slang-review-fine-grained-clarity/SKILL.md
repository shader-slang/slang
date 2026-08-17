---
name: slang-review-fine-grained-clarity
description: Review Slang changes for fine-grained clarity. Use whenever reviewing PRs or diffs for code quality or correctness. Produces candidate review comments in a markdown file.
argument-hint: "<pr-number-or-diff-path>"
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Write
  - Edit
---

# Slang Fine-Grained Clarity Review

Perform a detailed name/comment/definition consistency pass over a Slang PR.
This is a "nit-pick" review pass, but the nits are not cosmetic: unclear names, muddy
comments, and mismatched contracts are evidence that the code is not clear enough to be trusted.

The expected standard is exhaustive scrutiny of touched code. Consider every changed or newly
relevant file, type, function, declaration, comment, branch, expression, and line. Generate a
candidate unless you can confidently state that a careful reader would find the code/comments
obviously necessary, correct, and internally consistent. One candidate may cover a cluster of
related lines, but do not skip a concern merely because it is fine-grained or pedantic.

Do not directly post comments to PRs; write candidate comments to a markdown file under
`tmp/review-candidates/`.
If the caller provides an existing candidate file, append new candidates to that file instead
of creating a fresh file. This is useful for sequential workflows. When multiple review passes
may run concurrently, write separate raw files and let `slang-review-consolidate-candidates`
merge them.
When appending, inspect existing candidate IDs and continue numbering for this skill's `FG`
prefix instead of restarting at `FG001`.
If requested to post comments, write the candidates to a file, then run the consolidation and
filtering skills before posting the filtered results.

## Inputs

Prefer reviewing from files:

```bash
mkdir -p tmp/review-candidates
gh.exe pr diff <number> -R shader-slang/slang > tmp/pr-diff.patch
gh.exe pr view <number> -R shader-slang/slang --json files -q '.files[].path' > tmp/pr-files.txt
```

Read `CLAUDE.md`, `AGENTS.md`, `tmp/pr-files.txt`, `tmp/pr-diff.patch`, and the PR-version
source files. For large files, use grep first and then read focused ranges.

## Output File

If the caller provides an output file, append candidates there.
Otherwise, write candidates to:

```text
tmp/review-candidates/pr-<number>-fine-grained-clarity.md
```

If no PR number is known, use:

```text
tmp/review-candidates/fine-grained-clarity-review.md
```

## Scope Tagging

Generate candidates liberally and tag each one:

- `Scope: Direct` for changed lines, new declarations, renamed fields, new comments, or
  functions/types whose body changed.
- `Scope: Contextual` for existing declarations whose comment, name, or contract is made
  misleading by the PR's changes.
- `Scope: Probably out-of-scope` for real clarity issues that are not clearly owned by the PR.

Do not use scope tagging to suppress candidates during this pass. A later scope filter decides
which candidates are fair to post.

## Coverage Standard

This pass should leave an audit trail of scrutiny, not just the most interesting few comments.
For each changed source file, the final raw candidate set should make it plausible that every
touched declaration and non-trivial changed line was inspected. The outcome for each touched
region should be one of:

- a candidate records the missing clarity, uncertainty, or inconsistency;
- a nearby candidate covers the same conceptual issue;
- the code and comments are obviously sufficient, with no lingering question about necessity,
  correctness, naming, or contract.

When in doubt, write the candidate with lower confidence. Do not rely on later memory or final
summaries to recover skipped fine-grained concerns.

## Candidate Format

Use this exact structure:

````markdown
### FG001: Short Title

- Status: Proposed
- Confidence: High | Medium | Low
- Scope: Direct | Contextual | Probably out-of-scope
- Category: type contract | function contract | naming | terminology | comments | invariant | control flow | API shape
- Location: `path:line`
- GitHub link:

Context:

```cpp
<small PR-version snippet>
```

Proposed comment:

> <comment text suitable for GitHub after filtering>

Notes:
<why this candidate was raised, uncertainty, related candidates>
````

Include a code block for the relevant PR-version context. The GitHub link is useful but not
sufficient.

Review comments should identify the missing clarity or correctness argument. Do not merely
say "add a comment here." Explain what the reader cannot confidently infer: the contract,
invariant, case analysis, termination argument, precondition, provenance, or reason a line is
necessary and correct.
Lead with the missing understanding or uncertainty that blocks confident review. Only specify
the exact remedy when there is only one credible fix; otherwise, ask the author to make the
needed contract, invariant, naming distinction, or correctness argument clear, without
dictating the implementation or prose.

## Implementation vs. Documentation Comments

Understand the distinction between implementation and documentation comments:

- Documentation comments, whether or not they use a `///` convention or similar, are those comments attached to declarations/files/modules for the purpose of explaining an API contract, concept, or abstraction to a user of that API. They should be written in a style that is clear and informative to a user of the API, and they should not surface implementation details that are not relevant to a client of the API.

- Implementation comments are all comments that are not documentation comments. They are meant to explain the intent, approach, and correctness argument of a particular implementation to a future maintainer. They may include details about the implementation that would not be relevant or appropriate for a documentation comment.

Flag:

- Cases where documentation comments include implementation details.
- Cases where code has sufficient documentation comments, but is lacking in implementation comments that allow a future maintainer to understand the intent, approach, and correctness argument of the code.

## File Checklist

Apply to every touched source file, including header files.

Ask:

- Does each source file have a file-level implementation comment that explains the concern (problem, task, data structure, etc.) that the file pertains to, along with the overall approach or decomposition used to address that concern? If not, that is an issue.

- Is the overall order of declarations/definitions in the file clear and logical, and consistent with the organizational principles stated in the file-level comment (problem decomposition or approach)?
  Code should be organized to tell a clear story at the file level, as much as is possible within the constraints of C++ language rules.

  Changes to an existing file should respect existing organization, extending or adapting it as needed.
  If existing code is lacking in clear organization, a PR may elect to improve it, but must not make it worse.

  All new code files or significant additions to existing code files should have clear structural organization that supports the clarity of the file's overall story and approach.

- Does the file use Markdown-style conventions like headings to support the organizational structure of the file and make it easier to navigate?
  Use of Markdown-style conventions is highly recommended but not strictly required, if structure is clear without them.
  If a PR uses other notational conventions to delineate structure, review feedback should request that they be changed to Markdown-style conventions for consistency and readability.

  As a guiding principle: if the content of a code file were "flipped" so that the implementation comments became ordinary text in a Markdown file, with the remaining lines becoming interspersed code blocks, the resulting Markdown file should ideally read like a compelling chapter of a book, describing the relevant problem domain and solution approach taken in a pedagogical and engaging way.

## Function Checklist

Apply to every function, method, constructor, etc. introduced or touched by the PR.

Every non-trivial function should have a caller-visible contract. That contract may be
obvious from the name and signature for simple accessors and similarly direct routines, such
as `float Circle::getRadius()`. Otherwise, require a declaration-level comment that explains
what the function does, returns, requires, preserves, mutates, or may fail to do.
A definition in a `.cpp` file may rely on a commented declaration in a header file, but a
caller-visible contract must exist somewhere.

Ask:

- What does the name imply the function does, returns, or tests?
- What does the declaration or documentation comment say it does, returns, requires,
  preserves, mutates, or may fail to do?
- What does the signature imply, including parameter names, parameter order, out-parameters,
  return type, and ownership or mutation?
- What does the body actually do, return, and mutate?
- Are name, comment, signature, and body mutually consistent?
- Does the name match whether the routine is function-like or procedure-like?

Naming rules:

- Function-like routines, including pure functions, accessors, getters, queries, and
  predicates, should usually be named around the value or proposition they compute, optionally
  with helper verbs such as `get`, `find`, `calc`, or `calculate`.
  - Truly pure functions may be named simply with a noun phrase for what they return e.g., `area(rectangle)`.
  - Most function-like routines should use `get` as a prefix: e.g., `getDominators(graph, node)` or `getBestCandidate(args)`.
  - Function-like routines that perform an expensive computation, such that callers should be mindful of the cost, should use `calc` or `calculate` as a prefix: e.g., `calculateDominators(graph, node)` or `calcBestCandidate(args)`.
  - Functions that perform some kind of search and which return an optional/pointer and use nullopt/nullptr to represent failure should be named with `find`: e.g., `findBestCandidate(args)`.
  - Functions that test a boolean condition should be named as a predicate or proposition, starting with a verb like `is`, `does`, `has`, etc.: e.g., `isValid(candidate)` or `hasNonZeroElements(matrix)`.

- Procedure-like routines, including routines called partly or wholly for their side effects, should
  use a verb phrase that states what they do: e.g., `addEdge(graph, edge)`, `removeNode(graph, node)`, `emitCode(generator, ir)`, `checkCandidate(candidate)`.
  - Procedures that return a boolean success/failure flag should be named with `try` or `check` to indicate that the caller should inspect the result: e.g., `tryAddEdge(graph, edge)` or `checkCandidate(candidate)`.
  - Procedures which only perform the stated operation conditionally should include the condition in the name: e.g., `addEdgeIfValid(graph, edge)` or `emitCodeIfNeeded(generator, ir)`.

In all cases, prioritize semantic clarity at call sites.
Brevity does not justify ambiguity; if making the name clear means it is ugly and unwieldy, that serves as a code smell that can motivate and guide subsequent refactoring to improve the abstraction.

For static methods, consider how call sites will read when qualified with the type name.
E.g., `Matrix::rotationAroundAxis(axis, angle)` or `Image::createFromFile(filename)` both read clearly as factory functions, but `Buffer::roundUpToAlignment(size, alignment)` is awkward and clearly a utility/helper function that doesn't justify why it should be a static method.

Flag:

- Procedure-like routines whose names imply they are function-like, or vice versa.
- Boolean returns whose `true` and `false` meanings are not obvious.
- `try`, `find`, `get`, `match`, `map`, `convert`, or `solve` names without clear success and
  failure contracts.
- Declaration comments that explain a function's role (e.g., "helper function to...") or reason for existence, rather than directly stating what it does or computes.

## Function Body Checklist

Apply to every line of code introduced or touched by the PR that is within a function (constructor, etc.) body.

The hard requirement is not a particular density of comments. The hard requirement is that a
reader can confidently understand why every line exists and why it is correct as written.
If that understanding requires too much time or effort, the code is still not clear enough.
Sometimes one clear comment can justify ten or more straightforward lines that follow it;
sometimes one subtle line justifies a long explanatory comment. Treat missing or sparse
comments as evidence to scrutinize, and raise a candidate only when the code and existing
comments do not make the intent, necessity, or correctness argument clear enough.

Ask:

- What is the problem or task at hand, and what is the approach or decomposition being used?
  Do the comments clearly articulate a strategy (e.g., case analysis, iterate to convergence, etc.) in a way that a future maintainer could understand and trust is correct?
- Does every non-trivial function body have enough implementation commentary to explain the
  overall strategy or approach being taken?
  Truly trivial functions where the name and signature fully explain and justify the body can
  go without a strategy comment.
- Does the code that follows a comment clearly implement the strategy the comment describes?
  Would it be clear what the code does without the comment? If not, the *code* should be refactored to be more clear, so that it obviously does what the comment says; the comment should not be made more complicated to work around unclear code.
- Does the function ever have 5+ uninterrupted lines of code without a comment? Treat this as
  a scrutiny trigger, not an automatic finding.
  Is it actually obvious from nearby code and preceding comments why each of those lines of
  code is justified, correct, and necessary? If not, a reader of the code can no longer trust
  the author's intent and correctness argument.
- Do conditional branches, loops, and early exits have enough explanation, either locally or
  from an enclosing comment, to make it clear why the control-flow split is needed and why the
  branch condition is exactly the right one?
  Give control flow stricter scrutiny than straight-line code, because it carries more of the
  correctness argument.
  If the branch is meant to handle a corner case, does the comment explain when/how that case can actually arise, and justify why the method of handling it (e.g., early exit, exception, etc.) is provably the right one?
  Is the branch condition expression itself clear and self-explanatory? If not, prefer to add a helper routine with a clear and correct name for the condition, rather than adding a comment to explain an opaque condition.
- Is every use of nested control flow constructs clearly necessary and justified?
  Could the intent of the code be made more clear by refactoring, either to reduce nesting (by putting early-out cases before the main path), or to extract helper functions with clear names that explain the purpose of the code?
  If better clarity is possible, it should be pursued rather than letting the author slide with sub-optimal clarity.
- Is every local variable or temporary named in a way that clearly describes what it is (either as a noun phrase, or a proposition in the case of a boolean)? If not, that is an issue.
  Extremely narrowly-scoped variables can sometimes use single-letter names (e.g., for loop variables), but when a loop grows beyond 10 lines or two levels of nesting, more meaningful names are warranted.
- Are there cases where a large or compound expression could be made more obviously correct by breaking it up into multiple statements each declaring a well-named variable for a given sub-expression? If so, the refactoring should be pursued to improve clarity, rather than leaning entirely on comments to explain or justify the expression.

As a general rule, the body of a function should read as a kind of proof of the function's correctness, with the comments providing the key steps and insights in that proof.
A good proof is not just a sequence of statements that happen to be correct, but a clear, compelling and *structured* argument that the code is correct, necessary, and the best way to solve the problem at hand.

If a human or agent reading the code cannot come away understanding how it solves the problem and why that solution must be correct, then the code is not clear enough to be trusted.
The author must produce code that obviously has no bugs, rather than merely having no obvious bugs.

## Type Checklist

Apply to every `struct`, `class`, `enum`, etc. introduced or touched by the PR.

Make note of when C++ language constructs are being used to emulate a tagged-union-like type that represents a conceptual ADT (something that might just use an `enum` in Rust), and review the type as such, independent of the particular implementation approach/mechanism.

For all types, ask:

- Does it have a type-level comment? If not, flag it.
- What does the type's name imply it represents (or is, encodes, etc.)?
- What does the comment imply it represents?
- What does the body imply, considering the declared fields, accessors, methods, enum cases, tags, etc.?
- Are the name, comment, and body mutually consistent?
- Does every field or accessor make sense as a logical property, quality, or attribute of the concept?
  - For a non-boolean field/accessor named Y, it should feel natural to say "the Y of ..." when referring to the attribute/quality. E.g., `x.color` or `ifStmt.condition`.
  - For a boolean field/accessor, the name should naturally express a predicate on a value of the given type. E.g., `hasNonZeroElements` or `isValid`.

For enums, ask:

- Does every enum case read as a value, case, or adjective of the enum concept?
  E.g., `Color::Red`, `DiagnosticSeverity::Error`, `Status::Sending`, etc. are good; `Instruction::Add`, `Node::If`, `Edge::Call` are bad (e.g., "if" is not a node, even if it might be a *type* or kind of node).

For enums and tagged-union-like types, ask:

- Is the list of cases obviously exhaustive for the intended decomposition?

For tagged-union-like types, ask:

- For fields or properties that are only valid for specific cases/tags, are they private and only accessible through accessors that assert/validate that the tag is appropriate? If not, that is an issue.

- Does the type provide constructor or factory methods to ensure that instances are always created with the appropriate values for a given tag/case, validating (statically or dynamically) that the tag-specific invariants are satisfied? If not, that is an issue.

Flag:

- Types that read as bags of fields used by nearby code rather than coherent concepts.
- Public case-dependent payload fields with no constructors or checked accessors.
- Enum cases that are verbs or implementation steps rather than values or qualities.
- Type comments that enumerate fields without defining the abstraction and invariants.

Naming:

Type names should be nouns; typically "count nouns" in English.
Mass nouns are typically not good as type names (e.g., it is unclear what a type named `Water` would represent).
Abstract nouns are also allowed, but should typically be enumerations (`Severity`, `Color`) or typed scalars (`Size`, `Duration`), rather than structs or classes with multiple fields.

Type names may be qualified with adjectives, prepositional phrases, or other nouns, so long as the resulting term still reads clearly and unambiguously.
E.g., `DirectedEdge`, `NodeWithWeight`, `StatusForDisplay`, `WindowStyle`, `Mesh::Bounds`, etc.

For each type name introduced or touched by the PR, ask what a natural-language definition of the same concept would be, starting with language like "A [type name] is a [definition]."
If it is unclear how to fill in that template with a crisp and clear definition, that is a problem.
If the actual meaning or definition of the type as used in the code is inconsistent with the crisp and clear definition, then the factoring of the code is suspect.

## Terminology and Prose Checklist

For every touched name and comment:

- Identify terms of art that are being used or defined.
  This includes nouns for concepts, verbs for operations/processes, and adjectives for properties/qualities/states, etc.
- Check that terms are used consistently across all comments and code (in names of types, functions, variables, etc.).
  When terms have existing/established use in the Slang codebase or documentation, or in compiler literature more generally, check that terms are used consistently with precedent.
- Require new terms of art to be defined clearly and precisely, earlier in the file than their first use sites.
  If define-before-use is not possible, require earlier use sites to provide a descriptive explanation of the term and forward-reference the definition.
- Ask what purpose each sentence and word serves.
  How does it advance the reader's understanding of the problem, solution, or correctness argument?
- Mark prose for removal or rewrite if it is circular, evasive, decorative, or voluminous
  without helping the reader understand correctness.
- Identify places where comments speak to the process of discovery, exploration, or development rather than to the final product and the present-tense architecture and design of the system.
  It is more important for a contributor to know the current state of the world (which may include the location of technical debt, known issues, etc.) than to know the history of how the world came to be, and comments should reflect that.
- If removing a bad comment would leave code unexplained, ask for a better comment, not
  for silence.

Use the clarity bar of a serious technical paper or textbook. Code and comments should explain
the problem, relevant definitions, chosen decomposition, constraints, invariants, and trust
argument at every scale: from entire subsystems, through individual code files, and down to individual lines of code.

In clear and high-quality code the names of types, functions, etc. are primarily focused on describing the concepts, phenomena, processes, etc. of the problem domain in its natural terminology.
When the names in code are focused more on describing things in terms of the implementation details or solution domain (e.g., "visitor", "callback", "helper", "SFINAE", etc.), that is a potential code smell.
When possible, request that names, comments, etc. be revised to focus more on describing and decomposing a challenge in the problem domain, only bottoming out in the dirty details of C++ or other language quirks/mechanisms when it is strictly necessary to speak to them.

## Working Process

1. Build an inventory of changed or newly relevant types, functions, terms, etc.
2. Work file by file, in source order.
3. For each touched type, function, or free-standing comment, apply the appropriate checklist mechanically.
4. For touched statement-level logic, inspect each non-trivial branch, loop, early return,
   assertion, mutation, or other cluster of related statements.
5. Record candidates immediately in the output file with source context.
6. Perform a second coverage pass over the changed file list and diff hunks. For each changed
   file, ask which touched line/function/type has no candidate and is not obviously clear
   enough to review quickly and confidently. Add missing candidates before deduplicating.
7. Deduplicate only when two candidates ask for the same clarification at the same location.

Do not collapse a high volume of legitimate clarity candidates just because they are numerous.
Volume is expected for this pass.
