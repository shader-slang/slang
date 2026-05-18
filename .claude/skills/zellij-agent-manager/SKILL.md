---
name: zellij-agent-manager
description: >-
  Manage multiple Claude Code agent sessions running in Zellij. Report a
  status summary of all sessions, deliver instructions to a specific agent
  via zellij action send-keys, and automatically monitor the agent after
  each send to detect permission prompts, clarifying questions, or
  early-idle states. Use when the user wants to oversee concurrent Claude
  agents running under Zellij, check what sessions are doing, send a
  message to an agent, or monitor for sessions that need input or are
  stuck. Linux and macOS only — Zellij is not supported on Windows.
argument-hint: "[status | send <session> <message> | monitor [interval_seconds] | new <issue_number_or_prompt>]"
allowed-tools:
  - Bash
---

# Zellij Agent Manager

Oversees multiple Claude Code sessions running in Zellij. Mirrors the tmux variant
(`tmux-agent-manager`) but uses Zellij's CLI primitives: `list-sessions`,
`action list-panes`, `action dump-screen`, `action send-keys`, `action write-chars`,
and `attach -b` (create background).

---

## Environment Detection (run first, before any other step)

```bash
bash << 'DETECT'
if ! command -v zellij &>/dev/null; then
    echo "zellij is not on PATH. Install Zellij (https://zellij.dev) and retry."
    exit 1
fi

# Refuse to run on Windows — Zellij has no first-class Windows story.
case "$(uname -s)" in
    Linux*|Darwin*) HOST="unix" ;;
    *) echo "zellij-agent-manager supports Linux and macOS only. Detected: $(uname -s)"; exit 1 ;;
esac

ZJ="zellij"
GIT="git"
echo "ZJ=$ZJ GIT=$GIT HOST=$HOST"
DETECT
```

Variables used throughout: `ZJ` (zellij binary), `GIT`, `HOST=unix`.

---

## Pane targeting helper

Zellij does not use tmux-style `SESSION:W.P` addresses. Each pane has a numeric `id`
plus an `is_plugin` flag; the `--pane-id` flag accepts either a bare integer (treated
as a terminal pane) or the `terminal_<int>` / `plugin_<int>` string form. By
convention this skill treats **the first non-plugin pane in tab 0** as the agent pane
(parallel to tmux "window 0, pane 0").

Helper:

```bash
agent_pane_for_session() {
    local session="$1"
    $ZJ --session "$session" action list-panes --json 2>/dev/null \
        | python3 -c '
import json, sys
panes = json.load(sys.stdin)
# 0.44.3 returns a flat array of panes with tab_position and is_plugin fields.
panes.sort(key=lambda p: (p.get("tab_position", 0), p.get("id", 0)))
for p in panes:
    if not p.get("is_plugin") and not p.get("exited"):
        print("terminal_%d" % p["id"]); sys.exit(0)
sys.exit(1)
'
}
```

The output is `terminal_<id>` (e.g. `terminal_0`), which all `action` subcommands
accept via `--pane-id`. If your Zellij version returns a different JSON shape, fall
back to a one-liner:

```bash
$ZJ --session "$session" action list-panes --json 2>/dev/null \
    | python3 -c 'import json,sys; ps=json.load(sys.stdin); \
                  p=next(x for x in ps if not x.get("is_plugin")); \
                  print("terminal_%d"%p["id"])'
```

---

## Commands

- `/zellij-agent-manager` or `/zellij-agent-manager status` — snapshot of all sessions
- `/zellij-agent-manager send <session-name> <message>` — deliver instruction
- `/zellij-agent-manager monitor [interval_seconds]` — periodic health check; reschedules
  via ScheduleWakeup (default: 60s)
- `/zellij-agent-manager new <issue_number>` — create worktree + Zellij session for a
  GitHub issue and spawn a Claude agent to fix it
- `/zellij-agent-manager new <free-form prompt>` — same, driven by a task description

Parse `$ARGUMENTS` to determine which command to run. If empty or "status", run status.

---

## Step 1 — Enumerate all sessions

```bash
$ZJ list-sessions -s -n   # short, no-formatting → one session name per line
```

For each session name, find its agent pane via `agent_pane_for_session`.

---

## Step 2 — Capture pane state

