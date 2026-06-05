# Language Evolution

The Slang language evolution is driven by two processes:

- The specification proposal process.
- GitHub issue-driven process.

The specification proposal process is intended for changes to the application-visible part of Slang,
including the syntax and the standard modules. The GitHub issue-driven process is intended for fixing defects
and tracking experimental and implementation-specific parts of Slang.

The criteria for using the specification proposal process are as follows:

1. The change applies to the application-visible part of Slang. That is, syntax (grammar and the underlying
   semantics) or stable Slang standard modules; AND
2. The change is not a minor defect fix; AND
3. The change does not reflect changes in the downstream languages in an obvious way; AND
4. Any of the following applies:
   1. The change adds new syntax for Slang applications; OR
   2. The change adds new items in stable Slang standard modules; OR
   3. Existing syntax for Slang applications is modified; OR
   4. Existing syntax for Slang applications is deprecated; OR
   5. Existing standard modules for Slang applications are modified; OR
   6. Existing standard modules for Slang applications are deprecated; OR
   7. An experimental Slang standard module is matured as stable; OR
   8. A stable Slang standard module is deprecated or its status is reverted to experimental; OR
   9. The change removes support for a Slang language version.

A change is considered a minor defect fix when:

1. it fixes an obvious defect; OR
2. it clarifies existing and previously undocumented behavior.

When the criteria for the specification proposal process are not met, the GitHub issue-driven process may be used.

**Criteria interpretation examples:**

<table>
<tr>
  <th>Item</th><th>Process</th><th>Rationale</th>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/spec/blob/main/proposals/027-tuple-syntax.md">Tuple syntax (2026)</a></td>
  <td>Spec proposal</td>
  <td>New syntax to application-visible Slang, and existing syntax is modified.</td>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/spec/blob/main/proposals/020-stage-switch.md"><code>stage_switch</code></a></td>
  <td>Spec proposal</td>
  <td>New syntax to application-visible Slang (language construct).</td>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/slang/issues/11175">Deprecation of unscoped enumerations</a></td>
  <td>Spec proposal</td>
  <td>Deprecation and scheduled removal of existing syntax.</td>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/slang/issues/11216">Integer literal corner cases</a></td>
  <td>GitHub</td>
  <td>Clarification of previously undocumented corner cases.</td>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/slang/issues/11349">Fixing ambiguous generic application syntax</a></td>
  <td>GitHub</td>
  <td>Clarification of previously ambiguous grammatical rules.</td>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/slang/issues/11276">Fix handling of floating point literals</a></td>
  <td>GitHub</td>
  <td>Clarification of previously undocumented corner cases, and correction of obviously wrong existing behavior
    in the compiler.</td>
</tr>
<tr>
  <td><a href="https://github.com/shader-slang/slang/issues/11156">Fix GL_EXT_texture_shadow_lod capability requirements in depth sampling (GLSL)</a></td>
  <td>GitHub</td>
  <td>Adjust standard module behavior to better fit the downstream language.</td>
</tr>
</table>

> 📝 **Remark:** The specification proposal process is significantly heavier than the GitHub
> issue-driven process. This is the reason why fixing minor defects is exempted from the language
> specification proposal process, even if the fixes are potentially breaking changes.

## Specification Proposal Process

The specification proposal process begins with the submission of a proposal formatted according to the
<a href="https://github.com/shader-slang/spec/blob/main/proposals/000-template.md">proposal template</a>. The proposal is
then reviewed, and if approved, scheduled for implementation. The proposals are public, allowing for community
feedback.

When a proposal does not contain backward breaking changes, it may be implemented for all Slang language
versions.

When a proposal contains backward breaking changes, then:

1. The target Slang language version should be identified. This may be the language version currently being
   developed or a future language version.
2. When the proposal is implemented:
   1. The `slangc` compiler should select either the legacy or the new behavior according to the `-std`
      command-line switch or the `#language` directive.
   2. When legacy behavior is selected, the `slangc` compiler should emit warning diagnostics to ensure that
      users get notified on breaking changes in advance. There should be pointers to migration steps.
   3. When new behavior is selected, the `slangc` compiler is recommended to detect breaking legacy patterns
      when possible and point to migration steps.

**Example:**

*Unscoped enumerations* have been slated for removal, tracked by GitHub issue
#[11175](https://github.com/shader-slang/slang/issues/11175). This is a potentially significant breaking
change that requires following the specification proposal process.

Tentatively, the unscoped enumerations are going to be removed in Slang 2028. The current default language is 2025 at
the time of writing. The next steps:

1. Write a language specification proposal.
2. After the proposal has been reviewed and accepted, implement the diagnostics warning in `slangc` when
   unscoped enumerations are used. The diagnostic warning should state that unscoped enumerations are a
   deprecated feature, expected to be removed in Slang 2028. A suggestion to migrate to scoped enumerations
   should be included in diagnostics.
   - The diagnostic warning is included for all Slang language versions prior to 2028.
3. When Slang 2028 (or later) is selected, `slangc` shall trigger an error if unscoped enumerations are
   attempted to be used.
4. When support for all language versions prior to Slang 2028 is removed from `slangc`, all code related to
   unscoped enumerations may be removed from the compiler.

## GitHub Process

The GitHub process is straightforward: file a GitHub issue with "Language Maturity" issue type, and submit a pull request. See GitHub issue #[11216](https://github.com/shader-slang/slang/issues/11216) for an example.

## Checklist for Implementing Changes Related to Slang Language

1. Implement the feature.
   - For a staged syntax change, use the language version to select the behavior. See
     https://github.com/shader-slang/slang/blob/fbf41e87a3493bfe4417b3b3a92c814dde391960/source/slang/slang-parser.cpp#L5907
     for an example.
   - For a staged standard module change, use attributes
     [\[deprecated\]](../../../core-module-reference/attributes/deprecated.html) and
     [\[RemovedSince\]](../../../core-module-reference/attributes/removedsince-07.html) to mark declarations
     deprecated and removed.
2. Implement tests.
   - For language-version-dependent behavior changes, include before/after testing.
3. Update [Slang User's Guide](https://github.com/shader-slang/slang/tree/master/docs/user-guide).
4. Update [Slang Language Reference Manual](https://github.com/shader-slang/slang/tree/master/docs/language-reference).
5. Update [Slang Examples](https://github.com/shader-slang/slang/tree/master/examples) if appropriate.
6. If the specification proposal process is used, mark the related specification proposal implemented.
