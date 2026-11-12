---
name: Slang Maintainer Duty Template
about: A checklist template for Slang maintainer duties and tasks
title: 'Slang Maintainer Tasks - Sprint [NUMBER]'
labels: 'maintenance'
assignees: ''

---

# Slang Maintainer Tasks

**Legend:**
- ✅ = Completed/Yes
- ❌ = Not completed/No  
- 🟠 = Not applicable/N/A

**Instructions:** Delete the symbols you don't want to keep, leaving only your choice.

## Daily Activities

| # | Task | Status | Remarks |
|---|------|--------|---------|
| 1 | Triage incoming bugs on GitHub and GitLab | ✅ ❌ 🟠 | |
| 2 | Add "Dev_Reviewed" label to reviewed issues | ✅ ❌ 🟠 | |
| 3 | Monitor and reply to Discord, Nvr-slang, Nvr-slang-dev, Nvr-rtr-slangpy, and GitHub discussions | ✅ ❌ 🟠 | |
| 4 | Review/respond to open PRs on GitHub from external folks | ✅ ❌ 🟠 | |
| 5 | Monitor merge CI for intermittent failures | ✅ ❌ 🟠 | |
| 6 | Monitor VK-CTS-Nightly test status | ✅ ❌ 🟠 | |
| 7 | Investigate VK-CTS-Nightly failures and reproduce if needed | ✅ ❌ 🟠 | |
| 8 | Send daily status report via LLM (Cursor) to dev slack channel | ✅ ❌ 🟠 | |

## Per Sprint Activities

| # | Task | Status | Remarks |
|---|------|--------|---------|
| 1 | Perform GitHub release for Slang and Slangpy | ✅ ❌ 🟠 | |
| 2 | Update SPIRV submodule in Slang repo | ✅ ❌ 🟠 | |
| 3 | Perform GitHub to GitLab sync (rebase, not merge) | ✅ ❌ 🟠 | |
| 4 | Ensure CI/workflow is passing for GitLab sync | ✅ ❌ 🟠 | |
| 5 | Handle GitLab unique commits (upstream, drop, or keep) | ✅ ❌ 🟠 | |
| 6 | Update slang-vscode-extension for major updates (create PR with new tag) | ✅ ❌ 🟠 | |
| 7 | At the end of the sprint, reset values in this table to "Not started", and update sprint number. | ✅ ❌ 🟠 | |