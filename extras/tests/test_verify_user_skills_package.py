#!/usr/bin/env python3
"""Tests for the Slang user-skills release-package verifier."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import json
import os
import stat
import sys
import tarfile
import tempfile
import unittest
import warnings
import zipfile
from pathlib import Path
from unittest import mock


EXTRAS_DIR = Path(__file__).resolve().parents[1]
VERIFIER_PATH = EXTRAS_DIR / "verify-user-skills-package.py"
SPEC = importlib.util.spec_from_file_location("verify_user_skills_package", VERIFIER_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"could not load verifier from {VERIFIER_PATH}")
verifier = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(verifier)

EXPECTED_COMMIT = "0123456789abcdef0123456789abcdef01234567"
SLANG_VERSION = "2099.1-test"


class TestUserSkillsPackageVerifier(unittest.TestCase):
    """Exercises valid bundles and every independently enforced rejection path."""

    def setUp(self) -> None:
        """Create a minimal source bundle for one test."""

        self.temporary_directory = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary_directory.name)
        self.source_dir = self.root / "source"
        skill_dir = self.source_dir / "skills" / "example"
        reference_dir = skill_dir / "references"
        reference_dir.mkdir(parents=True)
        (self.source_dir / "README.md").write_text("skills readme\n", encoding="utf-8")
        (self.source_dir / "LICENSE").write_text("test license\n", encoding="utf-8")
        (skill_dir / "SKILL.md").write_text("# Example skill\n", encoding="utf-8")
        (reference_dir / "example.slang").write_text("void example() {}\n", encoding="utf-8")
        self.expected_files = verifier._expected_files(self.source_dir)

    def tearDown(self) -> None:
        """Remove the temporary source tree and archives."""

        self.temporary_directory.cleanup()

    def _provenance(self, **updates: object) -> bytes:
        """Return encoded provenance with selected fields overridden."""

        provenance = {
            "schemaVersion": 1,
            "sourceRepository": verifier.SOURCE_REPOSITORY,
            "sourceCommit": EXPECTED_COMMIT,
            "slangVersion": SLANG_VERSION,
        }
        provenance.update(updates)
        return json.dumps(provenance).encode("utf-8")

    def _valid_entries(self, prefix: str = "") -> dict[str, bytes]:
        """Return a complete valid archive file map with an optional shared prefix."""

        entries = {f"{prefix}{name}": content for name, content in self.expected_files.items()}
        entries[f"{prefix}{verifier.PROVENANCE_PATH}"] = self._provenance()
        return entries

    def _write_zip(self, name: str, entries: list[tuple[str, bytes]]) -> Path:
        """Write a ZIP containing the supplied entries and return its path."""

        archive_path = self.root / name
        with zipfile.ZipFile(archive_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            with warnings.catch_warnings():
                warnings.simplefilter("ignore", UserWarning)
                for entry_name, content in entries:
                    archive.writestr(entry_name, content)
        return archive_path

    def _write_tar(self, name: str, entries: list[tuple[str, bytes]]) -> Path:
        """Write a compressed TAR containing the supplied entries and return its path."""

        archive_path = self.root / name
        with tarfile.open(archive_path, "w:gz") as archive:
            for entry_name, content in entries:
                entry = tarfile.TarInfo(entry_name)
                entry.size = len(content)
                entry.mode = 0o644
                archive.addfile(entry, io.BytesIO(content))
        return archive_path

    def test_accepts_zip_and_tar_with_or_without_prefix(self) -> None:
        """Both archive formats support bare payloads and one shared top-level prefix."""

        cases = (
            ("bare.zip", "", self._write_zip, verifier._verify_zip),
            ("prefixed.zip", "slang-test/", self._write_zip, verifier._verify_zip),
            ("bare.tar.gz", "", self._write_tar, verifier._verify_tar),
            ("prefixed.tar.gz", "slang-test/", self._write_tar, verifier._verify_tar),
        )
        for name, prefix, writer, verify in cases:
            with self.subTest(name=name):
                archive_path = writer(name, list(self._valid_entries(prefix).items()))
                self.assertEqual(
                    verify(archive_path, self.expected_files, EXPECTED_COMMIT),
                    SLANG_VERSION,
                )

    def test_rejects_missing_extra_and_modified_bundle_files(self) -> None:
        """The packaged file set and bytes must match the source exactly."""

        skill_path = f"{verifier.BUNDLE_ROOT}/skills/example/SKILL.md"
        cases: list[tuple[str, dict[str, bytes], str]] = []

        missing = self._valid_entries()
        missing.pop(skill_path)
        cases.append(("missing.zip", missing, "bundle layout mismatch"))

        extra = self._valid_entries()
        extra[f"{verifier.BUNDLE_ROOT}/skills/example/extra.txt"] = b"unexpected\n"
        cases.append(("extra.zip", extra, "bundle layout mismatch"))

        modified = self._valid_entries()
        modified[skill_path] = b"modified\n"
        cases.append(("modified.zip", modified, "packaged file differs from source"))

        for name, entries, expected_message in cases:
            with self.subTest(name=name):
                archive_path = self._write_zip(name, list(entries.items()))
                with self.assertRaisesRegex(verifier.VerificationError, expected_message):
                    verifier._verify_zip(archive_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_invalid_provenance_fields(self) -> None:
        """Every provenance identity field and the Slang version are mandatory."""

        cases = (
            ("schemaVersion", 2, "schemaVersion"),
            ("sourceRepository", "https://example.invalid/skills", "sourceRepository"),
            ("sourceCommit", "f" * 40, "sourceCommit"),
            ("slangVersion", "", "slangVersion"),
        )
        for field, value, expected_message in cases:
            with self.subTest(field=field):
                entries = self._valid_entries()
                entries[verifier.PROVENANCE_PATH] = self._provenance(**{field: value})
                archive_path = self._write_zip(f"bad-{field}.zip", list(entries.items()))
                with self.assertRaisesRegex(verifier.VerificationError, expected_message):
                    verifier._verify_zip(archive_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_invalid_and_missing_provenance(self) -> None:
        """Provenance must appear exactly once and contain UTF-8 JSON."""

        invalid = self._valid_entries()
        invalid[verifier.PROVENANCE_PATH] = b"{not-json"
        invalid_path = self._write_zip("invalid-provenance.zip", list(invalid.items()))
        with self.assertRaisesRegex(verifier.VerificationError, "invalid provenance"):
            verifier._verify_zip(invalid_path, self.expected_files, EXPECTED_COMMIT)

        missing = self._valid_entries()
        missing.pop(verifier.PROVENANCE_PATH)
        missing_path = self._write_zip("missing-provenance.zip", list(missing.items()))
        with self.assertRaisesRegex(verifier.VerificationError, "expected one"):
            verifier._verify_zip(missing_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_unsafe_archive_paths(self) -> None:
        """Absolute paths and parent traversal are rejected before bundle inspection."""

        for unsafe_path in ("/absolute/path", "../escape", "prefix/../../escape"):
            with self.subTest(path=unsafe_path):
                with self.assertRaisesRegex(verifier.VerificationError, "unsafe path"):
                    verifier._normalize_archive_path(unsafe_path)

        self.assertEqual(
            verifier._normalize_archive_path("./share\\slang/agent-skills/README.md"),
            "share/slang/agent-skills/README.md",
        )

    def test_rejects_duplicate_zip_and_tar_entries(self) -> None:
        """Duplicate normalized paths are ambiguous and rejected in both formats."""

        entries = list(self._valid_entries().items())
        entries.append((verifier.PROVENANCE_PATH, self._provenance()))
        cases = (
            (self._write_zip("duplicate.zip", entries), verifier._verify_zip),
            (self._write_tar("duplicate.tar.gz", entries), verifier._verify_tar),
        )
        for archive_path, verify in cases:
            with self.subTest(archive=archive_path.name):
                with self.assertRaisesRegex(verifier.VerificationError, "duplicate archive path"):
                    verify(archive_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_unsafe_entries_in_zip_and_tar(self) -> None:
        """Archive readers normalize every entry, including files outside the bundle."""

        entries = list(self._valid_entries().items())
        entries.append(("../escape", b"unsafe\n"))
        cases = (
            (self._write_zip("unsafe.zip", entries), verifier._verify_zip),
            (self._write_tar("unsafe.tar.gz", entries), verifier._verify_tar),
        )
        for archive_path, verify in cases:
            with self.subTest(archive=archive_path.name):
                with self.assertRaisesRegex(verifier.VerificationError, "unsafe path"):
                    verify(archive_path, self.expected_files, EXPECTED_COMMIT)

        unsafe_directory_path = self.root / "unsafe-directory.zip"
        with zipfile.ZipFile(unsafe_directory_path, "w") as archive:
            for entry_name, content in self._valid_entries().items():
                archive.writestr(entry_name, content)
            archive.writestr("../escape/", b"")
        with self.assertRaisesRegex(verifier.VerificationError, "unsafe path"):
            verifier._verify_zip(
                unsafe_directory_path, self.expected_files, EXPECTED_COMMIT
            )

    def test_rejects_directory_file_path_collisions(self) -> None:
        """Directories and files cannot share one normalized path in either format."""

        collision_name = f"{verifier.BUNDLE_ROOT}/skills/example/collision"
        zip_path = self.root / "collision.zip"
        with zipfile.ZipFile(zip_path, "w") as archive:
            for entry_name, content in self._valid_entries().items():
                archive.writestr(entry_name, content)
            archive.writestr(f"{collision_name}/", b"")
            archive.writestr(collision_name, b"collision\n")

        tar_path = self.root / "collision.tar.gz"
        with tarfile.open(tar_path, "w:gz") as archive:
            for entry_name, content in self._valid_entries().items():
                entry = tarfile.TarInfo(entry_name)
                entry.size = len(content)
                archive.addfile(entry, io.BytesIO(content))
            directory = tarfile.TarInfo(f"{collision_name}/")
            directory.type = tarfile.DIRTYPE
            archive.addfile(directory)
            file_entry = tarfile.TarInfo(collision_name)
            file_entry.size = 1
            archive.addfile(file_entry, io.BytesIO(b"x"))

        for archive_path, verify in (
            (zip_path, verifier._verify_zip),
            (tar_path, verifier._verify_tar),
        ):
            with self.subTest(archive=archive_path.name):
                with self.assertRaisesRegex(verifier.VerificationError, "duplicate archive path"):
                    verify(archive_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_non_regular_zip_bundle_entries(self) -> None:
        """A ZIP symlink cannot stand in for a regular bundled file."""

        archive_path = self.root / "symlink.zip"
        with zipfile.ZipFile(archive_path, "w") as archive:
            for entry_name, content in self._valid_entries().items():
                archive.writestr(entry_name, content)
            link = zipfile.ZipInfo(f"{verifier.BUNDLE_ROOT}/skills/example/link")
            link.create_system = 3
            link.external_attr = (stat.S_IFLNK | 0o777) << 16
            archive.writestr(link, "SKILL.md")

        with self.assertRaisesRegex(verifier.VerificationError, "non-regular archive entry"):
            verifier._verify_zip(archive_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_non_regular_tar_bundle_entries(self) -> None:
        """A TAR symlink cannot stand in for a regular bundled file."""

        archive_path = self.root / "symlink.tar.gz"
        with tarfile.open(archive_path, "w:gz") as archive:
            for entry_name, content in self._valid_entries().items():
                entry = tarfile.TarInfo(entry_name)
                entry.size = len(content)
                archive.addfile(entry, io.BytesIO(content))
            link = tarfile.TarInfo(f"{verifier.BUNDLE_ROOT}/skills/example/link")
            link.type = tarfile.SYMTYPE
            link.linkname = "SKILL.md"
            archive.addfile(link)

        with self.assertRaisesRegex(verifier.VerificationError, "non-regular archive entry"):
            verifier._verify_tar(archive_path, self.expected_files, EXPECTED_COMMIT)

    def test_rejects_source_symlinks(self) -> None:
        """A symlink in the source skills tree is never accepted as package input."""

        link_path = self.source_dir / "skills" / "example" / "linked-skill.md"
        try:
            os.symlink("SKILL.md", link_path)
        except OSError as error:
            self.skipTest(f"creating symlinks is unavailable: {error}")
        with self.assertRaisesRegex(verifier.VerificationError, "cannot contain symlinks"):
            verifier._expected_files(self.source_dir)

    def test_excludes_dot_components_from_expected_files(self) -> None:
        """Hidden files and files below hidden directories mirror the CMake exclusion."""

        hidden_dir = self.source_dir / "skills" / "example" / ".cache"
        hidden_dir.mkdir()
        (hidden_dir / "generated.txt").write_text("ignored\n", encoding="utf-8")
        (self.source_dir / "skills" / "example" / ".hidden").write_text(
            "ignored\n", encoding="utf-8"
        )
        self.assertEqual(verifier._expected_files(self.source_dir), self.expected_files)

    def test_rejects_source_tree_without_skill_files(self) -> None:
        """README and license files cannot make an empty skills tree valid."""

        source_dir = self.root / "empty-source"
        hidden_dir = source_dir / "skills" / ".hidden"
        hidden_dir.mkdir(parents=True)
        (source_dir / "README.md").write_text("readme\n", encoding="utf-8")
        (source_dir / "LICENSE").write_text("license\n", encoding="utf-8")
        (hidden_dir / "ignored.md").write_text("ignored\n", encoding="utf-8")
        with self.assertRaisesRegex(verifier.VerificationError, "no skill files found"):
            verifier._expected_files(source_dir)

    def test_main_dispatches_both_archive_formats(self) -> None:
        """The command-line entry point recognizes and verifies ZIP and TAR archives."""

        zip_path = self._write_zip("main.zip", list(self._valid_entries().items()))
        tar_path = self._write_tar(
            "main.tar.gz", list(self._valid_entries("slang-test/").items())
        )
        arguments = [
            str(VERIFIER_PATH),
            "--source-dir",
            str(self.source_dir),
            "--expected-commit",
            EXPECTED_COMMIT,
            str(zip_path),
            str(tar_path),
        ]
        output = io.StringIO()
        with mock.patch.object(sys, "argv", arguments), contextlib.redirect_stdout(output):
            self.assertEqual(verifier.main(), 0)
        self.assertEqual(output.getvalue().count("Verified Slang user skills"), 2)

    def test_main_rejects_malformed_expected_commit(self) -> None:
        """The command-line entry point requires a full hexadecimal commit SHA."""

        archive_path = self._write_zip(
            "malformed-sha.zip", list(self._valid_entries().items())
        )
        arguments = [
            str(VERIFIER_PATH),
            "--source-dir",
            str(self.source_dir),
            "--expected-commit",
            "not-a-commit",
            str(archive_path),
        ]
        error_output = io.StringIO()
        with mock.patch.object(sys, "argv", arguments), contextlib.redirect_stderr(
            error_output
        ):
            with self.assertRaises(SystemExit) as exit_context:
                verifier.main()
        self.assertEqual(exit_context.exception.code, 2)
        self.assertIn("full 40-character Git commit SHA", error_output.getvalue())

    def test_main_reports_unsupported_archive(self) -> None:
        """Unsupported files use the verifier's diagnostic and failure return path."""

        archive_path = self.root / "not-an-archive.txt"
        archive_path.write_text("not an archive\n", encoding="utf-8")
        arguments = [
            str(VERIFIER_PATH),
            "--source-dir",
            str(self.source_dir),
            "--expected-commit",
            EXPECTED_COMMIT,
            str(archive_path),
        ]
        error_output = io.StringIO()
        with mock.patch.object(sys, "argv", arguments), contextlib.redirect_stderr(
            error_output
        ):
            self.assertEqual(verifier.main(), 1)
        self.assertIn("unsupported release archive", error_output.getvalue())


if __name__ == "__main__":
    unittest.main()
