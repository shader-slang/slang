"""Live BigQuery loader for the Phase 1 ingester.

Loads NDJSON-per-table into per-attempt staging tables, runs an atomic
multi-statement MERGE script against the live tables, then cleans up.

Naming contract for staging tables:

    slang_ci_staging.<table>__src_<src_run_id>_<src_run_attempt>__ing_<ing_run_id>_<ing_run_attempt>

`src_*` is the triggering CI run identity (the run we are ingesting).
`ing_*` is this ingester's own identity. In production both come from
GITHUB_RUN_ID / GITHUB_RUN_ATTEMPT environment variables; for manual CLI
invocations the ingester identity falls back to "manual" + a unix
timestamp so simultaneous CLI runs cannot collide.

Why both: a re-run of ci-telemetry.yml against the same CI run gets a
new ing_* but reuses src_*, so two concurrent ingest jobs cannot share
staging tables. The live row_keys are deterministic, so the eventual
MERGE converges to the same final state regardless of which attempt
wins.
"""

from __future__ import annotations

import json
import os
import random
import re
import secrets
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import Any, Iterable

# Caller-validated suffix shape — anything else risks SQL injection via the
# `str.format()`-templated MERGE script.
_SAFE_TOKEN = re.compile(r"^[A-Za-z0-9_]+$")

# The plan's BigQuery project. The MERGE template hardcodes the same name,
# so making this a parameter would create a footgun (loads in one project,
# MERGE against another). One constant is the simplest honest expression
# of "we have exactly one prod project."
PROJECT = "slang-runners"

# Per-subprocess wall-clock cap for `bq` invocations. The job-level
# `timeout-minutes` in ci-telemetry.yml is the outer bound, but it kills the
# whole process group, which would skip the finally-cleanup of staging tables.
# A per-process timeout lets the loader's exception path run (drop_staging),
# at the cost of failing the ingest on a stalled CLI/network. Override via
# the BQ_CMD_TIMEOUT_S env var if a particular run needs longer.
_BQ_CMD_TIMEOUT_S = int(os.environ.get("BQ_CMD_TIMEOUT_S", "600"))


def staging_suffix(
    *,
    src_run_id: int,
    src_run_attempt: int,
    ing_run_id: str | int,
    ing_run_attempt: str | int,
) -> str:
    """Build the canonical staging-table suffix.

    Format: `src_<src_run_id>_<src_run_attempt>__ing_<ing_run_id>_<ing_run_attempt>`.
    Every component is restricted to `[A-Za-z0-9_]+` because the suffix flows
    into a `str.format()`-templated SQL script.
    """
    parts = (
        "src",
        str(src_run_id),
        str(src_run_attempt),
        "_ing",  # leading underscore yields the `__ing_` double separator
        str(ing_run_id),
        str(ing_run_attempt),
    )
    suffix = "_".join(parts)
    if not _SAFE_TOKEN.match(suffix):
        raise ValueError(
            f"staging suffix {suffix!r} contains characters outside [A-Za-z0-9_]"
        )
    return suffix


def derive_ingester_identity() -> tuple[str, str]:
    """Return (ing_run_id, ing_run_attempt) for the current process.

    Inside GitHub Actions, GITHUB_RUN_ID and GITHUB_RUN_ATTEMPT are set; both
    are integers but we keep them as strings since they flow into the staging
    suffix. For manual CLI invocations, fall back to a hybrid identity
    `manual<unix_ms>p<pid>r<random_hex>` and attempt=1. The random suffix is
    what makes two invocations within the same millisecond — same `time()`,
    same `pid` after a quick fork-exec — still get distinct staging tables.
    """
    run_id = os.environ.get("GITHUB_RUN_ID")
    run_attempt = os.environ.get("GITHUB_RUN_ATTEMPT")
    if run_id and run_attempt:
        return run_id, run_attempt
    ms = int(time.time() * 1000)
    rand_suffix = secrets.token_hex(4)  # 8 hex chars; 32 bits of randomness
    return f"manual{ms}p{os.getpid()}r{rand_suffix}", "1"


# ---------- bq CLI wrappers ----------


