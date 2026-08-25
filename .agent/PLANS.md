<!--
SPDX-FileCopyrightText: The Khronos Group, Inc.
SPDX-License-Identifier: CC-BY-4.0
-->

# Slang ExecPlans

An ExecPlan is a self-contained implementation plan for one bounded piece of long-running work. It
is both the execution contract for the agent doing the work and the hand-off document for the next
agent or human who continues it. A reader should be able to start from the repository checkout and
the ExecPlan alone, understand the intended behavior, perform the work, and prove that it works.

## When to Use an ExecPlan

Create an ExecPlan before implementation when any of these conditions apply:

- the work is expected to span multiple focused sessions;
- the change crosses several compiler stages or subsystems;
- a prototype or compatibility experiment must succeed before the final design is known;
- the task has several independently testable milestones; or
- another agent or contributor may need to resume the work without the original conversation.

Do not create an ExecPlan for a small, single-session edit with an obvious validation command.

## File and Lifetime Policy

This file defines the reusable planning standard and is a repository artifact. Active ExecPlans are
working logs and follow the repository rule that working logs are not committed.

- Store an active plan at `issue-<topic>/plan.<slice>.md`. Existing `.gitignore` rules keep that
  path out of commits.
- Use one active plan per bounded slice. Keep a durable program-level architecture document under
  `docs/design/` instead of stretching one ExecPlan across an open-ended backlog.
- Update the active plan at every meaningful stopping point. Never rely on chat history as the only
  record of progress or a decision.
- Distill stable architecture and settled decisions into the relevant design document.
- Distill durable test/capability status into a checked-in manifest when the project defines one.
- Distill the working narrative, rejected alternatives, and input-shape audits into the PR's
  required five-part description.

If maintainers explicitly decide to commit an ExecPlan, record that exception in both the plan and
the repository instructions before committing it.

## Required Qualities

Every ExecPlan must be:

- **Self-contained.** Define project-specific terms and summarize every external contract needed
  for the slice. Link to authoritative references, but do not require the reader to reconstruct the
  design from those references.
- **Outcome-focused.** Start with the behavior someone can observe after the slice lands.
- **Repository-specific.** Name exact files, types, functions, commands, and expected outputs.
- **Incremental.** Prefer additive prototypes and parallel old/new paths that keep the repository
  buildable and provide a rollback.
- **Evidence-driven.** Each implementation change needs a failing test, an explicit contract, or a
  measured prototype result that demonstrates why the change exists.
- **Principled.** Apply the input-shape audit and producer-side/root-cause methodology from
  `AGENTS.md`; do not retain a fallback or special case merely because it makes one test pass.
- **Restartable.** Progress and decisions must describe the actual repository state, not the state
  the author expected to reach.

## Required Living Sections

Every plan must contain and maintain these sections.

### Purpose and Observable Result

Explain the user- or developer-visible capability the slice adds and the shortest way to observe
it working.

### Progress

Use timestamped checkboxes. Split partially completed work into completed and remaining portions so
the list always reflects reality.

### Surprises and Discoveries

Record unexpected behavior with concise evidence such as a command, diagnostic, test result, or
code trace. Do not record speculation as fact.

### Decision Log

For every material choice, record the decision, rationale, date, and author. State what evidence
would cause the decision to be revisited.

### Outcomes and Retrospective

At each major milestone, summarize what now works, what remains, and what the result teaches the
next slice. Complete this section before declaring the plan finished.

## Required Execution Sections

In addition to the living sections, include:

- **Context and current pipeline:** the motivating example and the producer-to-consumer code trace
  that reaches the code being changed;
- **Scope and non-goals:** explicit boundaries that prevent the slice from silently expanding;
- **Architecture and invariants:** the representation each stage owns and the contracts consumers
  can rely on;
- **Interfaces and dependencies:** public/internal interfaces to add or change, external library
  contracts, version rules, and artifact shapes;
- **Milestones:** ordered, independently verifiable steps, including exact files and the behavior
  added by each step;
- **Validation and acceptance:** the smallest focused tests, broader regression checks, expected
  diagnostics/output, and any environment requirements;
- **Failure and recovery:** how to diagnose partial failure, how reruns remain safe, and how to
  disable or remove an experimental path without disturbing the established path; and
- **Artifacts and hand-off:** generated evidence to retain locally and durable information to
  distill into repository documents and the PR description.

## Prototype Milestones

Use a prototype milestone when an external API, serialization format, optimizer behavior, or input
shape is uncertain. A prototype must state:

1. the isolated hypothesis;
2. the smallest implementation or fixture that tests it;
3. the exact command and expected evidence;
4. promotion criteria for incorporating the approach; and
5. discard criteria and cleanup instructions.

A successful prototype is evidence, not permission to keep throwaway architecture. Before
promotion, identify which code becomes production code, which code is replaced, and which artifact
or test preserves the learned contract.

## Multi-Agent Coordination

One lead owns the active ExecPlan and integration. Delegate only bounded tasks with non-overlapping
write ownership, such as external-contract research, test inventory, or review. The lead must read
and integrate each agent's evidence into the plan; agent messages are not durable project state.

When agents explore competing designs, keep both experiments additive until the plan's promotion
criteria select one. Do not let two agents independently modify the same compiler boundary.

## Validation Discipline

Validation must be proportional to risk and layered from the smallest contract to the established
test suite. A plan is complete only when all required acceptance evidence exists, not merely when
the code builds.

For compiler work, the plan should normally include:

1. a focused unit or file test for the new contract;
2. negative coverage for diagnostics or invalid representations;
3. the smallest end-to-end test that crosses the changed boundary;
4. relevant regression tests for the established path; and
5. performance or output-quality evidence when those properties motivate the work.

Record exact commands for the active platform. Follow `AGENTS.md` for host-tool selection and build
setup; do not substitute generic platform commands.

## Minimal Skeleton

Use this ordering unless the slice has a strong reason to add more sections:

```md
# <Action-oriented slice title>

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds.

## Purpose and Observable Result

## Progress

## Surprises and Discoveries

## Decision Log

## Outcomes and Retrospective

## Context and Current Pipeline

## Scope and Non-Goals

## Architecture and Invariants

## Interfaces and Dependencies

## Milestones

## Validation and Acceptance

## Failure and Recovery

## Artifacts and Hand-Off
```
