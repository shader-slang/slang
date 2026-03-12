#!/usr/bin/env python3
"""
Post a status update to the Slang CI analytics status page.

Clones shader-slang/slang-ci-analytics to a temporary directory, adds or
hides a status entry in status_updates.json, commits, and pushes. Cleans
up the temporary directory on exit regardless of success or failure.

Requires git with push access to shader-slang/slang-ci-analytics (via SSH
key or gh CLI auth). Uses HTTPS by default; set --ssh to use SSH.

Usage:
    # Add a new entry (interactive prompt for body):
    python3 ci_post_status.py add --title "Runner maintenance" --severity warning

    # Add with body inline:
    python3 ci_post_status.py add --title "GPU runners offline" --severity critical \
        --body "Drivers being upgraded. ETA 2 hours."

    # Hide an entry by title substring:
    python3 ci_post_status.py hide --title "GPU runners"

    # List current entries:
    python3 ci_post_status.py list
"""

import argparse
import atexit
import datetime
import json
import os
import shutil
import subprocess
import sys
import tempfile

REPO = "shader-slang/slang-ci-analytics"
HTTPS_URL = f"https://github.com/{REPO}.git"
SSH_URL = f"git@github.com:{REPO}.git"
STATUS_FILE = "status_updates.json"


def run_git(args, cwd):
    """Run a git command, returning the CompletedProcess."""
    return subprocess.run(
        ["git"] + args, cwd=cwd, capture_output=True, text=True, timeout=60
    )


def get_git_user(cwd):
    """Get git user.name and user.email, or None if not configured."""
    name_r = run_git(["config", "user.name"], cwd)
    email_r = run_git(["config", "user.email"], cwd)
    name = name_r.stdout.strip() if name_r.returncode == 0 else None
    email = email_r.stdout.strip() if email_r.returncode == 0 else None
    return name, email


def get_slang_repo_dir():
    """Find the slang repo root (this script lives inside it)."""
    return os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.dirname(os.path.abspath(__file__))
    )))


def get_slang_git_identity():
    """Get git user.name and user.email from the slang repo config."""
    slang_dir = get_slang_repo_dir()
    name_r = run_git(["config", "user.name"], slang_dir)
    email_r = run_git(["config", "user.email"], slang_dir)
    name = name_r.stdout.strip() if name_r.returncode == 0 and name_r.stdout.strip() else None
    email = email_r.stdout.strip() if email_r.returncode == 0 and email_r.stdout.strip() else None
    return name, email


