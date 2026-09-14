#!/usr/bin/env python3
"""Verify the Slang user-skill bundle in one or more release archives."""

from __future__ import annotations

import argparse
import json
import stat
import sys
import tarfile
import zipfile
from pathlib import Path, PurePosixPath
from typing import Callable


BUNDLE_ROOT = "share/slang/agent-skills"
PROVENANCE_PATH = f"{BUNDLE_ROOT}/PROVENANCE.json"
SOURCE_REPOSITORY = "https://github.com/shader-slang/slang-user-skills"


class VerificationError(Exception):
    """Reports a release archive that does not contain the expected bundle."""


def _normalize_archive_path(name: str) -> str:
    """Return a normalized safe archive path or raise VerificationError."""

    path = name.replace("\\", "/")
    while path.startswith("./"):
        path = path[2:]
    normalized = PurePosixPath(path)
    if normalized.is_absolute() or ".." in normalized.parts:
        raise VerificationError(f"archive contains an unsafe path: {name}")
    return normalized.as_posix()


def _is_bundle_path(path: str) -> bool:
    """Return whether an archive path is inside a possibly prefixed skills bundle."""

    return path.startswith(f"{BUNDLE_ROOT}/") or f"/{BUNDLE_ROOT}/" in path


def _expected_files(source_dir: Path) -> dict[str, bytes]:
    """Return the exact source file set, rejecting symlinks or an empty skills tree."""

    expected = {
        f"{BUNDLE_ROOT}/README.md": (source_dir / "README.md").read_bytes(),
        f"{BUNDLE_ROOT}/LICENSE": (source_dir / "LICENSE").read_bytes(),
    }
    skills_dir = source_dir / "skills"
    for source_path in sorted(skills_dir.rglob("*")):
        if source_path.is_symlink():
            raise VerificationError(f"skill bundles cannot contain symlinks: {source_path}")
        if not source_path.is_file():
            continue
        relative_path = source_path.relative_to(source_dir)
        # Keep this dot-component exclusion in sync with CMake's PATTERN ".*" EXCLUDE rule.
        if any(part.startswith(".") for part in relative_path.parts):
            continue
        expected[f"{BUNDLE_ROOT}/{relative_path.as_posix()}"] = source_path.read_bytes()
    if not any(name.startswith(f"{BUNDLE_ROOT}/skills/") for name in expected):
        raise VerificationError(f"no skill files found under {skills_dir}")
    return expected


