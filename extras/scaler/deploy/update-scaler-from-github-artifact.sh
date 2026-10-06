#!/bin/bash
# Update the production scaler host from the latest GitHub Actions artifact.
#
# This script is intended to run on gpu-scaler-host, usually from
# scaler-auto-update.timer. It deliberately skips the update when the installed
# binary has the same MD5 as the published artifact, and when a service does not
# drain within DRAIN_TIMEOUT_SECONDS. Unattended deploys must never force-stop
# an active scaler and risk runner orphans.

set -euo pipefail

GITHUB_REPOSITORY="${GITHUB_REPOSITORY:-shader-slang/slang}"
SCALER_WORKFLOW_FILE="${SCALER_WORKFLOW_FILE:-scaler-release.yml}"
SCALER_ARTIFACT_NAME="${SCALER_ARTIFACT_NAME:-scaler-linux-master}"
SCALER_INSTALL_DIR="${SCALER_INSTALL_DIR:-/opt/scaler}"
SCALER_STATE_DIR="${SCALER_STATE_DIR:-/var/lib/scaler}"
SCALER_SERVICES="${SCALER_SERVICES:-scaler-windows scaler-linux scaler-linux-sm80plus scaler-windows-build scaler-linux-build scaler-linux-analytics}"
DRAIN_TIMEOUT_SECONDS="${DRAIN_TIMEOUT_SECONDS:-1500}"
LOCK_FILE="${SCALER_LOCK_FILE:-/tmp/scaler-auto-update.lock}"
GITHUB_API_VERSION="${GITHUB_API_VERSION:-2026-03-10}"
GITHUB_TOKEN="${GITHUB_TOKEN:-${GH_TOKEN:-${SCALER_TOKEN:-}}}"
CURL_CONNECT_TIMEOUT_SECONDS="${CURL_CONNECT_TIMEOUT_SECONDS:-10}"
CURL_SPEED_LIMIT_BYTES="${CURL_SPEED_LIMIT_BYTES:-1024}"
CURL_SPEED_TIME_SECONDS="${CURL_SPEED_TIME_SECONDS:-60}"

log() {
  printf '%s %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$*"
}

die() {
  log "ERROR: $*"
  exit 1
}

run_privileged() {
  if [ "$(id -u)" -eq 0 ]; then
    "$@"
  else
    sudo "$@"
  fi
}

github_api() {
  url="$1"
  if [ -n "$GITHUB_TOKEN" ]; then
    curl -fsSL --retry 3 --retry-delay 5 \
      --connect-timeout "$CURL_CONNECT_TIMEOUT_SECONDS" \
      --speed-limit "$CURL_SPEED_LIMIT_BYTES" \
      --speed-time "$CURL_SPEED_TIME_SECONDS" \
      -H "Accept: application/vnd.github+json" \
      -H "Authorization: Bearer ${GITHUB_TOKEN}" \
      -H "X-GitHub-Api-Version: ${GITHUB_API_VERSION}" \
      "$url"
  else
    curl -fsSL --retry 3 --retry-delay 5 \
      --connect-timeout "$CURL_CONNECT_TIMEOUT_SECONDS" \
      --speed-limit "$CURL_SPEED_LIMIT_BYTES" \
      --speed-time "$CURL_SPEED_TIME_SECONDS" \
      -H "Accept: application/vnd.github+json" \
      -H "X-GitHub-Api-Version: ${GITHUB_API_VERSION}" \
      "$url"
  fi
}

github_download() {
  url="$1"
  output="$2"
  if [ -n "$GITHUB_TOKEN" ]; then
    curl -fL --retry 3 --retry-delay 5 \
      --connect-timeout "$CURL_CONNECT_TIMEOUT_SECONDS" \
      --speed-limit "$CURL_SPEED_LIMIT_BYTES" \
      --speed-time "$CURL_SPEED_TIME_SECONDS" \
      -H "Accept: application/vnd.github+json" \
      -H "Authorization: Bearer ${GITHUB_TOKEN}" \
      -H "X-GitHub-Api-Version: ${GITHUB_API_VERSION}" \
      -o "$output" "$url"
  else
    curl -fL --retry 3 --retry-delay 5 \
      --connect-timeout "$CURL_CONNECT_TIMEOUT_SECONDS" \
      --speed-limit "$CURL_SPEED_LIMIT_BYTES" \
      --speed-time "$CURL_SPEED_TIME_SECONDS" \
      -H "Accept: application/vnd.github+json" \
      -H "X-GitHub-Api-Version: ${GITHUB_API_VERSION}" \
      -o "$output" "$url"
  fi
}

json_value() {
  manifest_file="$1"
  key_path="$2"
  python3 - "$manifest_file" "$key_path" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    value = json.load(f)

for key in sys.argv[2].split("."):
    value = value[key]

print(value)
PY
}