def get_github_username():
    """Try to get the GitHub username via gh CLI."""
    try:
        result = subprocess.run(
            ["gh", "api", "user", "--jq", ".login"],
            capture_output=True, text=True, timeout=15,
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout.strip()
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    return None


def clone_repo(tmpdir, use_ssh):
    """Clone the analytics repo. Returns repo path or exits on failure."""
    url = SSH_URL if use_ssh else HTTPS_URL
    repo_dir = os.path.join(tmpdir, "repo")
    print(f"Cloning {REPO}...")
    result = run_git(["clone", "--depth", "1", url, repo_dir], tmpdir)
    if result.returncode != 0:
        stderr = result.stderr.strip()
        print(f"\nError: Failed to clone {REPO}.", file=sys.stderr)
        if "Permission denied" in stderr or "Authentication failed" in stderr:
            print(
                "\nAuthentication failed. Make sure you have push access to\n"
                f"  {url}\n\n"
                "Options:\n"
                "  - Install gh CLI and run: gh auth login\n"
                "  - Add an SSH key: https://github.com/settings/keys\n"
                "  - Use --ssh flag if you have SSH keys but not HTTPS auth",
                file=sys.stderr,
            )
        else:
            print(stderr, file=sys.stderr)
        sys.exit(1)
    return repo_dir


def load_entries(repo_dir):
    """Load existing entries from status_updates.json."""
    path = os.path.join(repo_dir, STATUS_FILE)
    if not os.path.exists(path):
        return []
    with open(path, encoding="utf-8") as f:
        data = json.load(f)
    return data.get("entries", [])


def save_entries(repo_dir, entries):
    """Write entries back to status_updates.json."""
    path = os.path.join(repo_dir, STATUS_FILE)
    with open(path, "w", encoding="utf-8") as f:
        json.dump({"entries": entries}, f, indent=2, ensure_ascii=False)
        f.write("\n")


def configure_git_identity(repo_dir):
    """Configure git identity on the cloned analytics repo for committing.

    Uses identity from the slang repo config (where this script lives),
    since the user likely has a GitHub-compatible identity set there.
    Falls back to gh CLI, then global git config.
    """
    slang_name, slang_email = get_slang_git_identity()

    name = slang_name
    email = slang_email

    if not name or not email:
        gh_user = get_github_username()
        if not name:
            name = gh_user or None
        if not email and gh_user:
            email = f"{gh_user}@users.noreply.github.com"

    if name:
        run_git(["config", "user.name", name], repo_dir)
    if email:
        run_git(["config", "user.email", email], repo_dir)

    # Verify we have an identity (may come from global config)
    final_name, final_email = get_git_user(repo_dir)
    if not final_name or not final_email:
        print(
            "Warning: No git identity configured.\n"
            "The push may fail. Fix:\n"
            "  git config --global user.name 'Your Name'\n"
            "  git config --global user.email 'you@users.noreply.github.com'",
            file=sys.stderr,
        )


def commit_and_push(repo_dir, message):
    """Stage, commit, and push. Returns True on success."""
    configure_git_identity(repo_dir)
    run_git(["add", STATUS_FILE], repo_dir)

    # Check if there are changes
    result = run_git(["diff", "--cached", "--quiet"], repo_dir)
    if result.returncode == 0:
        print("No changes to commit.")
        return True

    result = run_git(["commit", "-m", message], repo_dir)
    if result.returncode != 0:
        print(f"Error: commit failed:\n{result.stderr}", file=sys.stderr)
        return False

    print("Pushing...")
    result = run_git(["push"], repo_dir)
    if result.returncode != 0:
        stderr = result.stderr.strip()
        print("\nError: push failed.", file=sys.stderr)
        if "GH007" in stderr or "email privacy" in stderr:
            print(
                "\nGitHub blocked the push due to email privacy settings.\n"
                "Fix: Go to https://github.com/settings/emails and either:\n"
                "  1. Uncheck 'Block command line pushes that expose my email', or\n"
                "  2. Set your git email to your GitHub noreply address:\n"
                "     git config --global user.email "
                "'<ID>+<USER>@users.noreply.github.com'",
                file=sys.stderr,
            )
        elif "Permission denied" in stderr or "Authentication failed" in stderr:
            print(
                "\nPush access denied. You need write access to\n"
                f"  {REPO}\n"
                "Ask a maintainer to add you as a collaborator.",
                file=sys.stderr,
            )
        else:
            print(stderr, file=sys.stderr)
        return False

    print("Pushed successfully.")
    return True


def cmd_add(args, tmpdir):
    """Add a new status entry."""
    repo_dir = clone_repo(tmpdir, args.ssh)
    entries = load_entries(repo_dir)

    body = args.body
    if not body:
        print("Enter the status body (end with an empty line):")
        lines = []
        try:
            while True:
                line = input()
                if line == "":
                    break
                lines.append(line)
        except EOFError:
            pass
        body = "\n".join(lines)

    if not body.strip():
        print("Error: body cannot be empty.", file=sys.stderr)
        sys.exit(1)

    author = args.author or get_github_username() or "unknown"
    date = datetime.date.today().isoformat()

    entry = {
        "date": date,
        "severity": args.severity,
        "title": args.title,
        "body": body.strip(),
        "author": author,
    }

    entries.insert(0, entry)
    save_entries(repo_dir, entries)

    if not commit_and_push(repo_dir, f"Status: {args.title}"):
        sys.exit(1)

    print(f'\nAdded [{args.severity}] "{args.title}"')
    print("The status page will update within 15 minutes.")


def cmd_hide(args, tmpdir):
    """Hide entries matching a title substring."""
    repo_dir = clone_repo(tmpdir, args.ssh)
    entries = load_entries(repo_dir)

    search = args.title.lower()
    matched = []
    for entry in entries:
        if search in entry.get("title", "").lower() and entry.get("visible", True):
            entry["visible"] = False
            matched.append(entry.get("title", ""))

    if not matched:
        print(f'No visible entries matching "{args.title}".')
        sys.exit(0)

    save_entries(repo_dir, entries)

    msg = (
        f"Hide status: {matched[0]}"
        if len(matched) == 1
        else f"Hide {len(matched)} status entries"
    )
    if not commit_and_push(repo_dir, msg):
        sys.exit(1)

    titles = "\n  ".join(matched)
    print(f"\nHidden {len(matched)} entry(ies):\n  {titles}")
    print("The status page will update within 15 minutes.")


def cmd_list(args, tmpdir):
    """List current status entries."""
    repo_dir = clone_repo(tmpdir, args.ssh)
    entries = load_entries(repo_dir)

    if not entries:
        print("No status entries.")
        return

    for entry in entries:
        visible = entry.get("visible", True)
        sev = entry.get("severity", "info").upper()
        date = entry.get("date", "")
        title = entry.get("title", "")
        author = entry.get("author", "")
        hidden = " [HIDDEN]" if not visible else ""
        print(f"  {date}  [{sev:8s}]  {title}  ({author}){hidden}")


def main():
    parser = argparse.ArgumentParser(
        description="Post status updates to the Slang CI analytics status page.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "examples:\n"
            '  %(prog)s add --title "GPU runners offline" --severity critical '
            '--body "ETA 2h"\n'
            '  %(prog)s add --title "Maintenance window" --severity warning\n'
            '  %(prog)s hide --title "GPU runners"\n'
            "  %(prog)s list"
        ),
    )
    parser.add_argument(
        "--ssh",
        action="store_true",
        help="Use SSH URL instead of HTTPS for git clone",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # add
    add_p = subparsers.add_parser("add", help="Add a new status entry")
    add_p.add_argument("--title", required=True, help="Short title for the entry")
    add_p.add_argument(
        "--severity",
        choices=["info", "warning", "critical"],
        default="info",
        help="Severity level (default: info)",
    )
    add_p.add_argument("--body", help="Entry body text (prompted if omitted)")
    add_p.add_argument(
        "--author", help="Author (default: GitHub username from gh CLI)"
    )

    # hide
    hide_p = subparsers.add_parser("hide", help="Hide entries matching a title")
    hide_p.add_argument("--title", required=True, help="Title substring to match")

    # list
    subparsers.add_parser("list", help="List current status entries")

    args = parser.parse_args()

    tmpdir = tempfile.mkdtemp(prefix="ci-status-")
    atexit.register(lambda: shutil.rmtree(tmpdir, ignore_errors=True))

    if args.command == "add":
        cmd_add(args, tmpdir)
    elif args.command == "hide":
        cmd_hide(args, tmpdir)
    elif args.command == "list":
        cmd_list(args, tmpdir)


if __name__ == "__main__":
    main()
