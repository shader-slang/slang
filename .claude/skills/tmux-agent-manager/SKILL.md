---
name: tmux-agent-manager
description: >-
  Manage multiple Claude Code agent sessions running in tmux. Report a
  status summary of all sessions, deliver instructions to a specific agent
  via tmux send-keys, and send notifications when any session needs attention.
  Use when the user wants to oversee concurrent Claude agents, check what
  sessions are doing, send a message to an agent, or monitor for sessions
  that need input or are stuck. Works on native Linux, macOS, WSL (inside),
  and Windows with WSL (Git Bash or PowerShell host).
argument-hint: "[status | send <session> <message> | monitor [interval_seconds] | new <issue_number_or_prompt>]"
allowed-tools:
  - Bash
---

# Tmux Agent Manager

Oversees multiple Claude Code sessions running in tmux.

---

## Environment Detection (run first, before any other step)

Run this once at the start of every skill invocation to set the variables
used throughout:

```bash
bash << 'DETECT'
# How to reach tmux
if command -v tmux &>/dev/null; then
    TMUX_EXEC="tmux"
    SH="bash -c"        # run inline shell commands
else
    # tmux not in PATH — assume Windows host, reach it through WSL
    TMUX_EXEC="wsl tmux"
    SH="wsl bash -c"
fi

# Which git binary produces consistent paths
# Inside WSL, git.exe gives Windows-style paths that match the worktree list
if grep -qi microsoft /proc/version 2>/dev/null; then
    HOST="wsl_inside"
    GIT="git.exe"
elif [ "$TMUX_EXEC" = "wsl tmux" ]; then
    HOST="windows"
    GIT="git.exe"
else
    HOST="unix"         # native Linux or macOS
    GIT="git"
fi

echo "TMUX_EXEC=$TMUX_EXEC SH=$SH HOST=$HOST GIT=$GIT"
DETECT
```

**Variable reference used in all steps below:**

| Variable | Windows/Git Bash | WSL inside | Native Linux/macOS |
|---|---|---|---|
| `TMUX_EXEC` | `wsl tmux` | `tmux` | `tmux` |
| `SH` | `wsl bash -c` | `bash -c` | `bash -c` |
| `GIT` | `git.exe` | `git.exe` | `git` |
| `HOST` | `windows` | `wsl_inside` | `unix` |

**Path helpers** — use these when building paths for tmux vs. for git:

- On `unix`: only one path form; no conversion needed.
- On `wsl_inside`: `git.exe` returns Windows paths (`D:/foo`); convert with `wslpath` before
  passing to tmux or bash. Shell path = `wslpath "D:/foo"` → `/mnt/d/foo`.
- On `windows`: `git.exe` returns Windows paths; convert with `wsl wslpath` before passing to
  `wsl tmux` or `wsl bash`. Windows path → WSL path: `wsl wslpath "D:/foo"` → `/mnt/d/foo`.

---

## Commands

- `/tmux-agent-manager` or `/tmux-agent-manager status` — snapshot of all sessions
- `/tmux-agent-manager send <session-name> <message>` — deliver instruction to an agent
- `/tmux-agent-manager monitor [interval_seconds]` — check all sessions, notify on any
  needing attention, schedule the next check via ScheduleWakeup (default: 60s)
- `/tmux-agent-manager new <issue_number>` — create worktree + tmux session for a GitHub
  issue and spawn a Claude agent to fix it
- `/tmux-agent-manager new <free-form prompt>` — same, driven by a task description

Parse `$ARGUMENTS` to determine which command to run. If empty or "status", run status.

---

## Step 1 — Enumerate all panes

```bash
$TMUX_EXEC list-panes -a -F "#{session_name}:#{window_index}.#{pane_index} [#{pane_current_command}]"
```

The first pane of each session (window 0, pane 0) is where the Claude Code agent lives.

---

## Step 2 — Capture pane state

For each pane target `SESSION:W.P`, capture the last 35 lines:

```bash
$TMUX_EXEC capture-pane -t "SESSION:W.P" -p | tail -35
```

**State detection rules** (apply to the captured tail):

| State | Signal in captured output |
|---|---|
| `idle` | `─ Worked for` separator present AND `›` prompt at bottom with model info line but NO pending message text after `›` |
| `working` | Lines contain `• Ran`, `• Read`, `• Writing`, `• Searching`, spinner chars, or active build/test output |
| `needs_approval` | Lines near bottom contain "Do you want to", "Allow", "(y/n)", "Yes/No", or "approve" |
| `pending_message` | `›` prompt followed by user message text (received but not yet processed) |
| `stuck` | Pane content identical across two consecutive polls AND state is not `idle` |
| `unknown` | None of the above — treat as working |

---

## Step 3 — Status report

Present a compact table, one row per session:

```
SESSION                              STATE             SUMMARY
add-skill-to-resolve-…               idle              Pushed commit 326730bd — waiting for next instruction
descheap-for-raytracing              working           Running slang-test on descriptor-heap-acceleration-structure.slang
wgsl-require-bab-load                idle              Build succeeded 8m ago
```

Truncate session names to 35 chars with `…`. SUMMARY = last meaningful agent output line.

---

## Step 4 — Send instruction

1. Parse `$ARGUMENTS`: first token after `send` = session name; rest = message.
2. Send to the agent pane:

```bash
$TMUX_EXEC send-keys -t "SESSION:0.0" "MESSAGE" Enter
```

3. Wait 3 seconds, capture pane tail, confirm message appears after `›`.