md5_file() {
  path="$1"
  if command -v md5sum >/dev/null 2>&1; then
    md5sum "$path" | awk '{print $1}'
  elif command -v md5 >/dev/null 2>&1; then
    md5 -q "$path"
  else
    die "md5sum or md5 is required"
  fi
}

sha256_file() {
  path="$1"
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$path" | awk '{print $1}'
  elif command -v shasum >/dev/null 2>&1; then
    shasum -a 256 "$path" | awk '{print $1}'
  else
    die "sha256sum or shasum is required"
  fi
}

is_active() {
  run_privileged systemctl is-active --quiet "$1"
}

start_previously_stopped() {
  stopped_services="$1"
  for svc in $stopped_services; do
    log "$svc: restarting after skipped update"
    run_privileged systemctl start "$svc" || true
  done
}

restart_previously_stopped() {
  stopped_services="$1"
  for svc in $stopped_services; do
    log "$svc: restarting after rollback"
    run_privileged systemctl restart "$svc" || run_privileged systemctl start "$svc" || true
  done
}

restore_after_post_drain_failure() {
  reason="$1"
  log "ERROR: ${reason}; restoring previous scaler state"

  if [ "$backup_available" = "1" ]; then
    run_privileged install -m 755 -o scaler -g scaler "$backup_binary" "$installed_binary" || true
  else
    log "no previous scaler binary was available to restore"
  fi

  restart_previously_stopped "$stopped_services"
  exit 1
}

write_current_state() {
  run_privileged cp "$manifest" "${SCALER_STATE_DIR}/current-manifest.json" || return 1
  printf '%s\n' "$commit" | run_privileged tee "${SCALER_STATE_DIR}/current-commit" >/dev/null ||
    return 1
  printf '%s\n' "$actual_md5" | run_privileged tee "${SCALER_STATE_DIR}/current-md5" >/dev/null ||
    return 1
  printf '%s\n' "$actual_sha256" | run_privileged tee "${SCALER_STATE_DIR}/current-sha256" >/dev/null ||
    return 1
}

write_current_state_or_restore() {
  if ! write_current_state; then
    restore_after_post_drain_failure "failed to write scaler state files"
  fi
}

start_updated_services_or_restore() {
  failed=0
  for svc in $stopped_services; do
    if run_privileged systemctl start "$svc"; then
      if is_active "$svc"; then
        log "$svc: active"
      else
        log "$svc: FAILED TO START"
        failed=1
      fi
    else
      log "$svc: FAILED TO START"
      failed=1
    fi
  done

  if [ "$failed" -ne 0 ]; then
    restore_after_post_drain_failure "one or more scaler services failed to start"
  fi
}

case "$DRAIN_TIMEOUT_SECONDS" in
'' | *[!0-9]*)
  die "DRAIN_TIMEOUT_SECONDS must be a non-negative integer"
  ;;
esac

exec 9>"$LOCK_FILE"
if command -v flock >/dev/null 2>&1; then
  if ! flock -n 9; then
    log "another scaler auto-update is already running"
    exit 0
  fi
fi

tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

runs_json="${tmp_dir}/runs.json"
artifacts_json="${tmp_dir}/artifacts.json"
artifact_zip="${tmp_dir}/artifact.zip"
artifact_dir="${tmp_dir}/artifact"
manifest="${artifact_dir}/manifest.json"
binary="${artifact_dir}/scaler-linux"

owner="${GITHUB_REPOSITORY%/*}"
repo="${GITHUB_REPOSITORY#*/}"
api_root="https://api.github.com/repos/${owner}/${repo}"
runs_url="${api_root}/actions/workflows/${SCALER_WORKFLOW_FILE}/runs?branch=master&event=push&status=completed&per_page=20"

log "checking latest successful ${SCALER_WORKFLOW_FILE} artifact"
github_api "$runs_url" >"$runs_json"

if ! run_info="$(
  python3 - "$runs_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    runs = json.load(f).get("workflow_runs", [])

for run in runs:
    if (
        run.get("conclusion") == "success"
        and run.get("head_branch") == "master"
        and run.get("head_sha")
    ):
        print(f"{run['id']} {run['head_sha']}")
        break
else:
    sys.exit(1)
PY
)"; then
  log "no successful master run found for ${SCALER_WORKFLOW_FILE}; nothing to deploy"
  exit 0
fi

run_id="${run_info%% *}"
run_head_sha="${run_info#* }"
artifacts_url="${api_root}/actions/runs/${run_id}/artifacts?per_page=100"
github_api "$artifacts_url" >"$artifacts_json"

if ! artifact_download_url="$(
  python3 - "$artifacts_json" "$SCALER_ARTIFACT_NAME" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    artifacts = json.load(f).get("artifacts", [])

name = sys.argv[2]
for artifact in artifacts:
    if artifact.get("name") == name and not artifact.get("expired", False):
        print(artifact["archive_download_url"])
        break
else:
    sys.exit(1)
PY
)"; then
  log "artifact ${SCALER_ARTIFACT_NAME} was not found on run ${run_id}; nothing to deploy"
  exit 0