def _verify_entries(
    archive_path: Path,
    entry_names: set[str],
    read_entry: Callable[[str], bytes],
    expected_files: dict[str, bytes],
    expected_commit: str,
) -> str:
    """Verify bundle bytes and provenance, then return its Slang version."""

    provenance_matches = [
        name
        for name in entry_names
        if name == PROVENANCE_PATH or name.endswith(f"/{PROVENANCE_PATH}")
    ]
    if len(provenance_matches) != 1:
        raise VerificationError(
            f"expected one {PROVENANCE_PATH} in {archive_path}, found "
            f"{len(provenance_matches)}"
        )

    provenance_name = provenance_matches[0]
    # A release archive may wrap its payload in one shared top-level directory. The unique
    # provenance path identifies the bundle and lets us strip that prefix before comparing its
    # complete file set with the prefix-free source layout.
    archive_prefix = provenance_name[: -len(PROVENANCE_PATH)]
    bundle_prefix = f"{archive_prefix}{BUNDLE_ROOT}/"
    actual_bundle_files = {
        name[len(archive_prefix) :]
        for name in entry_names
        if name.startswith(bundle_prefix)
    }
    expected_bundle_files = set(expected_files) | {PROVENANCE_PATH}
    if actual_bundle_files != expected_bundle_files:
        missing = sorted(expected_bundle_files - actual_bundle_files)
        unexpected = sorted(actual_bundle_files - expected_bundle_files)
        details = []
        if missing:
            details.append(f"missing: {', '.join(missing)}")
        if unexpected:
            details.append(f"unexpected: {', '.join(unexpected)}")
        raise VerificationError(f"bundle layout mismatch in {archive_path}: {'; '.join(details)}")

    for relative_name, expected_content in expected_files.items():
        packaged_name = f"{archive_prefix}{relative_name}"
        if read_entry(packaged_name) != expected_content:
            raise VerificationError(f"packaged file differs from source: {relative_name}")

    try:
        provenance = json.loads(read_entry(provenance_name).decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise VerificationError(f"invalid provenance in {archive_path}: {error}") from error
    expected_provenance = {
        "schemaVersion": 1,
        "sourceRepository": SOURCE_REPOSITORY,
        "sourceCommit": expected_commit,
    }
    for key, expected_value in expected_provenance.items():
        if provenance.get(key) != expected_value:
            raise VerificationError(
                f"provenance {key!r} is {provenance.get(key)!r}, expected {expected_value!r}"
            )
    slang_version = provenance.get("slangVersion")
    if not isinstance(slang_version, str) or not slang_version:
        raise VerificationError("provenance slangVersion must be a non-empty string")
    return slang_version


def _verify_zip(
    archive_path: Path, expected_files: dict[str, bytes], expected_commit: str
) -> str:
    """Verify a ZIP skills bundle and return its provenance Slang version."""

    with zipfile.ZipFile(archive_path) as archive:
        entries: dict[str, zipfile.ZipInfo] = {}
        seen_paths: set[str] = set()
        for entry in archive.infolist():
            normalized_name = _normalize_archive_path(entry.filename)
            if normalized_name in seen_paths:
                raise VerificationError(f"duplicate archive path: {normalized_name}")
            seen_paths.add(normalized_name)
            if entry.is_dir():
                continue
            # Unix-origin ZIP entries encode a file type in external_attr; zero means that no
            # type bits were supplied. Other ZIP creators expose no portable symlink bit here,
            # so their members are treated as file data. As with TAR, only bundle paths are in
            # scope for the non-regular-entry guarantee.
            unix_file_type = stat.S_IFMT(entry.external_attr >> 16)
            if (
                entry.create_system == 3
                and unix_file_type not in (0, stat.S_IFREG)
                and _is_bundle_path(normalized_name)
            ):
                raise VerificationError(
                    f"bundle contains a non-regular archive entry: {normalized_name}"
                )
            entries[normalized_name] = entry
        return _verify_entries(
            archive_path,
            set(entries),
            lambda name: archive.read(entries[name]),
            expected_files,
            expected_commit,
        )


def _verify_tar(
    archive_path: Path, expected_files: dict[str, bytes], expected_commit: str
) -> str:
    """Verify a TAR skills bundle and return its provenance Slang version."""

    with tarfile.open(archive_path, "r:*") as archive:
        entries: dict[str, tarfile.TarInfo] = {}
        seen_paths: set[str] = set()
        for entry in archive.getmembers():
            normalized_name = _normalize_archive_path(entry.name)
            if normalized_name in seen_paths:
                raise VerificationError(f"duplicate archive path: {normalized_name}")
            seen_paths.add(normalized_name)
            if (
                not entry.isfile()
                and not entry.isdir()
                # This verifier vouches only for entry types inside the skills bundle.
                and _is_bundle_path(normalized_name)
            ):
                raise VerificationError(
                    f"bundle contains a non-regular archive entry: {normalized_name}"
                )
            if not entry.isfile():
                continue
            entries[normalized_name] = entry

        def read_entry(name: str) -> bytes:
            """Read one regular TAR entry or raise VerificationError."""

            extracted = archive.extractfile(entries[name])
            if extracted is None:
                raise VerificationError(f"could not read archive path: {name}")
            return extracted.read()

        return _verify_entries(
            archive_path,
            set(entries),
            read_entry,
            expected_files,
            expected_commit,
        )


def main() -> int:
    """Verify command-line archives and return a process exit status."""

    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--expected-commit", required=True)
    parser.add_argument("archives", nargs="+", type=Path)
    args = parser.parse_args()

    if len(args.expected_commit) != 40 or any(
        character not in "0123456789abcdefABCDEF" for character in args.expected_commit
    ):
        parser.error("--expected-commit must be a full 40-character Git commit SHA")

    try:
        expected_files = _expected_files(args.source_dir)
        for archive_path in args.archives:
            if zipfile.is_zipfile(archive_path):
                slang_version = _verify_zip(
                    archive_path, expected_files, args.expected_commit.lower()
                )
            elif tarfile.is_tarfile(archive_path):
                slang_version = _verify_tar(
                    archive_path, expected_files, args.expected_commit.lower()
                )
            else:
                raise VerificationError(f"unsupported release archive: {archive_path}")
            print(
                f"Verified Slang user skills in {archive_path} "
                f"(skills {args.expected_commit.lower()}, Slang {slang_version})"
            )
    except (OSError, VerificationError, zipfile.BadZipFile, tarfile.TarError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
