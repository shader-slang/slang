# Recover historical NVVM evidence

Current behavior belongs in the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md), [STATUS](STATUS.md) and
[RESULTS](RESULTS.md). This file is an archive access guide, not an append-only slice index.

The last complete pre-consolidation tree is commit
`b20a4b4f680085b9394b029d1d95ac1f1b5cb4e0`. It contains the completed plans/reports, older validation
snapshots, focused experiments and historical result packages through slice296. Recover a file with:

```bash
git show b20a4b4f680085b9394b029d1d95ac1f1b5cb4e0:issue-nvvm-backend/report.slice-296-record-array-boundary.md
```

Use `git ls-tree -r --name-only <commit> -- issue-nvvm-backend` to find an old path, and
`git log --all -- <path>` to inspect later replacements. An exported/shallow checkout needs the
referenced Git objects to retrieve archival detail; current commands and accepted comparisons must
work without them. Keep fetched history separate from current documentation.

The current accepted baseline is a byte-preserving copy of the accepted293 record. Historical paths
inside that evidence retain their original spelling and hashes: removed repository artifacts resolve
against the commit above. They are provenance, not live input dependencies. Check the recorded SHA256
after retrieval. `build/` paths identify local raw evidence; those ignored artifacts were never Git
archives and may be unavailable on another machine. Recorded identities/outcomes remain durable.

Future replacements use their own Git revision for provenance. Do not grow this guide into another
chronological ledger, rewrite Git history, or manufacture fresh validation while migrating documents.