def _bq_run(args: list[str]) -> subprocess.CompletedProcess:
    """Run a `bq` CLI command. The bq CLI is available wherever the gcloud SDK
    is installed; for production we'll be inside the google-github-actions/auth
    + setup-gcloud composite, which also installs bq.

    On non-zero exit, surface the captured stderr to the calling process's
    stderr so failures are debuggable. Without this, bq's error message is
    swallowed by subprocess.run(check=True) and only the command line appears
    in the traceback.
    """
    proc = subprocess.run(
        ["bq", *args],
        capture_output=True,
        text=True,
        timeout=_BQ_CMD_TIMEOUT_S,
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        proc.check_returncode()
    return proc


_schema_cache: dict[str, list[dict[str, Any]]] = {}


def _live_schema_path(table: str, *, schema_dir: Path) -> Path:
    """Return a path to a JSON file containing the live table's schema, fit
    for `bq load --schema=<path>`. Cached per table within the process so
    repeated loads in one ingest only hit `bq show` once.
    """
    if table not in _schema_cache:
        proc = subprocess.run(
            [
                "bq",
                "show",
                "--format=prettyjson",
                "--schema",
                f"{PROJECT}:slang_ci.{table}",
            ],
            check=True,
            capture_output=True,
            text=True,
            timeout=_BQ_CMD_TIMEOUT_S,
        )
        _schema_cache[table] = json.loads(proc.stdout)
    schema_path = schema_dir / f"{table}.schema.json"
    schema_path.write_text(json.dumps(_schema_cache[table]))
    return schema_path


def load_ndjson_to_staging(
    *,
    table: str,
    ndjson_path: Path,
    suffix: str,
    schema_dir: Path,
) -> str:
    """`bq load` an NDJSON file into a new per-attempt staging table.

    Returns the fully-qualified staging table identifier.

    Each load passes an explicit `--schema` argument derived from the live
    table's current schema, so the staging table inherits the same shape.
    BigQuery cannot infer the schema of an empty NDJSON file, so this is
    mandatory anyway.
    """
    if not _SAFE_TOKEN.match(table):
        raise ValueError(f"bad table name {table!r}")
    staging_id = f"{PROJECT}:slang_ci_staging.{table}__{suffix}"
    schema_path = _live_schema_path(table, schema_dir=schema_dir)
    # --replace lets retries clobber a leftover staging table from a previous
    # crash; the suffix already encodes attempt identity so this is correct.
    _bq_run(
        [
            "load",
            "--source_format=NEWLINE_DELIMITED_JSON",
            "--replace",
            f"--schema={schema_path}",
            staging_id,
            str(ndjson_path),
        ]
    )
    return staging_id


def drop_staging_tables(suffix: str, *, tables: Iterable[str]) -> None:
    """Drop the per-attempt staging tables, swallowing per-table errors so the
    cleanup pass always tries every table. Called from a finally: clause, so
    it must tolerate the "table never existed because its load failed" case.
    `bq rm -f` already exits 0 when the table is missing.
    """
    for table in tables:
        if not _SAFE_TOKEN.match(table):
            continue
        staging_id = f"{PROJECT}:slang_ci_staging.{table}__{suffix}"
        try:
            _bq_run(["rm", "-f", "-t", staging_id])
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
            # Swallow per-table failures so the loop continues — the goal is
            # to attempt cleanup of every staging table even if one is stuck.
            # CalledProcessError handles bq-side errors (auth, permissions);
            # TimeoutExpired handles a hung bq subprocess after we added the
            # _BQ_CMD_TIMEOUT_S guard. The staging dataset's 1-day default
            # expiration sweeps anything we miss.
            sys.stderr.write(
                f"warning: failed to drop staging table {staging_id}: {e}\n"
            )


# ---------- Transactional MERGE with retry ----------


# BigQuery returns a specific error string for cancelled mutating transactions.
# We match on substring rather than error code because the bq CLI surfaces the
# message body in non-zero exit text.
_TXN_CONFLICT_MARKERS = (
    "Transaction is aborted",
    "concurrent update",
    "Could not serialize access",
    "TRANSACTION_ABORTED",
)


def _is_transaction_conflict(stderr: str) -> bool:
    return any(m in stderr for m in _TXN_CONFLICT_MARKERS)


def run_merge_with_retry(
    *,
    sql: str,
    max_attempts: int = 7,
) -> None:
    """Run the multi-statement MERGE transaction with conflict retry.

    Plan retry budget: 7 attempts, sleep `random.uniform(0, 2**attempt)`
    between attempts 1..6, worst case ~126s of cumulative sleep. After
    exhaustion the call raises so the GH Actions job fails loudly.

    The SQL is fed through stdin rather than as a positional argument
    because `bq query` argv-parses any positional containing `=` against
    its flag list (a runaway suggestion-distance algorithm in absl
    blows the stack on a multi-statement MERGE).
    """
    # Server-side job timeout in ms, matched to the client-side
    # subprocess timeout so both sides agree. Killing the local `bq`
    # process does not by itself cancel an already-submitted BigQuery job;
    # --job_timeout_ms makes the BQ server give up on the query if it
    # exceeds its own deadline. Cancellation is best-effort per Google's
    # docs but it bounds the orphan-job blast radius.
    job_timeout_ms = _BQ_CMD_TIMEOUT_S * 1000

    for attempt in range(1, max_attempts + 1):
        proc = subprocess.run(
            [
                "bq",
                "query",
                "--use_legacy_sql=false",
                "--project_id",
                PROJECT,
                f"--job_timeout_ms={job_timeout_ms}",
                "--format=none",
            ],
            input=sql,
            capture_output=True,
            text=True,
            timeout=_BQ_CMD_TIMEOUT_S,
        )
        if proc.returncode == 0:
            return
        if attempt < max_attempts and _is_transaction_conflict(proc.stderr):
            sleep_s = random.uniform(0, 2**attempt)
            sys.stderr.write(
                f"MERGE transaction conflict on attempt {attempt}; "
                f"sleeping {sleep_s:.2f}s and retrying.\n"
            )
            time.sleep(sleep_s)
            continue
        # Non-conflict error, or budget exhausted on a conflict.
        sys.stderr.write(proc.stderr)
        raise subprocess.CalledProcessError(
            proc.returncode, proc.args, proc.stdout, proc.stderr
        )


# ---------- Top-level loader ----------


# Tables this Phase 1 slice ingests. Order matters only for the MERGE SQL
# template's DECLARE block, not here.
_PHASE1_TABLES = ("workflow_runs", "jobs", "job_steps")


def load_rows_to_bq(
    rows: dict[str, list[dict[str, Any]]],
    *,
    src_run_id: int,
    src_run_attempt: int,
    merge_template_path: Path | None = None,
) -> None:
    """End-to-end: write per-table NDJSON, load into per-attempt staging,
    run the atomic MERGE with retry, then drop staging tables in a finally
    clause.

    No-ops on tables with zero rows in `rows` — the MERGE template still
    references them, so an empty NDJSON file is `bq load`-ed and the
    MIN/MAX-derived partition predicate degrades to (NULL, NULL), which the
    MERGE BETWEEN clause correctly evaluates to "no rows match".
    """
    ing_run_id, ing_run_attempt = derive_ingester_identity()
    suffix = staging_suffix(
        src_run_id=src_run_id,
        src_run_attempt=src_run_attempt,
        ing_run_id=ing_run_id,
        ing_run_attempt=ing_run_attempt,
    )

    if merge_template_path is None:
        merge_template_path = (
            Path(__file__).resolve().parent
            / "bq_schema"
            / "merge_live_tables.sql.tmpl"
        )
    sql = merge_template_path.read_text().format(staging_suffix=suffix)

    with tempfile.TemporaryDirectory(prefix="bq-stage-") as tmp:
        tmp_dir = Path(tmp)
        # try/finally wraps both the staging-load phase and the MERGE phase
        # so a failure partway through the loads still triggers cleanup.
        # bq load --replace creates the staging tables as a side effect, so
        # any table we attempted to load may already exist when we land here.
        try:
            for table in _PHASE1_TABLES:
                table_rows = rows.get(table, [])
                ndjson_path = tmp_dir / f"{table}.ndjson"
                with ndjson_path.open("w") as f:
                    for r in table_rows:
                        f.write(json.dumps(r))
                        f.write("\n")
                load_ndjson_to_staging(
                    table=table,
                    ndjson_path=ndjson_path,
                    suffix=suffix,
                    schema_dir=tmp_dir,
                )
            run_merge_with_retry(sql=sql)
        finally:
            # Always clean up staging tables — they're disposable and the
            # staging dataset has a 1-day default expiration as a safety net,
            # but explicit drop keeps things tidy. drop_staging_tables
            # tolerates "table not found" so it's safe to call even when a
            # load failed before creating a given table.
            drop_staging_tables(suffix, tables=_PHASE1_TABLES)
