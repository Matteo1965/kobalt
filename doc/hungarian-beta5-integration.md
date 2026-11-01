# Hungarian Edition beta 5 integration

Base: upstream `nicoverbruggen/kobalt` tag `v1.0-beta.5` at `3d8514c1b08eef5ed7c8c263e88e4254d71cd03c`.

Previous Hungarian references (do not overwrite):
- `hungarian-edition-beta4` at `95a173e0f7f1fb931585adad3817b9c5f6ace015`
- `hungarian-edition-dictionary-language-test` at `d1a7a6307d8680acb9feb1ff3625bb4ac43df76e`
- Last successful Qt5 dictionary test: GitHub Actions run `41220914525`.

## Porting checklist

- [ ] Adapt Hungarian translations (`assets/hu.strings`) to the beta 5 English string keys.
- [ ] Reconcile Hungarian paragraph spacing / indentation behavior with the upstream beta 5 implementation.
- [ ] Port Extended Hungarian Hyphenation and verify with Hungarian word fixtures.
- [ ] Rebase CPHUN-224 StarDict lookup/morphology logic onto beta 5 `src/dictionaries/stardict.cpp`; keep the new upstream lookups.
- [ ] Preserve the HU source-language gating and catalog/metadata behavior in the new `src/dictionaries/manager.cpp`.
- [ ] Verify custom dictionary import: beta 4 HU-specific ZIP path must not be mistaken for a generic custom StarDict importer.
- [ ] Port Hungarian UI and previously validated typography and initial-cap changes with explicit tests.
- [ ] Validate Kobo Qt5 build and then test Clara BW / Libra 2.

No Hungarian port is included merely by creating this branch. The base remains untouched upstream beta 5 code until each item is implemented and tested.
