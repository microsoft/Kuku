# BLAKE2 reference implementation

This directory contains the upstream-vendored BLAKE2 C reference implementation used by Kuku's location functions.

- **Upstream:** https://github.com/BLAKE2/BLAKE2
- **Pinned commit:** see `cgmanifest.json` in this directory.
- **License:** the upstream files carry their own copyright headers offering the code under CC0 1.0, the OpenSSL Licence, or Apache 2.0, at your option; do not re-license.
  Kuku exercises the CC0 1.0 option, as recorded in the repository root `NOTICE`.

`blake2.h` and `blake2-impl.h` are vendored verbatim from upstream at the pinned commit.
`blake2b.c` differs from upstream only in its two `#include` path rewrites to Kuku's install layout (`kuku/internal/...`).
`blake2xb.c` has the same include-path rewrites plus four range-preserving integer casts, grouped under three `EDIT: ...` annotations, which silence misleading MSVC narrowing warnings without changing any hashed value.
These files are excluded from project-wide clang-tidy via the per-directory `.clang-tidy` in `third_party/`, and from `scripts/clang-format-all.sh`, so that they stay byte-identical to upstream.
