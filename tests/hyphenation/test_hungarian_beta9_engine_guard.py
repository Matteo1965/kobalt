"""Guard the Hungarian renderer during the Beta 9 engine migration.

These are source-level checks, not a claim that the hybrid engine already runs.
They fail if the old CREngine patches disappear before equivalent Blaze behavior
has been implemented. Extend the checks to inspect Blaze when it is imported.
"""
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PATCHES = ROOT / "dependencies" / "patches"
PATCH_LIST = ROOT / "dependencies" / "crengine-patches.txt"


class HungarianBeta9EngineGuard(unittest.TestCase):
    def test_hungarian_extended_hyphenation_preserved(self):
        patch = PATCHES / "crengine-hungarian-extended.patch"
        self.assertTrue(patch.is_file())
        self.assertIn("crengine-hungarian-extended.patch", PATCH_LIST.read_text())
        self.assertIn("LCHAR_HUNGARIAN_HYPH_REPLACE", patch.read_text())

    def test_hungarian_replacement_width_preserved(self):
        patch = PATCHES / "crengine-hungarian-width.patch"
        self.assertTrue(patch.is_file())
        self.assertIn("crengine-hungarian-width.patch", PATCH_LIST.read_text())
        self.assertIn("huReplacementWidth", patch.read_text())

    def test_hungarian_only_optical_hanging_preserved(self):
        patch = PATCHES / "crengine-hungarian-optical-hyphen.patch"
        self.assertTrue(patch.is_file())
        self.assertIn("crengine-hungarian-optical-hyphen.patch", PATCH_LIST.read_text())
        source = patch.read_text()
        self.assertIn("isHungarian()", source)
        self.assertIn("return 100;", source)
        self.assertIn("return 70;", source)

    def test_dropcap_paragraph_repair_preserved(self):
        source = (ROOT / "src/engine/crengine/reader_source.cpp").read_text()
        self.assertIn("int repairShortDropcapParagraphs(ldomDocument *doc)", source)
        self.assertIn("repairShortDropcapParagraphs(document);", source)

    def test_hybrid_cooperative_preemption_api(self):
        worker = (ROOT / "src/engine/crengine/engine_worker.h").read_text()
        self.assertIn("class EngineIdleCheckpoint", worker)
        self.assertIn("engineWorkRequests() != requestGeneration_", worker)
        self.assertIn("engineWorkQuietMilliseconds() >= minimumQuiet_", worker)


if __name__ == "__main__":
    unittest.main()
