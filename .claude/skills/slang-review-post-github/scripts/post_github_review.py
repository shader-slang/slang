#!/usr/bin/env python3
"""Post filtered Slang clarity candidates as one GitHub PR review.

This script intentionally parses a narrow markdown format. If a postable candidate is
ambiguous or cannot be mapped to the GitHub diff, it exits before creating a review.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import tempfile
from typing import Dict, Iterable, List, Optional, Sequence, Set, Tuple


POSTABLE_STATUSES = {"Keep", "Revise", "Needs judgment call", "Needs human judgment"}
JUDGMENT_STATUSES = {"Needs judgment call", "Needs human judgment"}
IGNORED_STATUSES = {"Drop"}
VALID_EVENTS = {"REQUEST_CHANGES", "COMMENT"}
POSTABLE_SCOPE_DECISIONS = {"Direct", "Contextual"}
POSTABLE_OVERLAP_DECISIONS = {"Keep", "Needs judgment call", "Needs human judgment"}
DEFAULT_AGENT_REVIEW_PREFIX = "Agent-generated clarity review: "


class FatalError(Exception):
    pass


class Candidate:
    def __init__(
        self,
        candidate_id: str,
        title: str,
        status: str,
        scope_decision: str,
        overlap_decision: str,
        path: str,
        start_line: int,
        end_line: int,
        body: str,
    ) -> None:
        self.candidate_id = candidate_id
        self.title = title
        self.status = status
        self.scope_decision = scope_decision
        self.overlap_decision = overlap_decision
        self.path = path
        self.start_line = start_line
        self.end_line = end_line
        self.body = body


HEADING_RE = re.compile(r"^###\s+([A-Z]+[0-9]+):\s+(.+?)\s*$")
META_RE = re.compile(r"^-\s+([^:]+):\s*(.*?)\s*$")
LOCATION_RE = re.compile(r"^`?(.+?):([0-9]+)(?:-([0-9]+))?`?$")
HUNK_RE = re.compile(r"^@@\s+-[0-9]+(?:,[0-9]+)?\s+\+([0-9]+)(?:,([0-9]+))?\s+@@")
REVIEW_BODY_BOUNDARY_HEADINGS = {
    "## Kept",
    "## Needs Judgment Call",
    "## Needs Judgment Calls",
    "## Dropped",
}
AGENT_REVIEW_LABEL_RE = re.compile(
    r"^(?:codex-generated|codex|agent-generated|automated)\s+clarity\s+review\s*:",
    re.IGNORECASE,
)


def fail(message: str) -> None:
    raise FatalError(message)


def run_json(args: Sequence[str]) -> object:
    result = subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        fail(
            "command failed: {}\nstdout:\n{}\nstderr:\n{}".format(
                " ".join(args), result.stdout, result.stderr
            )
        )
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as e:
        fail("command did not return JSON: {}\n{}".format(" ".join(args), e))


def run_text(args: Sequence[str]) -> str:
    result = subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        fail(
            "command failed: {}\nstdout:\n{}\nstderr:\n{}".format(
                " ".join(args), result.stdout, result.stderr
            )
        )
    return result.stdout


def is_wsl() -> bool:
    try:
        text = Path("/proc/version").read_text(encoding="utf-8", errors="ignore").lower()
    except OSError:
        return False
    return "microsoft" in text or "wsl" in text


def find_gh(explicit: Optional[str]) -> str:
    if explicit:
        if shutil.which(explicit) or Path(explicit).exists():
            return explicit
        fail("requested GitHub CLI executable was not found: {}".format(explicit))

    env = os.environ.get("GH_CLI")
    if env:
        if shutil.which(env) or Path(env).exists():
            return env
        fail("GH_CLI points to a missing executable: {}".format(env))

    default_name = "gh.exe" if platform.system() == "Windows" or is_wsl() else "gh"
    found = shutil.which(default_name)
    if found:
        return found

    fail(
        "could not find GitHub CLI executable '{}'; pass --gh or set GH_CLI".format(
            default_name
        )
    )


def parse_location(raw: str, candidate_id: str) -> Tuple[str, int, int]:
    text = raw.strip()
    match = LOCATION_RE.match(text)
    if not match:
        fail("{} has invalid Location value: {}".format(candidate_id, raw))
    path = match.group(1).strip()
    start = int(match.group(2))
    end = int(match.group(3) or match.group(2))
    if not path:
        fail("{} has an empty Location path".format(candidate_id))
    if start <= 0 or end <= 0 or end < start:
        fail("{} has invalid Location line range: {}".format(candidate_id, raw))
    return path, start, end


def parse_proposed_comment(block: List[str], candidate_id: str) -> str:
    for i, line in enumerate(block):
        if line.strip() == "Proposed comment:":
            body_lines: List[str] = []
            started = False
            for raw in block[i + 1 :]:
                if not started and raw.strip() == "":
                    continue
                if raw.startswith(">"):
                    started = True
                    text = raw[1:]
                    if text.startswith(" "):
                        text = text[1:]
                    body_lines.append(text.rstrip())
                    continue
                if started:
                    break
                fail("{} has Proposed comment but no blockquote body".format(candidate_id))
            body = "\n".join(body_lines).strip()
            if not body:
                fail("{} has an empty Proposed comment body".format(candidate_id))
            return body
    fail("{} is missing a Proposed comment section".format(candidate_id))


def extract_review_body(lines: List[str]) -> Optional[str]:
    start: Optional[int] = None
    for index, line in enumerate(lines):
        if line.strip() == "## Review Body":
            start = index + 1
            break
    if start is None:
        return None

    section_lines: List[Tuple[int, str]] = []
    for offset, raw in enumerate(lines[start:], start=start + 1):
        if HEADING_RE.match(raw) or raw.strip() in REVIEW_BODY_BOUNDARY_HEADINGS:
            break
        section_lines.append((offset, raw.rstrip()))

    while section_lines and section_lines[0][1].strip() == "":
        section_lines.pop(0)
    while section_lines and section_lines[-1][1].strip() == "":
        section_lines.pop()
    if not section_lines:
        fail("candidate file contains an empty ## Review Body section")

    body_lines: List[str] = []
    invalid_lines: List[str] = []
    for line_number, raw in section_lines:
        if not raw.startswith(">"):
            invalid_lines.append("{}: {}".format(line_number, raw))
            continue
        text = raw[1:]
        if text.startswith(" "):
            text = text[1:]
        body_lines.append(text.rstrip())

    if invalid_lines:
        fail(
            "## Review Body must contain only strict blockquote lines after trimming "
            "leading/trailing blanks; offending line(s):\n- "
            + "\n- ".join(invalid_lines)
        )

    body = "\n".join(body_lines).strip()
    if not body:
        fail("candidate file contains an empty ## Review Body section")
    return body


def parse_candidates(path: Path, include_judgment_calls: bool) -> Tuple[List[Candidate], Optional[str]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    review_body = extract_review_body(lines)
    headings: List[Tuple[int, str, str]] = []
    malformed_headings: List[str] = []
    for index, line in enumerate(lines):
        if not line.startswith("### "):
            continue
        match = HEADING_RE.match(line)
        if not match:
            malformed_headings.append("{}: {}".format(index + 1, line))
        else:
            headings.append((index, match.group(1), match.group(2)))

    if malformed_headings:
        fail("malformed candidate heading(s):\n- " + "\n- ".join(malformed_headings))
    if not headings:
        fail("no candidate headings found in {}".format(path))

    seen_ids: Set[str] = set()
    duplicate_ids: List[str] = []
    for _, candidate_id, _ in headings:
        if candidate_id in seen_ids:
            duplicate_ids.append(candidate_id)
        seen_ids.add(candidate_id)
    if duplicate_ids:
        fail("duplicate candidate ID(s): " + ", ".join(sorted(set(duplicate_ids))))

    candidates: List[Candidate] = []
    errors: List[str] = []
    for h_index, (start_index, candidate_id, title) in enumerate(headings):
        end_index = headings[h_index + 1][0] if h_index + 1 < len(headings) else len(lines)
        block = lines[start_index + 1 : end_index]
        metadata: Dict[str, str] = {}
        for line in block:
            match = META_RE.match(line)
            if match:
                metadata[match.group(1).strip()] = match.group(2).strip()

        status = metadata.get("Status")
        if not status:
            errors.append("{} is missing Status".format(candidate_id))
            continue

        if status in IGNORED_STATUSES:
            continue
        if status in JUDGMENT_STATUSES and not include_judgment_calls:
            continue
        if status not in POSTABLE_STATUSES:
            errors.append(
                "{} has non-postable Status {!r}; run consolidation and scope filtering first".format(
                    candidate_id, status
                )
            )
            continue

        scope_decision = metadata.get("Scope decision")
        if not scope_decision:
            errors.append("{} is missing Scope decision".format(candidate_id))
            continue
        if scope_decision not in POSTABLE_SCOPE_DECISIONS:
            errors.append(
                "{} has non-postable Scope decision {!r}; expected Direct or Contextual".format(
                    candidate_id, scope_decision
                )
            )
            continue

        scope_rationale = metadata.get("Scope rationale")
        if not scope_rationale:
            errors.append("{} is missing Scope rationale".format(candidate_id))
            continue

        overlap_decision = metadata.get("Overlap decision")
        if not overlap_decision:
            errors.append("{} is missing Overlap decision".format(candidate_id))
            continue
        if overlap_decision not in POSTABLE_OVERLAP_DECISIONS:
            errors.append(
                "{} has non-postable Overlap decision {!r}; run consolidation first".format(
                    candidate_id, overlap_decision
                )
            )
            continue

        overlap_rationale = metadata.get("Overlap rationale")
        if not overlap_rationale:
            errors.append("{} is missing Overlap rationale".format(candidate_id))
            continue

        location = metadata.get("Location")
        if not location:
            errors.append("{} is missing Location".format(candidate_id))
            continue

        try:
            comment_body = parse_proposed_comment(block, candidate_id)
            file_path, start_line, end_line = parse_location(location, candidate_id)
        except FatalError as e:
            errors.append(str(e))
            continue

        candidates.append(
            Candidate(
                candidate_id,
                title,
                status,
                scope_decision,
                overlap_decision,
                file_path,
                start_line,
                end_line,
                comment_body,
            )
        )

    if errors:
        fail("candidate file is not postable:\n- " + "\n- ".join(errors))
    if not candidates:
        fail("no postable candidates found")
    return candidates, review_body


def flatten_pages(value: object) -> List[dict]:
    if not isinstance(value, list):
        fail("expected a JSON array from GitHub")
    if not value:
        return []
    if all(isinstance(item, list) for item in value):
        result: List[dict] = []
        for page in value:
            result.extend(page)
        return result
    if all(isinstance(item, dict) for item in value):
        return value  # type: ignore[return-value]
    fail("unexpected paginated JSON shape from GitHub")


def right_lines_from_patch(patch: str) -> Set[int]:
    lines: Set[int] = set()
    current: Optional[int] = None
    for raw in patch.splitlines():
        match = HUNK_RE.match(raw)
        if match:
            current = int(match.group(1))
            continue
        if current is None:
            continue
        if raw.startswith("+") and not raw.startswith("+++"):
            lines.add(current)
            current += 1
        elif raw.startswith(" "):
            lines.add(current)
            current += 1
        elif raw.startswith("-") and not raw.startswith("---"):
            continue
        elif raw == r"\ No newline at end of file":
            continue
    return lines


def validate_locations(
    gh: str, repo: str, pr: int, candidates: Iterable[Candidate]
) -> Tuple[str, Dict[str, Set[int]]]:
    pr_info = run_json([gh, "api", "repos/{}/pulls/{}".format(repo, pr)])
    if not isinstance(pr_info, dict):
        fail("unexpected PR JSON shape")
    head = pr_info.get("head")
    if not isinstance(head, dict) or not isinstance(head.get("sha"), str):
        fail("could not determine PR head commit")
    commit_id = head["sha"]

    pages = run_json(
        [
            gh,
            "api",
            "--paginate",
            "--slurp",
            "repos/{}/pulls/{}/files?per_page=100".format(repo, pr),
        ]
    )
    files = flatten_pages(pages)
    diff_lines: Dict[str, Set[int]] = {}
    for item in files:
        if not isinstance(item, dict):
            continue
        filename = item.get("filename")
        patch = item.get("patch")
        if isinstance(filename, str) and isinstance(patch, str):
            diff_lines[filename] = right_lines_from_patch(patch)

    errors: List[str] = []
    for candidate in candidates:
        lines = diff_lines.get(candidate.path)
        if lines is None:
            errors.append(
                "{} Location path is not present in fetched PR diff patches: {}".format(
                    candidate.candidate_id, candidate.path
                )
            )
            continue
        missing = [
            line
            for line in range(candidate.start_line, candidate.end_line + 1)
            if line not in lines
        ]
        if missing:
            errors.append(
                "{} Location {}:{}-{} is not fully present in the GitHub diff; missing line(s): {}".format(
                    candidate.candidate_id,
                    candidate.path,
                    candidate.start_line,
                    candidate.end_line,
                    ", ".join(str(line) for line in missing),
                )
            )

    if errors:
        fail("candidate locations are not postable:\n- " + "\n- ".join(errors))
    return commit_id, diff_lines


def build_payload(
    commit_id: str,
    event: str,
    candidates: List[Candidate],
    review_body: Optional[str],
    agent_review_prefix: Optional[str],
) -> dict:
    judgment_call_count = sum(1 for candidate in candidates if candidate.status in JUDGMENT_STATUSES)
    if review_body is None:
        review_body = "Clarity review."
        if judgment_call_count:
            review_body += (
                "\n\nSome included comments were marked as needing a judgment call in the "
                "candidate file and are included because the automated review considered them "
                "credible enough to raise."
            )

    if agent_review_prefix:
        if not AGENT_REVIEW_LABEL_RE.match(review_body):
            review_body = agent_review_prefix + review_body

    comments = []
    for candidate in candidates:
        comment = {
            "path": candidate.path,
            "body": candidate.body,
            "line": candidate.end_line,
            "side": "RIGHT",
        }
        if candidate.start_line != candidate.end_line:
            comment["start_line"] = candidate.start_line
            comment["start_side"] = "RIGHT"
        comments.append(comment)

    return {
        "commit_id": commit_id,
        "event": event,
        "body": review_body,
        "comments": comments,
    }


def post_review(gh: str, repo: str, pr: int, payload: dict) -> str:
    with tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".json", delete=False) as f:
        json.dump(payload, f, indent=2)
        temp_path = f.name
    try:
        return run_text(
            [
                gh,
                "api",
                "--method",
                "POST",
                "--input",
                temp_path,
                "repos/{}/pulls/{}/reviews".format(repo, pr),
            ]
        )
    finally:
        try:
            os.unlink(temp_path)
        except OSError:
            pass


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, help="GitHub repository, e.g. shader-slang/slang")
    parser.add_argument("--pr", required=True, type=int, help="Pull request number")
    parser.add_argument("--candidates", required=True, type=Path, help="Candidate markdown file")
    parser.add_argument("--gh", help="Path to gh or gh.exe; defaults to GH_CLI or platform default")
    parser.add_argument(
        "--event",
        choices=sorted(VALID_EVENTS),
        default="REQUEST_CHANGES",
        help="GitHub review event to submit",
    )
    parser.add_argument("--body", help="Review body text")
    parser.add_argument("--body-file", type=Path, help="Read review body text from this file")
    parser.add_argument(
        "--agent-review-prefix",
        default=DEFAULT_AGENT_REVIEW_PREFIX,
        help="Prefix added to the review body to identify the agent-authored review",
    )
    parser.add_argument(
        "--omit-agent-review-label",
        action="store_true",
        help="Do not add the default agent-authorship prefix to the review body",
    )
    parser.add_argument(
        "--exclude-needs-human-judgment",
        action="store_true",
        help="Alias for --exclude-judgment-calls",
    )
    parser.add_argument(
        "--exclude-judgment-calls",
        action="store_true",
        help="Do not post candidates with Status: Needs judgment call",
    )
    parser.add_argument("--dry-run", action="store_true", help="Validate and print payload; do not post")
    return parser.parse_args(argv)


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    if not args.candidates.exists():
        fail("candidate file does not exist: {}".format(args.candidates))

    gh = find_gh(args.gh)
    if args.body and args.body_file:
        fail("use either --body or --body-file, not both")

    candidates, candidate_file_body = parse_candidates(
        args.candidates,
        include_judgment_calls=not (
            args.exclude_judgment_calls or args.exclude_needs_human_judgment
        ),
    )
    commit_id, _ = validate_locations(gh, args.repo, args.pr, candidates)

    review_body = args.body
    if args.body_file:
        if not args.body_file.exists():
            fail("review body file does not exist: {}".format(args.body_file))
        review_body = args.body_file.read_text(encoding="utf-8").strip()
        if not review_body:
            fail("review body file is empty: {}".format(args.body_file))
    if review_body is None:
        review_body = candidate_file_body

    agent_review_prefix = None if args.omit_agent_review_label else args.agent_review_prefix
    payload = build_payload(commit_id, args.event, candidates, review_body, agent_review_prefix)

    if args.dry_run:
        print(json.dumps(payload, indent=2))
        return 0

    output = post_review(gh, args.repo, args.pr, payload)
    print(output)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except FatalError as e:
        print("error: {}".format(e), file=sys.stderr)
        raise SystemExit(2)
