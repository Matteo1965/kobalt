# Hungarian Edition Beta 9 integration

Source: nicoverbruggen/kobalt `v1.0-beta.9` (published 2026-11-18).
Starting point: Hungarian Edition Beta 8 Build 332, commit `4f0b00be83c15ec7c687208d4830b1310e79b200`.
Development branch: `hungarian-edition-beta9-integration`. Keep `main` unchanged until device validation.

## Approved scope and sequence

1. Hybrid chapter-first pagination and background whole-book layout.
2. Notes on highlights and quick annotation.
3. Editable sleep and power-off screen layouts.
4. Faster text-preset previews with visible hyphenation.
5. Kobo opening/closing reliability fixes.
6. Search and KOSync fixes **last**, with separate regressions.

Out of scope: Activity reading calendar and upstream Beta 9 native StarDict 2.4.2 installation/integration. **Preserve the existing Hungarian dictionary implementation.**

## First migration gate: hybrid engine

This is a source-layout migration, not a single setting change. Beta 8 keeps its engine adapter in `src/engine/crengine/` and builds an external patched CREngine through `dependencies/CMakeLists.txt` and `dependencies/crengine-patches.txt`. Upstream Beta 9 ships its Blaze source in `src/engine/blaze/`, exposes the engine contract in `src/engine/engine.h`, and builds the local engine using `dependencies/local-engine.cmake`.

Do not copy the upstream engine over the Beta 8 tree or delete its patch list until the following are explicitly preserved and tested:

- Hungarian hyphenation patterns and selection modes, including the extended pattern set and exception words.
- Hungarian-only optical hyphen hanging: `dependencies/patches/crengine-hungarian-optical-hyphen.patch` sets `getHyphenHangingPercent()` to 100 for Hungarian and 70 for other languages. Carry the behavior into the vendored Blaze source, not necessarily the patch file.
- Existing paragraph indentation/spacing, drop-cap and `<br>` handling, right-edge rendering and layout defaults.
- Existing dictionary selection, Hungarian inflected word lookup and UI translations.
- Saved EPUB positions, bookmarks, highlights, annotations, KOSync-compatible locations, and per-book typography.
- Kobo Qt5 firmware compatibility (Clara BW / Libra 2).

### Required implementation stages

- [ ] **Engine migration:** integrate upstream engine contract, Blaze adapter and local engine build wiring while keeping Hungarian changes. Verify upstream engine version/pins and licenses. No hybrid flag until this compiles.
- [ ] **Compatibility tests:** convert the Hungarian-only optical-hyphen patch and bring over engine-specific tests; verify baseline layout and book-position data against Build 332.
- [ ] **Hybrid behavior:** chapter-first opening, estimated page counts, background layout yielding to user input, cached full-book reuse, and cancel/restart on typography changes. Full-book/accurate fallback must remain available.
- [ ] **Qt5 CI:** validation, native tests and Kobo Qt5 build all succeed.
- [ ] **Device check:** test a cold EPUB opening, reopening, next/previous chapter, font/margin changes, sleep/return and closing on the actual Kobo. Observe blocking time and page-count changes.

Upstream design/reference: `doc/engines.md`, `doc/engines/migration.md`, `src/engine/blaze/adapter/reader_source.cpp` at tag `v1.0-beta.9`.

## Release rule

No public release, Beta 9 version label or merge to `main` until the integrated Qt5 build is tested on-device. The working Beta 8 Build 332 remains the rollback baseline.