For each `(session, pane_id)` pair, capture the last 35 lines of the viewport:

```bash
$ZJ --session "$session" action dump-screen -p "$pane_id" | tail -35
```

Note: `dump-screen` without `-a` strips ANSI, which is what we want for matching.
Add `-a` only if you need to debug colored output.

**State detection rules** (apply to the captured tail — identical to the tmux skill,
since these match Claude Code's output, not the multiplexer):

| State             | Signal in captured output                                                                                            |
| ----------------- | -------------------------------------------------------------------------------------------------------------------- |
| `idle`            | `─ Worked for` separator present AND `›` prompt at bottom with model info line but NO pending message text after `›` |
| `working`         | Lines contain `• Ran`, `• Read`, `• Writing`, `• Searching`, spinner chars, or active build/test output              |
| `needs_approval`  | Lines near bottom contain "Do you want to", "Allow", "(y/n)", "Yes/No", or "approve"                                 |
| `pending_message` | `›` prompt followed by user message text (received but not yet processed)                                            |
| `stuck`           | Pane content identical across two consecutive polls AND state is not `idle`                                          |
| `unknown`         | None of the above — treat as working                                                                                 |

---

## Step 3 — Status report

Present a compact table, one row per session:

```
SESSION                              STATE             ETA        SUMMARY
add-skill-to-resolve-…               idle              —          Pushed commit 326730bd — waiting for next instruction
descheap-for-raytracing              working           ~20 min    Running slang-test on descriptor-heap-acceleration-structure.slang
wgsl-require-bab-load                needs_approval    ⚠ blocked  Waiting for permission prompt: "Allow bash command?"
fix-lambda-capture                   working           ~2 min     Editing source/slang/slang-check-expr.cpp
```

Truncate session names to 35 chars with `…`. SUMMARY = last meaningful agent output line.

**ETA estimation rules:**

| Activity detected in pane                                | ETA          |
| -------------------------------------------------------- | ------------ |
| State is `idle`                                          | `—`          |
| State is `needs_approval` or `stuck`                     | `⚠ blocked`  |
| State is `pending_message`                               | `~0 min`     |
| Running `cmake --build` or `cmake --workflow`            | `~5–20 min`  |
| Running `slang-test` (full suite)                        | `~15–30 min` |
| Running a single test file                               | `~1–3 min`   |
| Editing/writing files, running short shell commands      | `~1–5 min`   |
| Waiting for CI (GitHub Actions, mentions "workflow run") | `~10–30 min` |
| Submodule init or large git operation                    | `~1–2 min`   |
| Activity clearly just started (spinner, first tool call) | `~5–15 min`  |
| Cannot determine from pane content                       | `?`          |

When the pane contains progress indicators (e.g. `[12/240]` in a cmake build), use them
to refine the estimate: remaining fraction × typical total time.

---

## Step 4 — Send instruction

1. Parse `$ARGUMENTS`: first token after `send` = session name; rest = message.
2. Resolve the agent pane: `PID=$(agent_pane_for_session "$SESSION")`.
3. Deliver the message. For single-line text, `send-keys` is enough; for multi-line
   payloads use `write-chars` (which inserts text verbatim, bracketed-paste style),
   then a separate `send-keys "Enter"` to submit:

```bash
# Multi-line safe path
cat > /tmp/agent_send_msg.txt << 'MSG'
MESSAGE
MSG
$ZJ --session "$SESSION" action write-chars -p "$PID" -- "$(cat /tmp/agent_send_msg.txt)"
$ZJ --session "$SESSION" action send-keys  -p "$PID" "Enter"
```

For a single-line message you can collapse to:

```bash
$ZJ --session "$SESSION" action write-chars -p "$PID" -- "MESSAGE"
$ZJ --session "$SESSION" action send-keys  -p "$PID" "Enter"
```

4. Wait 3 seconds, capture pane tail, confirm the message appears after `›`.
5. Run the **Post-send monitoring** phase (Step 4b) before returning to the user.

> Note: older Zellij builds may not implement `write-chars`. If it errors, fall back to
> piping the message through `send-keys` one chunk at a time, or via a pipe-redirect
> trick: `printf '%s' "$MSG" | $ZJ pipe --plugin file:... ` — but in practice
> `write-chars` has been stable since 0.40.

---

## Step 4b — Post-send monitoring

After every `send` (and after Step 7h for new sessions), verify the agent is actually
making progress and is not silently blocked. This is identical to the tmux skill's
algorithm — it works on captured strings, not on multiplexer specifics.

### Parameters

| Parameter        | Default | Meaning                                                      |
| ---------------- | ------- | ------------------------------------------------------------ |
| `CHECK_INTERVAL` | 10 s    | Seconds between pane polls                                   |
| `MAX_WAIT`       | 120 s   | Stop monitoring after this many seconds                      |
| `WORKING_GRACE`  | 20 s    | Seconds after send before an `idle` return triggers an alert |

### Algorithm

```
elapsed = 0
saw_working = false

loop every CHECK_INTERVAL until elapsed >= MAX_WAIT:
    capture last 35 lines of (session, pane_id) via dump-screen
    classify state (idle / working / needs_approval / unknown)

    if state == needs_approval:
        ALERT: "⚠ SESSION needs approval — agent is waiting for a permission prompt."
        show the relevant pane lines
        return (stop monitoring)

    if state == working or state == unknown:
        saw_working = true
        if elapsed >= MAX_WAIT - CHECK_INTERVAL:
            report "✓ SESSION is working — monitoring complete."
            return

    if state == idle:
        if NOT saw_working AND elapsed < WORKING_GRACE:
            continue
        if NOT saw_working:
            ALERT: "⚠ SESSION returned to idle without any visible tool activity.
                    The agent may be asking a clarifying question or encountered an error."
            show last 35 pane lines
            return
        else:
            report "✓ SESSION completed the task and is now idle."
            return

    sleep CHECK_INTERVAL
    elapsed += CHECK_INTERVAL

report "✓ SESSION is still working after MAX_WAIT s — no attention needed."
```

### Alert format

When emitting an alert, always include: session name, detected state, and the last 35
lines of the pane so the user can see the exact prompt or error.

### After an alert

Do **not** automatically send any reply or click "Yes". Present the pane content and
let the user decide how to respond (e.g., use `send` to answer or approve).

---

## Step 5 — Notifications

<!-- TODO: implement cross-platform notifications (osascript, notify-send). -->

When a session is in `needs_approval` or `stuck` state, skip the notification for now
and simply include a prominent `⚠ NEEDS ATTENTION` marker in the status report.

---

## Step 6 — Monitor loop

1. Run status workflow (Steps 1–3).
2. Mark every `needs_approval` or `stuck` session with `⚠ NEEDS ATTENTION` in the table.
3. Report status table to user.
4. Schedule next wakeup via ScheduleWakeup:
   - `delaySeconds`: interval from `$ARGUMENTS` (default 60, min 60)
   - `prompt`: `/zellij-agent-manager monitor <interval>`
   - `reason`: "periodic zellij agent health check"

---

## Step 7 — Spawn a new agent session

When command is `new <args>`:

- If `<args>` is a bare integer → GitHub issue number
- Otherwise → free-form task prompt

### 7a — Determine slug and task prompt

**Issue number path:**

```bash
gh issue view <number> --repo <REPO> --json number,title,body,labels
```

- `slug` ← from `title`
- `branch prefix` ← labels: "bug"/"crash" → `fix/`; "feature"/"enhancement" → `feature/`;
  else `fix/`
- Claude prompt: issue title + body (truncated to 3000 chars) + instruction to fix,
  test, commit

**Free-form path:**

- `slug` ← from the prompt text
- `branch prefix` ← prompt contains "feature"/"add" → `feature/`; else `fix/`
- Claude prompt: the user's prompt verbatim + instruction to test and commit

**Slug rule:** lowercase → replace runs of non-alphanumeric chars with `-` → collapse
consecutive `-` → strip leading/trailing `-` → truncate to 45 chars.

Full branch: `<prefix><slug>` (e.g. `fix/getTypeNameHint-crash-on-export`)
Session/worktree name: `<slug>` (no prefix)

> Zellij session names allow letters, digits, dashes, and underscores. The slug rule
> above already conforms.

### 7b — Discover paths

```bash
MAIN=$($GIT worktree list --porcelain | awk '/^worktree/{print $2; exit}')
PARENT=$(dirname "$MAIN")
REPO=$($GIT -C "$MAIN" remote get-url origin 2>/dev/null \
    | sed 's|.*github\.com[:/]\(.*\)\.git$|\1|; s|.*github\.com[:/]\(.*\)|\1|')
```

New worktree path: `$PARENT/<slug>`.

### 7c — Collision check

```bash
$ZJ list-sessions -s -n | grep -qx "<slug>" && echo EXISTS || echo OK
test -d "$PARENT/<slug>"                    && echo EXISTS || echo OK
```

Stop and tell the user if either returns `EXISTS`. If a previous Zellij session of the
same name exists as a _resurrectable_ (exited) entry, prefer a fresh slug rather than
reusing it — `zellij delete-session <slug>` is destructive.

### 7d — Create the worktree

```bash
$GIT -C "$MAIN" worktree add "$PARENT/<slug>" -b "<branch>"
```

### 7e — Initialize submodules with local reference

```bash
cd "$PARENT/<slug>"
$GIT submodule update --init --recursive --reference "$MAIN"
```

Tell the user this step is running; it may take up to a minute the first time.

### 7f — Create the Zellij session (detached)

Zellij's equivalent of `tmux new-session -d` is `attach -b` (create-background):

```bash
( cd "$PARENT/<slug>" && $ZJ attach -b "<slug>" </dev/null >/dev/null 2>&1 & )
# Give Zellij a moment to register the session.
sleep 2
$ZJ list-sessions -s -n | grep -qx "<slug>" || {
    echo "Zellij did not register session <slug>; bailing."
    exit 1
}
```

The `-b` flag creates a backgrounded session if one with that name doesn't exist; the
`cd` ensures the initial shell pane opens with the worktree as CWD. Redirecting
stdin/stdout and backgrounding the call prevents Zellij from trying to take over the
parent terminal.

### 7g — Start Claude Code (or Codex)

Resolve the agent pane, then push the launcher command:

```bash
PID=$(agent_pane_for_session "<slug>")

# Claude Code (default)
$ZJ --session "<slug>" action write-chars -p "$PID" -- "claude --dangerously-skip-permissions"
$ZJ --session "<slug>" action send-keys  -p "$PID" "Enter"

# Codex alternative
# $ZJ --session "<slug>" action write-chars -p "$PID" -- "codex --dangerously-bypass-approvals-and-sandbox"
# $ZJ --session "<slug>" action send-keys  -p "$PID" "Enter"
```

Wait 8 seconds, then dump-screen and look for the `›` prompt or model info line. Retry
every 5 seconds up to 3 times. If Claude still hasn't started, show the raw pane
content to the user and stop.

### 7h — Send the task prompt

```bash
cat > /tmp/agent_prompt_<slug>.txt << 'PROMPT'
<composed prompt text>
PROMPT

$ZJ --session "<slug>" action write-chars -p "$PID" -- "$(cat /tmp/agent_prompt_<slug>.txt)"
$ZJ --session "<slug>" action send-keys  -p "$PID" "Enter"
```

After sending, run **Step 4b — Post-send monitoring** targeting `(<slug>, $PID)` to
confirm the agent starts working (not blocked on a permission prompt or asking
questions). Use `WORKING_GRACE=30` for new sessions since Claude Code takes a moment to
start.

### 7i — Report to user

- Branch: `<branch>`
- Worktree: `$PARENT/<slug>`
- Zellij session: `<slug>` — attach with `zellij attach <slug>`
- Agent is running with the task prompt

---

## Notes

- Session names can be long — truncate to 35 chars in the status table with `…`.
- Pane IDs are opaque (`terminal_<n>`); always resolve them via
  `agent_pane_for_session` rather than guessing.
- Use `dump-screen` (no `-a`) to get plain text suitable for matching; add `-a` only
  for debugging.
- Never `kill-session` or `delete-session` without explicit user confirmation —
  `delete-session` removes resurrection metadata and is unrecoverable.
- If Zellij is not on PATH, report that Zellij must be installed and stop. Windows is
  not supported.
- A sibling skill `tmux-agent-manager` provides the same commands for tmux users; the
  two skills coexist and only differ in their environment requirements.
