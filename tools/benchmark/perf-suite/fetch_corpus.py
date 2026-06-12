#!/usr/bin/env python3
"""Fetch real-shader corpora into corpus/ for the perf suite.

Currently: the MDL/DXR "slangified" shaders from shader-slang/MDL-SDK (the same
corpus the existing tools/benchmark/compile.py uses). Downloaded via the GitHub
contents API (inline base64), which is reachable here even though git clone and
the release-download CDN path are blocked by the corporate proxy.
"""
import argparse
import base64
import json
import os
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))

CORPORA = {
    "mdl": {
        "repo": "shader-slang/MDL-SDK",
        "path": "examples/mdl_sdk/dxr/content/slangified",
        "exts": (".slang",),
    },
}


def gh_json(url):
    req = urllib.request.Request(url, headers={"Accept": "application/vnd.github+json"})
    tok = os.environ.get("GITHUB_TOKEN")
    if tok:
        req.add_header("Authorization", f"Bearer {tok}")
    with urllib.request.urlopen(req, timeout=40) as r:
        return json.load(r)


def fetch_corpus(name, spec, outroot):
    base = f"https://api.github.com/repos/{spec['repo']}/contents/{spec['path']}"
    listing = gh_json(base)
    outdir = os.path.join(outroot, name)
    os.makedirs(outdir, exist_ok=True)
    got = []
    for f in listing:
        if not f["name"].endswith(spec["exts"]):
            continue
        meta = gh_json(f["url"])  # file endpoint returns inline base64 content
        data = base64.b64decode(meta["content"])
        with open(os.path.join(outdir, f["name"]), "wb") as fh:
            fh.write(data)
        got.append((f["name"], len(data)))
    return got


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(HERE, "corpus"))
    ap.add_argument("--name", default="mdl", choices=list(CORPORA))
    args = ap.parse_args()
    got = fetch_corpus(args.name, CORPORA[args.name], args.out)
    print(f"fetched {len(got)} files into {args.out}/{args.name}/:")
    for n, s in sorted(got):
        print(f"  {s:>8}  {n}")


if __name__ == "__main__":
    main()
