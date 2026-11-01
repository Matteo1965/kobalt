#!/usr/bin/env python3
"""HU hyphenation regression *source guard*, not an engine rendering test.

This checks curated compound entries and the priority contract in the CRengine
patch. It cannot prove that a rendered EPUB wraps correctly; that requires a
compiled CRengine integration test and device verification.
"""
from pathlib import Path
import re
import unittest

PATCH = Path(__file__).resolve().parents[2] / "dependencies/patches/crengine-hungarian-extended.patch"
CASES = {
    ("meg", "gyullad"): "meg-gyullad",
    ("meg", "gyaláz"): "meg-gyaláz",
    ("meg", "gyón"): "meg-gyón",
    ("arc", "csont"): "arc-csont",
    ("kis", "szék"): "kis-szék",
    ("szín", "nyomás"): "szín-nyomás",
    ("okos", "szemüveg"): "okos-szemüveg",
    ("tánc", "csoport"): "tánc-csoport",
    ("gyors", "szolgálat"): "gyors-szolgálat",
    ("hús", "szelet"): "hús-szelet",
    ("cipős", "szekrény"): "cipős-szekrény",
    ("ruhás", "szekrény"): "ruhás-szekrény",
}
RESTORATIONS = {
    "asszonnyal": "asz-szonnyal",
    "könnyek": "köny-nyek",
    "eddzük": "ed-dzük",
    "fröccsenés": "fröc-csenés",
    "rozzsal": "roz-zsal",
}

class HungarianSourceRegression(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.patch = PATCH.read_text(encoding="utf-8")

    def test_curated_compounds_are_present(self):
        section = self.patch.split("static const HuCompoundFamily huCompoundFamilies[] =", 1)[1].split("};", 1)[0]
        families = re.findall(r'\\{U"([^"]+)", U"([^"]+)"\\}', section)
        for (left, right), expected in CASES.items():
            word = left + right
            self.assertEqual(expected, left + "-" + right)
            self.assertTrue(any(word.startswith(a + b) and a == left for a, b in families),
                            f"Missing curated compound entry for {word}")

    def test_compound_priority_before_replacement_marking(self):
        section = self.patch.split("static bool huAddExtendedHyphenation(", 1)[1].split("return found;", 1)[0]
        self.assertLess(section.index("const int compound ="), section.index("for ( int i=0; i<len; i++ )"))
        self.assertIn("flags[compound - 1] &= ~LCHAR_HUNGARIAN_HYPH_REPLACE", section)
        self.assertIn("if ( compound && split <= compound ) continue;", section)

    def test_restoration_reference_data(self):
        for word, form in RESTORATIONS.items():
            self.assertIn("-", form)
            self.assertGreater(len(word), 4)

if __name__ == "__main__":
    unittest.main()