fi

github_download "$artifact_download_url" "$artifact_zip"
mkdir -p "$artifact_dir"
python3 - "$artifact_zip" "$artifact_dir" <<'PY'
import os
import sys
import zipfile

zip_path, output_dir = sys.argv[1:3]
wanted = {"manifest.json", "scaler-linux", "scaler-linux.md5", "scaler-linux.sha256"}

with zipfile.ZipFile(zip_path) as zf:
    names = set(zf.namelist())
    missing = {"manifest.json", "scaler-linux"} - names
    if missing:
        raise SystemExit(f"artifact is missing required files: {sorted(missing)}")
    for name in wanted & names:
        with zf.open(name) as src, open(os.path.join(output_dir, name), "wb") as dst:
            dst.write(src.read())
PY

schema_version="$(json_value "$manifest" schema_version)"
repository="$(json_value "$manifest" repository)"
artifact_name="$(json_value "$manifest" artifact_name)"
commit="$(json_value "$manifest" commit)"
expected_md5="$(json_value "$manifest" binary.md5)"
expected_sha256="$(json_value "$manifest" binary.sha256)"

if [ "$schema_version" != "1" ]; then
  die "unsupported manifest schema_version ${schema_version}"
fi

if [ "$repository" != "$GITHUB_REPOSITORY" ]; then
  die "manifest repository ${repository} is not ${GITHUB_REPOSITORY}"
fi

if [ "$artifact_name" != "$SCALER_ARTIFACT_NAME" ]; then
  die "manifest artifact ${artifact_name} is not ${SCALER_ARTIFACT_NAME}"
fi

if [ "$commit" != "$run_head_sha" ]; then
  die "manifest commit ${commit} does not match workflow run head ${run_head_sha}"
fi

if ! printf '%s\n' "$commit" | grep -Eq '^[0-9a-f]{40}$'; then
  die "manifest commit ${commit} is not a full lowercase SHA-1"
fi

actual_md5="$(md5_file "$binary")"
if [ "$actual_md5" != "$expected_md5" ]; then
  die "md5 mismatch for downloaded scaler artifact"
fi

actual_sha256="$(sha256_file "$binary")"
if [ "$actual_sha256" != "$expected_sha256" ]; then
  die "sha256 mismatch for downloaded scaler artifact"
fi
chmod 755 "$binary"

installed_binary="${SCALER_INSTALL_DIR}/scaler"
backup_binary="${tmp_dir}/scaler.previous"
backup_available=0
if [ -f "$installed_binary" ]; then
  installed_md5="$(md5_file "$installed_binary")"
  if [ "$installed_md5" = "$expected_md5" ]; then
    log "installed scaler binary already matches ${commit} md5 ${expected_md5}; skipping restart"
    run_privileged install -d -m 755 "$SCALER_STATE_DIR"
    run_privileged cp "$manifest" "${SCALER_STATE_DIR}/current-manifest.json"
    printf '%s\n' "$commit" | run_privileged tee "${SCALER_STATE_DIR}/current-commit" >/dev/null
    printf '%s\n' "$expected_md5" | run_privileged tee "${SCALER_STATE_DIR}/current-md5" >/dev/null
    printf '%s\n' "$expected_sha256" | run_privileged tee "${SCALER_STATE_DIR}/current-sha256" >/dev/null
    exit 0
  fi

  run_privileged cp "$installed_binary" "$backup_binary"
  backup_available=1
fi

stopped_services=""
for svc in $SCALER_SERVICES; do
  if ! is_active "$svc"; then
    log "$svc: inactive, skipping drain"
    continue
  fi

  log "$svc: requesting drain (timeout ${DRAIN_TIMEOUT_SECONDS}s)"
  run_privileged systemctl reload "$svc" || true

  elapsed=0
  while [ "$elapsed" -lt "$DRAIN_TIMEOUT_SECONDS" ]; do
    if ! is_active "$svc"; then
      break
    fi
    sleep 5
    elapsed=$((elapsed + 5))
  done

  if is_active "$svc"; then
    log "$svc: still active after ${elapsed}s; leaving it to finish draining and skipping update"
    start_previously_stopped "$stopped_services"
    exit 0
  fi

  log "$svc: drained cleanly after ${elapsed}s"
  run_privileged systemctl stop "$svc" || true
  stopped_services="${stopped_services} ${svc}"
done

log "installing scaler ${commit}"
run_privileged install -d -m 755 "$SCALER_INSTALL_DIR" ||
  restore_after_post_drain_failure "failed to create scaler install directory"
run_privileged install -d -m 755 "$SCALER_STATE_DIR" ||
  restore_after_post_drain_failure "failed to create scaler state directory"
run_privileged install -m 755 -o scaler -g scaler "$binary" "${SCALER_INSTALL_DIR}/scaler" ||
  restore_after_post_drain_failure "failed to install scaler binary"

start_updated_services_or_restore
write_current_state_or_restore

log "scaler update complete: ${commit}"
