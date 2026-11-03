# Hungarian Edition CRengine integrations (beta 5)

The Kobo builds use a **pinned** CRengine source archive from `dependencies/manifest.json`, not a vendored copy of CRengine. The patches listed in `dependencies/crengine-patches.txt` are **production source changes** applied automatically during dependency compilation by `dependencies/apply-patches.cmake`. They are committed to this repository, not stored in GitHub Actions artifacts.

## Permanent Hungarian changes

- `crengine-hungarian-extended.patch` — extended Hungarian syllabification, compound boundaries, doubled consonant restoration, exact replacement drawing, and width handling.
- `crengine-hungarian-width.patch` — line-fit width validation for restored Hungarian consonants.
- `crengine-hungarian-optical-hyphen.patch` — **100%** of the hyphen advance may hang when optical margin is enabled **in Hungarian text**; other languages keep upstream **70%**.
- `src/engine/crengine/reader_model.cpp` and `reader_source.cpp` — persist and apply the optical-margin toggle for Hungarian books, with separate layout-cache identities and a fresh document load on toggling; other book languages keep normal CRengine behavior.
- `src/engine/crengine/reader_alignment.cpp`, `reader_beautifier.cpp`, `reader_source.cpp`, `reader_model.cpp` — retain the beta 4 initial/drop-cap rendering repairs in beta 5.

Persistent writing of `hu-hyphen-diagnostic.log` has been removed. Do not re-enable it in production.

## Update contract

**Do not remove these patches or the lines referring to them from `dependencies/crengine-patches.txt` when updating CRengine.** First port their changes to the newer upstream source, and update the patch hunks if needed. Keep the same Hungarian optical-margin behavior and regression tests.

Required before shipping:
1. `bash platforms/ci/check-crengine-patches.sh` — downloads the pinned source, verifies its SHA256, applies *all* patches in order and checks the resulting code.
2. `python3 -m unittest discover -s tests/hyphenation -p 'test_*.py' -v`
3. `bash platforms/ci/test.sh`, followed by a successful Qt5 package build.
4. Device test with optical margin ON and OFF on Hungarian EPUB, plus normal hyphenation and drop-cap regression cases.

This project deliberately keeps a patch series instead of checking in the entire external CRengine source. A future upstream update must explicitly rebase and revalidate the series; otherwise the build fails instead of silently dropping the fixes.