---

## Step 5 — Notifications

<!-- TODO: implement cross-platform notifications (osascript, notify-send, PowerShell toast) -->

When a session is in `needs_approval` or `stuck` state, skip the notification for now and
simply include a prominent `⚠ NEEDS ATTENTION` marker in the status report printed to the
user.

---

## Step 6 — Monitor loop

1. Run status workflow (Steps 1–3).
2. Fire notifications for every `needs_approval` or `stuck` session.
3. Report status table to user.
4. Schedule next wakeup via ScheduleWakeup:
   - `delaySeconds`: interval from `$ARGUMENTS` (default 60, min 60)
   - `prompt`: `/tmux-agent-manager monitor <interval>`
   - `reason`: "periodic tmux agent health check"

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
- `branch prefix` ← labels: "bug"/"crash" → `fix/`; "feature"/"enhancement" → `feature/`; else `fix/`
- Claude prompt: issue title + body (truncated to 3000 chars) + instruction to fix, test, commit

**Free-form path:**
- `slug` ← from the prompt text
- `branch prefix` ← prompt contains "feature"/"add" → `feature/`; else `fix/`
- Claude prompt: the user's prompt verbatim + instruction to test and commit

**Slug rule:** lowercase → replace runs of non-alphanumeric chars with `-` → collapse
consecutive `-` → strip leading/trailing `-` → truncate to 45 chars.

Full branch: `<prefix><slug>` (e.g. `fix/getTypeNameHint-crash-on-export`)
Session/worktree name: `<slug>` (no prefix)

### 7b — Discover paths dynamically

Run this inside a single shell call (adapt prefix for HOST):

```bash
# Get the main worktree path (first entry — always the primary checkout)
MAIN_NATIVE=$($GIT worktree list --porcelain | awk '/^worktree/{print $2; exit}')

# Convert to the shell's native path if needed
if [ "$HOST" = "wsl_inside" ]; then
    MAIN_SHELL=$(wslpath "$MAIN_NATIVE")       # D:/foo → /mnt/d/foo
elif [ "$HOST" = "windows" ]; then
    MAIN_SHELL=$(wsl wslpath "$MAIN_NATIVE")   # D:/foo → /mnt/d/foo (for wsl tmux calls)
else
    MAIN_SHELL="$MAIN_NATIVE"                  # already a POSIX path
fi

PARENT_SHELL=$(dirname "$MAIN_SHELL")          # sibling worktrees live here
PARENT_NATIVE=$(dirname "$MAIN_NATIVE")

# Derive GitHub repo (owner/name) from remote URL
REPO=$($GIT -C "$MAIN_SHELL" remote get-url origin 2>/dev/null \
    | sed 's|.*github\.com[:/]\(.*\)\.git$|\1|; s|.*github\.com[:/]\(.*\)|\1|')
```

New worktree paths:
- Shell path (for tmux `-c` and cd): `$PARENT_SHELL/<slug>`
- Native path (for `git worktree add`): `$PARENT_NATIVE/<slug>`

### 7c — Collision check

```bash
$TMUX_EXEC has-session -t "<slug>" 2>/dev/null && echo EXISTS || echo OK
test -d "$PARENT_SHELL/<slug>" && echo EXISTS || echo OK
```

Stop and tell the user if either returns `EXISTS`.

### 7d — Create the worktree

```bash
$GIT -C "$MAIN_SHELL" worktree add "$PARENT_NATIVE/<slug>" -b "<branch>"
```

### 7e — Initialize submodules with local reference

`--reference` points git at the main worktree's object store so it copies blobs
locally instead of downloading them again — much faster on large repos:

```bash
cd "$PARENT_SHELL/<slug>"
$GIT submodule update --init --recursive --reference "$MAIN_NATIVE"
```

Tell the user this step is running; it may take up to a minute the first time.

### 7f — Create the tmux session

```bash
$TMUX_EXEC new-session -d -s "<slug>" -c "$PARENT_SHELL/<slug>"
```

### 7g — Start Claude Code

```bash
$TMUX_EXEC send-keys -t "<slug>:0.0" "claude --dangerously-skip-permissions" Enter
```

Wait 8 seconds, then capture the pane tail and look for the `›` prompt or model info
line. Retry every 5 seconds up to 3 times. If Claude still hasn't started, show the raw
pane content to the user and stop.

### 7h — Send the task prompt

Write to a temp file to safely handle newlines and special characters:

```bash
cat > /tmp/agent_prompt_<slug>.txt << 'PROMPT'
<composed prompt text>
PROMPT

$TMUX_EXEC load-buffer /tmp/agent_prompt_<slug>.txt
$TMUX_EXEC paste-buffer -t "<slug>:0.0"
$TMUX_EXEC send-keys -t "<slug>:0.0" "" Enter
```

### 7i — Report to user

- Branch: `<branch>`
- Worktree: `$PARENT_NATIVE/<slug>`
- Tmux session: `<slug>` — attach with `tmux attach -t <slug>`
- Agent is running with the task prompt

---

## Notes

- Session names can be long — truncate to 35 chars in the status table with `…`
- The Claude Code pane is always window 0, pane 0 unless the user specifies otherwise
- When a session has multiple windows, check window 0 for the agent; note other windows
  separately if they show interesting activity (build output, test results)
- Never kill or restart a session without explicit user confirmation
- If tmux is not found at all (neither native nor via `wsl tmux`), report that tmux must
  be installed and stop
