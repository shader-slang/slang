#!/usr/bin/env python3
"""Unit tests for post_github_review.py."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name("post_github_review.py")
SPEC = importlib.util.spec_from_file_location("post_github_review", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load post_github_review.py")
post_github_review = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(post_github_review)


def make_candidate_text(
    *,
    status: str = "Keep",
    scope_decision: str | None = "Direct",
    scope_rationale: str | None = "changed line",
    overlap_decision: str | None = "Keep",
    overlap_rationale: str | None = "unique concern",
    location: str = "`source/file.cpp:10-11`",
    review_body: str | None = "Review summary.",
    quote_review_body: bool = True,
) -> str:
    lines: list[str] = ["# Scope-Filtered Review Candidates", ""]
    if review_body is not None:
        lines.append("## Review Body")
        if quote_review_body:
            for line in review_body.splitlines():
                lines.append(">" if line == "" else f"> {line}")
        else:
            lines.extend(review_body.splitlines())
        lines.append("")
    lines.extend(
        [
            "## Kept",
            "",
            "### C001: Clarify the invariant",
            "",
            f"- Status: {status}",
            "- Confidence: High",
            "- Scope: Direct",
        ]
    )
    if scope_decision is not None:
        lines.append(f"- Scope decision: {scope_decision}")
    if scope_rationale is not None:
        lines.append(f"- Scope rationale: {scope_rationale}")
    if overlap_decision is not None:
        lines.append(f"- Overlap decision: {overlap_decision}")
    if overlap_rationale is not None:
        lines.append(f"- Overlap rationale: {overlap_rationale}")
    lines.extend(
        [
            f"- Location: {location}",
            "",
            "Context:",
            "",
            "```cpp",
            "example();",
            "```",
            "",
            "Proposed comment:",
            "",
            "> The invariant needed to trust this line is not clear.",
            "> Please make the condition being relied on explicit.",
            "",
        ]
    )
    return "\n".join(lines)


class PostGithubReviewTests(unittest.TestCase):
    def write_candidate_file(self, text: str) -> Path:
        temp_dir = tempfile.TemporaryDirectory()
        self.addCleanup(temp_dir.cleanup)
        path = Path(temp_dir.name) / "candidates.md"
        path.write_text(text, encoding="utf-8")
        return path

    def test_parse_candidates_extracts_review_body_and_comment(self) -> None:
        path = self.write_candidate_file(
            make_candidate_text(
                review_body=(
                    "Review summary.\n"
                    "\n"
                    "## Details\n"
                    "\n"
                    "- The body may contain markdown after unquoting."
                )
            )
        )

        candidates, review_body = post_github_review.parse_candidates(
            path, include_judgment_calls=True
        )

        self.assertEqual(
            review_body,
            (
                "Review summary.\n"
                "\n"
                "## Details\n"
                "\n"
                "- The body may contain markdown after unquoting."
            ),
        )
        self.assertEqual(len(candidates), 1)
        self.assertEqual(candidates[0].candidate_id, "C001")
        self.assertEqual(candidates[0].path, "source/file.cpp")
        self.assertEqual(candidates[0].start_line, 10)
        self.assertEqual(candidates[0].end_line, 11)
        self.assertIn("invariant needed", candidates[0].body)

    def test_unquoted_review_body_line_fails(self) -> None:
        path = self.write_candidate_file(
            make_candidate_text(review_body="Review summary.", quote_review_body=False)
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "strict blockquote"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_lazy_review_body_continuation_fails(self) -> None:
        path = self.write_candidate_file(
            make_candidate_text(
                review_body="> Review summary.\nlazy continuation",
                quote_review_body=False,
            )
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "strict blockquote"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_unquoted_review_body_heading_fails_instead_of_truncating(self) -> None:
        path = self.write_candidate_file(
            make_candidate_text(
                review_body="> Review summary.\n## Details\n> More detail.",
                quote_review_body=False,
            )
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "strict blockquote"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_missing_required_metadata_fails(self) -> None:
        path = self.write_candidate_file(make_candidate_text(scope_decision=None))

        with self.assertRaisesRegex(post_github_review.FatalError, "missing Scope decision"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_invalid_location_fails_before_posting(self) -> None:
        path = self.write_candidate_file(make_candidate_text(location="`source/file.cpp:0`"))

        with self.assertRaisesRegex(post_github_review.FatalError, "invalid Location line range"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_build_payload_labels_agent_review_and_maps_range(self) -> None:
        candidate = post_github_review.Candidate(
            "C001",
            "Clarify the invariant",
            "Keep",
            "Direct",
            "Keep",
            "source/file.cpp",
            10,
            11,
            "The invariant is unclear.",
        )

        payload = post_github_review.build_payload(
            "abc123",
            "REQUEST_CHANGES",
            [candidate],
            "Review summary.",
            post_github_review.DEFAULT_AGENT_REVIEW_PREFIX,
        )

        self.assertEqual(payload["commit_id"], "abc123")
        self.assertEqual(payload["event"], "REQUEST_CHANGES")
        self.assertEqual(payload["body"], "Agent-generated clarity review: Review summary.")
        self.assertEqual(
            payload["comments"],
            [
                {
                    "path": "source/file.cpp",
                    "body": "The invariant is unclear.",
                    "line": 11,
                    "side": "RIGHT",
                    "start_line": 10,
                    "start_side": "RIGHT",
                }
            ],
        )

    def test_build_payload_does_not_duplicate_existing_agent_label(self) -> None:
        payload = post_github_review.build_payload(
            "abc123",
            "COMMENT",
            [
                post_github_review.Candidate(
                    "C001",
                    "Clarify the invariant",
                    "Keep",
                    "Direct",
                    "Keep",
                    "source/file.cpp",
                    10,
                    10,
                    "The invariant is unclear.",
                )
            ],
            "Codex clarity review: Existing label.",
            post_github_review.DEFAULT_AGENT_REVIEW_PREFIX,
        )

        self.assertEqual(payload["body"], "Codex clarity review: Existing label.")

    def test_validate_locations_accepts_lines_in_patch(self) -> None:
        old_run_json = post_github_review.run_json

        def fake_run_json(args: list[str]) -> object:
            joined = " ".join(args)
            if "/pulls/123" in joined and "/files" not in joined:
                return {"head": {"sha": "abc123"}}
            if "/pulls/123/files" in joined:
                return [
                    [
                        {
                            "filename": "source/file.cpp",
                            "patch": (
                                "@@ -1,2 +10,3 @@\n"
                                " context\n"
                                "+added\n"
                                " context2\n"
                            ),
                        }
                    ]
                ]
            raise AssertionError("unexpected gh api call: " + joined)

        post_github_review.run_json = fake_run_json
        self.addCleanup(setattr, post_github_review, "run_json", old_run_json)

        commit_id, diff_lines = post_github_review.validate_locations(
            "gh",
            "shader-slang/slang",
            123,
            [
                post_github_review.Candidate(
                    "C001",
                    "Clarify the invariant",
                    "Keep",
                    "Direct",
                    "Keep",
                    "source/file.cpp",
                    10,
                    12,
                    "The invariant is unclear.",
                )
            ],
        )

        self.assertEqual(commit_id, "abc123")
        self.assertEqual(diff_lines["source/file.cpp"], {10, 11, 12})

    def test_validate_locations_rejects_lines_not_in_patch(self) -> None:
        old_run_json = post_github_review.run_json

        def fake_run_json(args: list[str]) -> object:
            joined = " ".join(args)
            if "/pulls/123" in joined and "/files" not in joined:
                return {"head": {"sha": "abc123"}}
            if "/pulls/123/files" in joined:
                return [[{"filename": "source/file.cpp", "patch": "@@ -1 +10 @@\n+added\n"}]]
            raise AssertionError("unexpected gh api call: " + joined)

        post_github_review.run_json = fake_run_json
        self.addCleanup(setattr, post_github_review, "run_json", old_run_json)

        with self.assertRaisesRegex(post_github_review.FatalError, "missing line\\(s\\): 12"):
            post_github_review.validate_locations(
                "gh",
                "shader-slang/slang",
                123,
                [
                    post_github_review.Candidate(
                        "C001",
                        "Clarify the invariant",
                        "Keep",
                        "Direct",
                        "Keep",
                        "source/file.cpp",
                        12,
                        12,
                        "The invariant is unclear.",
                    )
                ],
            )


if __name__ == "__main__":
    unittest.main()
