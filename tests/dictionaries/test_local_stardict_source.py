"""Source-level guard for local StarDict auto-registration and format compatibility.

Device and native dictionary_store.cpp fixtures verify runtime behavior.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
manager = (root / "src/dictionaries/manager.cpp").read_text(encoding="utf-8")
engine = (root / "src/dictionaries/stardict.cpp").read_text(encoding="utf-8")
fixture = (root / "tests/model/dictionary_store.cpp").read_text(encoding="utf-8")

assert "discoverLocalDictionaries(data->root);" in manager
assert "prepareLocalDictionary" in manager
assert "localDictionaryIds(data->root)" in manager
assert 'meta["name"].toString(id)' in manager
assert 'meta["language"].toString(id.section' in manager
assert 'values["version"] == "3.0.0"' in engine
assert 'values.value("idxoffsetbits", "32") == "32"' in engine
assert "hu-en-wiktionary" in fixture
assert "locally copied 3.0.0 StarDict must appear without catalog edits" in fixture
assert "local Hungarian source uses CPHUN224 morphology" in fixture
print("PASS: local StarDict source integration and 3.0.0 recognition")
