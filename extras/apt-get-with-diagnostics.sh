#!/bin/bash

# Run apt-get normally, preserving its exit status if diagnostics also fail.
# For example, a background apt-get update can hold the lists lock while a
# fresh runner starts CI. Capture its ancestry immediately to identify which
# boot service or job launched it, without dumping unrelated process arguments.
set -uo pipefail

if sudo apt-get "$@"; then
  exit 0
else
  apt_status=$?
fi

diagnose_locks() {
  local locks pid ancestor parent depth
  echo "::group::APT failure: package-manager lock owners"
  # lslocks reports actual kernel locks, rather than every process with the
  # file open. Inspect only the package-manager locks, not a host-wide dump.
  if ! locks=$(sudo -n lslocks --raw --noheadings --notruncate --output PID,PATH | awk '
    $2 == "/var/lib/apt/lists/lock" ||
    $2 == "/var/cache/apt/archives/lock" ||
    $2 == "/var/lib/dpkg/lock" ||
    $2 == "/var/lib/dpkg/lock-frontend" { print }
  '); then
    echo "Unable to inspect package-manager locks."
    echo "::endgroup::"
    return
  fi
  if [ -z "$locks" ]; then
    echo "No package-manager lock owners captured (the holder may have exited)."
  else
    printf '%s\n' "$locks"
    for pid in $(printf '%s\n' "$locks" | awk '{print $1}' | sort -u); do
      echo "Lock holder command:"
      sudo -n ps -ww -p "$pid" -o pid,ppid,user,lstart,args
      ancestor=$pid
      depth=0
      # Ancestors may include the Actions runner with credentials in argv.
      # Print executable names and cgroups only for the ancestry trace.
      echo "Process ancestry and cgroup (systemd unit):"
      while [ "$ancestor" -gt 0 ] && [ "$depth" -lt 32 ]; do
        sudo -n ps -ww -p "$ancestor" -o pid,ppid,user,lstart,comm
        sudo -n cat "/proc/$ancestor/cgroup"
        parent=$(ps -p "$ancestor" -o ppid= | tr -d '[:space:]')
        case "$parent" in
        '' | *[!0-9]*) break ;;
        esac
        [ "$parent" != "$ancestor" ] || break
        ancestor=$parent
        depth=$((depth + 1))
      done
    done
  fi
  echo "::endgroup::"
}

# Do not let errexit inherited from a caller or a disappearing process replace
# the apt-get failure. Diagnostic commands are best effort and never retry APT.
diagnose_locks || true
exit "$apt_status"
