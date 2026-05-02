# BLAKE2 reference implementation

This directory contains the upstream-vendored BLAKE2 C reference implementation
used by Kuku's location functions.

- **Upstream:** https://github.com/BLAKE2/BLAKE2
- **Pinned commit:** see `cgmanifest.json` in this directory.
- **License:** the upstream files carry their own dual-licensed (CC0 / OpenSSL /
  Apache 2.0) copyright headers; do not re-license.

These files are vendored verbatim apart from minor edits marked `EDIT: ...` in
the source. They are excluded from project-wide clang-tidy via the per-directory
`.clang-tidy` in `third_party/`.
