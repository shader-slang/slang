#!/usr/bin/env python3
"""Unit tests for post_github_review.py."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name("post_github_review.py")
SPEC = importlib.util.spec_from_file_location("post_github_review", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("could not load post_github_review.py")
post_github_review = importlib.util.module_from_spec(SPEC)
# Decorators such as `dataclass` expect the module to be registered while it is executing.
sys.modules[SPEC.name] = post_github_review
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
    proposed_comment_lines: list[str] | None = None,
    notes_lines: list[str] | None = None,
) -> str:
    """Build a minimal candidate file for parser and payload tests."""

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
    if proposed_comment_lines is None:
        proposed_comment_lines = [
            "> The invariant needed to trust this line is not clear.",
            "> Please make the condition being relied on explicit.",
        ]
    if notes_lines is None:
        notes_lines = []
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
        ]
    )
    lines.extend(proposed_comment_lines)
    lines.append("")
    if notes_lines:
        lines.append("Notes:")
        lines.extend(notes_lines)
        lines.append("")
    return "\n".join(lines)


class PostGithubReviewTests(unittest.TestCase):
    """Regression tests for candidate parsing, diff validation, and review payloads."""

    def write_candidate_file(self, text: str) -> Path:
        """Write candidate markdown to a temporary file cleaned up after the test."""

        temp_dir = tempfile.TemporaryDirectory()
        self.addCleanup(temp_dir.cleanup)
        path = Path(temp_dir.name) / "candidates.md"
        path.write_text(text, encoding="utf-8")
        return path

    def test_parse_candidates_extracts_review_body_and_comment(self) -> None:
        """Quoted review-body and proposed-comment content is extracted without metadata."""

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
        """A review body with ordinary unquoted content is rejected."""

        path = self.write_candidate_file(
            make_candidate_text(review_body="Review summary.", quote_review_body=False)
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "strict blockquote"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_lazy_review_body_continuation_fails(self) -> None:
        """A review body using Markdown lazy blockquote continuation is rejected."""

        path = self.write_candidate_file(
            make_candidate_text(
                review_body="> Review summary.\nlazy continuation",
                quote_review_body=False,
            )
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "strict blockquote"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_review_body_stops_at_later_top_level_heading(self) -> None:
        """A later top-level heading starts a new section after the review body."""

        path = self.write_candidate_file(
            make_candidate_text(
                review_body="> Review summary.\n## Details\nThis is a later section.",
                quote_review_body=False,
            )
        )

        _, review_body = post_github_review.parse_candidates(
            path, include_judgment_calls=True
        )

        self.assertEqual(review_body, "Review summary.")

    def test_review_body_stops_at_later_top_level_section(self) -> None:
        """The review body ends before any later top-level `##` section."""

        text = make_candidate_text(review_body="Review summary.")
        text = text.replace(
            "\n## Kept\n",
            "\n## PR Summary\n\nThis summary is not part of the review body.\n\n## Kept\n",
        )
        path = self.write_candidate_file(text)

        _, review_body = post_github_review.parse_candidates(
            path, include_judgment_calls=True
        )

        self.assertEqual(review_body, "Review summary.")

    def test_missing_required_metadata_fails(self) -> None:
        """Posting fails when a required post-filter metadata field is absent."""

        path = self.write_candidate_file(make_candidate_text(scope_decision=None))

        with self.assertRaisesRegex(post_github_review.FatalError, "missing Scope decision"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_invalid_location_fails_before_posting(self) -> None:
        """Invalid candidate line ranges are rejected during candidate parsing."""

        path = self.write_candidate_file(make_candidate_text(location="`source/file.cpp:0`"))

        with self.assertRaisesRegex(post_github_review.FatalError, "invalid Location line range"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_lazy_proposed_comment_continuation_fails(self) -> None:
        """A proposed comment with unquoted continuation content is rejected."""

        path = self.write_candidate_file(
            make_candidate_text(
                proposed_comment_lines=[
                    "> The invariant needed to trust this line is not clear.",
                    "lazy continuation",
                ]
            )
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "strict blockquote"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_stray_text_after_proposed_comment_terminator_fails(self) -> None:
        """Only blank lines or a `Notes:` section may follow a terminated comment."""

        path = self.write_candidate_file(
            make_candidate_text(
                proposed_comment_lines=[
                    "> The invariant needed to trust this line is not clear.",
                    "",
                    "This line would otherwise be silently dropped.",
                ]
            )
        )

        with self.assertRaisesRegex(post_github_review.FatalError, "non-note content"):
            post_github_review.parse_candidates(path, include_judgment_calls=True)

    def test_notes_metadata_does_not_override_candidate_metadata(self) -> None:
        """Bullets in `Notes:` cannot override the candidate header metadata."""

        path = self.write_candidate_file(
            make_candidate_text(
                notes_lines=[
                    "This note records discarded alternatives.",
                    "- Status: Drop",
                    "- Scope decision: Probably out-of-scope",
                ]
            )
        )

        candidates, _ = post_github_review.parse_candidates(path, include_judgment_calls=True)

        self.assertEqual(len(candidates), 1)
        self.assertEqual(candidates[0].status, "Keep")
        self.assertEqual(candidates[0].scope_decision, "Direct")

    def test_prepare_review_body_accepts_hyphenated_agent_attribution(self) -> None:
        """Human-proxied reviews must already carry agent-authorship attribution."""

        body = post_github_review.prepare_review_body(
            "GPT-4.1-authored clarity review: Review summary.",
            [],
            acting_as_bot_user=False,
        )

        self.assertEqual(body, "GPT-4.1-authored clarity review: Review summary.")

    def test_prepare_review_body_accepts_whitespace_agent_attribution(self) -> None:
        """The attribution separator may be whitespace instead of a hyphen."""

        body = post_github_review.prepare_review_body(
            "Claude 3.7 Sonnet authored review: Review summary.",
            [],
            acting_as_bot_user=False,
        )

        self.assertEqual(body, "Claude 3.7 Sonnet authored review: Review summary.")

    def test_prepare_review_body_accepts_punctuated_agent_identity(self) -> None:
        """Agent identity may use punctuation as long as it stays on one line."""

        body = post_github_review.prepare_review_body(
            "OpenAI/GPT-4.1 (reviewer)-authored clarity review: Review summary.",
            [],
            acting_as_bot_user=False,
        )

        self.assertEqual(
            body,
            "OpenAI/GPT-4.1 (reviewer)-authored clarity review: Review summary.",
        )

    def test_prepare_review_body_accepts_attribution_without_review_type(self) -> None:
        """When the review type is omitted, `authored review:` is sufficient."""

        body = post_github_review.prepare_review_body(
            "Codex-authored review: Review summary.",
            [],
            acting_as_bot_user=False,
        )

        self.assertEqual(body, "Codex-authored review: Review summary.")

    def test_prepare_review_body_rejects_missing_agent_attribution(self) -> None:
        """Human-proxied reviews without attribution fail before posting."""

        with self.assertRaisesRegex(post_github_review.FatalError, "must start"):
            post_github_review.prepare_review_body(
                "Review summary.", [], acting_as_bot_user=False
            )

    def test_prepare_review_body_rejects_overlong_agent_identity(self) -> None:
        """Agent identity must stay within the conservative length limit."""

        agent_identity = "A" * (post_github_review.MAX_AGENT_IDENTITY_SCALARS + 1)

        with self.assertRaisesRegex(post_github_review.FatalError, "50 Unicode"):
            post_github_review.prepare_review_body(
                f"{agent_identity}-authored clarity review: Review summary.",
                [],
                acting_as_bot_user=False,
            )

    def test_prepare_review_body_rejects_multiline_agent_attribution(self) -> None:
        """The agent identity must not contain a newline."""

        with self.assertRaisesRegex(post_github_review.FatalError, "must start"):
            post_github_review.prepare_review_body(
                "GPT-4.1\n-authored clarity review: Review summary.",
                [],
                acting_as_bot_user=False,
            )

    def test_prepare_review_body_allows_missing_attribution_for_bot_user(self) -> None:
        """Bot-account reviews may omit in-body attribution."""

        body = post_github_review.prepare_review_body(
            "Review summary.", [], acting_as_bot_user=True
        )

        self.assertEqual(body, "Review summary.")

    def test_prepare_review_body_uses_fallback_only_for_bot_user(self) -> None:
        """A missing body gets a fallback only when GitHub provides bot attribution."""

        body = post_github_review.prepare_review_body(
            None,
            [
                post_github_review.Candidate(
                    "C001",
                    "Clarify the invariant",
                    "Needs judgment call",
                    "Direct",
                    "Needs judgment call",
                    "source/file.cpp",
                    10,
                    10,
                    "The invariant is unclear.",
                )
            ],
            acting_as_bot_user=True,
        )

        self.assertIn("Clarity review.", body)
        self.assertIn("needing a judgment call", body)

    def test_build_payload_preserves_review_body_and_maps_range(self) -> None:
        """Payload construction preserves review text and multi-line comment ranges."""

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
            "Codex-authored clarity review: Review summary.",
        )

        self.assertEqual(payload["commit_id"], "abc123")
        self.assertEqual(payload["event"], "REQUEST_CHANGES")
        self.assertEqual(payload["body"], "Codex-authored clarity review: Review summary.")
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

    def test_parse_args_defaults_to_comment_event(self) -> None:
        """Automated review posting defaults to a non-blocking GitHub review event."""

        args = post_github_review.parse_args(
            [
                "--repo",
                "shader-slang/slang",
                "--pr",
                "123",
                "--candidates",
                "candidates.md",
            ]
        )

        self.assertEqual(args.event, "COMMENT")

    def test_main_rejects_bot_user_request_changes_before_gh_lookup(self) -> None:
        """Bot-account attribution cannot be combined with a blocking review event."""

        path = self.write_candidate_file(make_candidate_text())
        old_find_gh = post_github_review.find_gh

        def fake_find_gh(_override: str | None) -> str:
            """Prove the policy validation happens before GitHub CLI discovery."""

            raise AssertionError("find_gh should not run")

        post_github_review.find_gh = fake_find_gh
        self.addCleanup(setattr, post_github_review, "find_gh", old_find_gh)

        with self.assertRaisesRegex(post_github_review.FatalError, "requires --event COMMENT"):
            post_github_review.main(
                [
                    "--repo",
                    "shader-slang/slang",
                    "--pr",
                    "123",
                    "--candidates",
                    str(path),
                    "--acting-as-bot-user",
                    "--event",
                    "REQUEST_CHANGES",
                    "--dry-run",
                ]
            )

    def test_run_command_uses_utf8_text_decoding(self) -> None:
        """GitHub CLI output is decoded as UTF-8 instead of using the process locale."""

        old_run = post_github_review.subprocess.run
        seen_kwargs: dict[str, object] = {}

        def fake_run(args: list[str], **kwargs: object) -> object:
            """Record subprocess keyword arguments without launching a child process."""

            seen_kwargs.update(kwargs)
            return post_github_review.subprocess.CompletedProcess(args, 0, "{}", "")

        post_github_review.subprocess.run = fake_run
        self.addCleanup(setattr, post_github_review.subprocess, "run", old_run)

        post_github_review.run_command(["gh", "api"])

        self.assertTrue(seen_kwargs["text"])
        self.assertEqual(seen_kwargs["encoding"], "utf-8")

    def test_run_json_wraps_command_startup_failure(self) -> None:
        """Command startup errors become `FatalError` instead of tracebacks."""

        old_run = post_github_review.subprocess.run

        def fake_run(*_args: object, **_kwargs: object) -> object:
            """Pretend the executable exists but cannot be started."""

            raise OSError("bad executable")

        post_github_review.subprocess.run = fake_run
        self.addCleanup(setattr, post_github_review.subprocess, "run", old_run)

        with self.assertRaisesRegex(post_github_review.FatalError, "failed to execute"):
            post_github_review.run_json(["gh", "api"])

    def test_run_text_wraps_command_startup_failure(self) -> None:
        """Text-command startup errors become `FatalError` instead of tracebacks."""

        old_run = post_github_review.subprocess.run

        def fake_run(*_args: object, **_kwargs: object) -> object:
            """Pretend the executable exists but cannot be started."""

            raise OSError("bad executable")

        post_github_review.subprocess.run = fake_run
        self.addCleanup(setattr, post_github_review.subprocess, "run", old_run)

        with self.assertRaisesRegex(post_github_review.FatalError, "failed to execute"):
            post_github_review.run_text(["gh", "api"])

    def test_validate_locations_accepts_lines_in_patch(self) -> None:
        """Location validation accepts right-side context and added lines from a patch."""

        old_run_json = post_github_review.run_json

        def fake_run_json(args: list[str]) -> object:
            """Return enough fake GitHub API data to validate a successful location."""

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

    def test_validate_locations_uses_full_diff_when_file_patch_is_missing(self) -> None:
        """Location validation falls back when the PR files API omits a file patch."""

        old_run_json = post_github_review.run_json
        old_run_command = post_github_review.run_command

        def fake_run_json(args: list[str]) -> object:
            """Return PR metadata and a changed file entry without inline patch text."""

            joined = " ".join(args)
            if "/pulls/123" in joined and "/files" not in joined:
                return {"head": {"sha": "abc123"}}
            if "/pulls/123/files" in joined:
                return [[{"filename": "source/large-file.cpp"}]]
            raise AssertionError("unexpected gh api call: " + joined)

        def fake_run_command(args: list[str]) -> object:
            """Return the full PR diff that still contains the file's hunks."""

            joined = " ".join(args)
            self.assertIn("Accept: application/vnd.github.v3.diff", joined)
            self.assertIn("repos/shader-slang/slang/pulls/123", joined)
            return post_github_review.subprocess.CompletedProcess(
                args,
                0,
                stdout=(
                    "diff --git a/source/large-file.cpp b/source/large-file.cpp\n"
                    "--- a/source/large-file.cpp\n"
                    "+++ b/source/large-file.cpp\n"
                    "@@ -1,2 +20,3 @@\n"
                    " context\n"
                    "+added\n"
                    " context2\n"
                ),
                stderr="",
            )

        post_github_review.run_json = fake_run_json
        post_github_review.run_command = fake_run_command
        self.addCleanup(setattr, post_github_review, "run_json", old_run_json)
        self.addCleanup(setattr, post_github_review, "run_command", old_run_command)

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
                    "source/large-file.cpp",
                    20,
                    22,
                    "The invariant is unclear.",
                )
            ],
        )

        self.assertEqual(commit_id, "abc123")
        self.assertEqual(diff_lines["source/large-file.cpp"], {20, 21, 22})

    def test_full_diff_parser_keeps_added_lines_that_look_like_headers(self) -> None:
        """Added source text beginning with `++ ` must not reset the current file."""

        diff_lines = post_github_review.right_lines_from_full_diff(
            "diff --git a/source/file.cpp b/source/file.cpp\n"
            "--- a/source/file.cpp\n"
            "+++ b/source/file.cpp\n"
            "@@ -1,2 +10,3 @@\n"
            " context\n"
            "+++ b/not-the-current-file.cpp\n"
            "+after\n"
        )

        self.assertEqual(diff_lines["source/file.cpp"], {10, 11, 12})
        self.assertNotIn("not-the-current-file.cpp", diff_lines)

    def test_validate_locations_reports_full_diff_size_limit(self) -> None:
        """A GitHub full-diff size failure gets an actionable validation error."""

        old_run_json = post_github_review.run_json
        old_run_command = post_github_review.run_command

        def fake_run_json(args: list[str]) -> object:
            """Return PR metadata and a changed file entry without inline patch text."""

            joined = " ".join(args)
            if "/pulls/123" in joined and "/files" not in joined:
                return {"head": {"sha": "abc123"}}
            if "/pulls/123/files" in joined:
                return [[{"filename": "source/large-file.cpp"}]]
            raise AssertionError("unexpected gh api call: " + joined)

        def fake_run_command(args: list[str]) -> object:
            """Simulate GitHub rejecting the full diff because it is too large."""

            return post_github_review.subprocess.CompletedProcess(
                args,
                1,
                stdout="",
                stderr="HTTP 406: Sorry, the diff exceeded the maximum number of lines.",
            )

        post_github_review.run_json = fake_run_json
        post_github_review.run_command = fake_run_command
        self.addCleanup(setattr, post_github_review, "run_json", old_run_json)
        self.addCleanup(setattr, post_github_review, "run_command", old_run_command)

        with self.assertRaisesRegex(
            post_github_review.FatalError, "full PR diff endpoint.*too large"
        ):
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
                        "source/large-file.cpp",
                        20,
                        22,
                        "The invariant is unclear.",
                    )
                ],
            )

    def test_validate_locations_rejects_lines_not_in_patch(self) -> None:
        """Location validation rejects lines that GitHub cannot comment on."""

        old_run_json = post_github_review.run_json

        def fake_run_json(args: list[str]) -> object:
            """Return enough fake GitHub API data to validate a failing location."""

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
