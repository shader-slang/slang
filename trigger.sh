#!/bin/bash

# Claude Code Action Trigger Script
# Run this to execute the Claude Code workflow locally with act

set -e

echo "🚀 Starting Claude Code workflow with act..."
echo "Repository: szihs/slang"
echo "Event: issue_comment on issue #8"
echo "Actor: szihs"
echo ""

# Run act with all the correct parameters and save logs
act -s GITHUB_TOKEN="$(gh auth token)" \
    --actor szihs \
    --secret-file .env \
    -W .github/workflows/claude.yml \
    -e event.json \
    --container-architecture linux/arm64 \
    issue_comment \
    2>&1 | tee "claude-workflow-$(date +%Y%m%d_%H%M%S).log"

echo ""
echo "✅ Claude Code workflow execution completed"
echo "📄 Logs saved to: claude-workflow-$(date +%Y%m%d_%H%M%S).log"