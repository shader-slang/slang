#!/usr/bin/env python3
"""Download + cache prebuilt linux-x86_64 slangc for a range of Slang releases.

The corporate proxy blocks the github.com /releases/download/ path, but the
GitHub *API* asset endpoint (Accept: application/octet-stream) redirects to the
reachable release-assets CDN, so that is what we use.

The default tag set is the minor releases (vYYYY.N, no patch suffix) in a date
window, read from the local Slang git checkout's tags. Patch releases can be
added with --include-patches, or an explicit list passed with --tags.

Writes releases/<tag>/ with the extracted tree and a releases/index.json mapping
tag -> {slangc, version, date}.
"""
import argparse
import json
import os
import subprocess
import sys
import tarfile
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))  # slang checkout
API = "https://api.github.com/repos/shader-slang/slang"
# Preferred asset suffixes, best first. Host glibc (2.39) runs all of them; the
# plain build targets the newest glibc, the -glibc-N builds are for older hosts.
ASSET_SUFFIXES = ["-linux-x86_64.tar.gz", "-linux-x86_64-glibc-2.28.tar.gz",
                  "-linux-x86_64-glibc-2.27.tar.gz"]


def gh_get(url):
    req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json"})
    tok = os.environ.get("GITHUB_TOKEN")
    if tok:
        req.add_header("Authorization", f"Bearer {tok}")
    with urllib.request.urlopen(req, timeout=30) as r:
        return json.load(r)


def window_tags(repo, since, until, include_patches):
    """Ordered [(date, tag)] of release tags in [since, until] from local git."""
    out = subprocess.check_output(
        ["git", "-C", repo, "for-each-ref",
         "--sort=creatordate", "--format=%(creatordate:short) %(refname:short)",
         "refs/tags"],
        text=True,
    )
    import re
    minor = re.compile(r"^v\d{4}\.\d+$")
    patch = re.compile(r"^v\d{4}\.\d+\.\d+$")
    tags = []
    for line in out.splitlines():
        date, _, tag = line.partition(" ")
        if not (since <= date <= until):
            continue
        if minor.match(tag) or (include_patches and patch.match(tag)):
            tags.append((date, tag))
    return tags


def pick_asset(release):
    by_name = {a["name"]: a for a in release.get("assets", [])}
    ver = release["tag_name"].lstrip("v")
    for suf in ASSET_SUFFIXES:
        name = f"slang-{ver}{suf}"
        if name in by_name:
            return by_name[name]
    return None


def download_asset(asset, dest):
    """Download via the API asset id with octet-stream Accept (proxy-safe)."""
    url = f"{API}/releases/assets/{asset['id']}"
    req = urllib.request.Request(url, headers={"Accept": "application/octet-stream"})
    tok = os.environ.get("GITHUB_TOKEN")
    if tok:
        req.add_header("Authorization", f"Bearer {tok}")
    with urllib.request.urlopen(req, timeout=180) as r, open(dest, "wb") as fh:
        while True:
            chunk = r.read(1 << 20)
            if not chunk:
                break
            fh.write(chunk)


def find_slangc(root):
    for dirpath, _, files in os.walk(root):
        if "slangc" in files and os.path.basename(dirpath) == "bin":
            return os.path.join(dirpath, "slangc")
    return None


def fetch_one(tag, date, outdir, force):
    tagdir = os.path.join(outdir, tag)
    existing = find_slangc(tagdir) if os.path.isdir(tagdir) else None
    if existing and not force:
        return {"tag": tag, "date": date, "slangc": existing, "cached": True}

    release = gh_get(f"{API}/releases/tags/{tag}")
    asset = pick_asset(release)
    if not asset:
        return {"tag": tag, "date": date, "error": "no linux-x86_64 asset"}

    os.makedirs(tagdir, exist_ok=True)
    tarpath = os.path.join(tagdir, asset["name"])
    print(f"  downloading {asset['name']} ({asset['size'] // (1<<20)} MB) ...",
          flush=True)
    download_asset(asset, tarpath)
    with tarfile.open(tarpath, "r:gz") as t:
        t.extractall(tagdir)
    os.remove(tarpath)
    slangc = find_slangc(tagdir)
    if not slangc:
        return {"tag": tag, "date": date, "error": "slangc not found in archive"}
    os.chmod(slangc, 0o755)
    return {"tag": tag, "date": date, "slangc": slangc, "cached": False}


def verify(slangc):
    try:
        v = subprocess.run([slangc, "-v"], capture_output=True, text=True, timeout=30)
        return (v.stdout + v.stderr).strip().splitlines()[0] if (v.stdout or v.stderr) else "?"
    except Exception as e:  # noqa: BLE001
        return f"FAILED: {e}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", default=DEFAULT_REPO, help="local slang git checkout")
    ap.add_argument("--since", default="2025-08-01", help="YYYY-MM-DD inclusive")
    ap.add_argument("--until", default="2026-06-30", help="YYYY-MM-DD inclusive")
    ap.add_argument("--include-patches", action="store_true")
    ap.add_argument("--tags", default=None, help="comma-separated explicit tags (overrides window)")
    ap.add_argument("--out", default=os.path.join(HERE, "releases"))
    ap.add_argument("--force", action="store_true", help="re-download even if cached")
    args = ap.parse_args()

    if args.tags:
        # keep chronological order from git where possible
        all_in_window = window_tags(args.repo, "0000", "9999", True)
        order = {t: d for d, t in all_in_window}
        tags = [(order.get(t, "?"), t) for t in args.tags.split(",")]
    else:
        tags = window_tags(args.repo, args.since, args.until, args.include_patches)

    if not tags:
        sys.exit("no tags matched")
    os.makedirs(args.out, exist_ok=True)
    print(f"{len(tags)} releases to fetch -> {args.out}")

    index = []
    for date, tag in tags:
        print(f"[{tag}] {date}")
        rec = fetch_one(tag, date, args.out, args.force)
        if "slangc" in rec:
            rec["version"] = verify(rec["slangc"])
            state = "cached" if rec.get("cached") else "downloaded"
            print(f"  ok ({state}): {rec['version']}")
        else:
            print(f"  ERROR: {rec['error']}")
        index.append(rec)

    ipath = os.path.join(args.out, "index.json")
    with open(ipath, "w") as fh:
        json.dump(index, fh, indent=2)
    ok = sum(1 for r in index if "slangc" in r)
    print(f"\n{ok}/{len(index)} releases ready. wrote {ipath}")
    if ok != len(index):
        sys.exit(1)


if __name__ == "__main__":
    main()
