#!/usr/bin/env python3
"""Static integration contract for Hungarian CRengine hyphenation.

This verifies that the build patches wire the helper, line width, word flag,
and renderer together. It does not replace a full engine/layout test.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
HELPERS = (ROOT / "dependencies/patches/crengine-hungarian-extended.patch").read_text(encoding="utf-8")
WIDTH = (ROOT / "dependencies/patches/crengine-hungarian-width.patch").read_text(encoding="utf-8")
ORDER = (ROOT / "dependencies/crengine-patches.txt").read_text(encoding="utf-8").splitlines()

class HungarianPipelineContract(unittest.TestCase):
    def test_normal_hyphenation_precedes_hungarian_correction(self):
        start = HELPERS.index("const bool ordinaryHyphenation")
        end = HELPERS.index("if (ordinaryHyphenation || extendedHungarian)", start)
        block = HELPERS[start:end]
        self.assertLess(block.index("huMethod->hyphenate("), block.index("huAddExtendedHyphenation("))
        self.assertIn("m_flags+wstart", block)
        self.assertIn("isHungarian()", block)

    def test_compound_flags_and_renderer_use_same_marker(self):
        self.assertIn("flags[compound - 1] &= ~LCHAR_HUNGARIAN_HYPH_REPLACE", HELPERS)
        self.assertIn("word->hungarian_replace = 1", HELPERS)
        self.assertIn("if ( flgHyphen && word->hungarian_replace", HELPERS)
        self.assertIn("draw_len += replacement_len", HELPERS)

    def test_compound_preserves_second_member_syllable_boundaries(self):
        # rossz-in-du-la-tu: the legal break after "in" is
        # two characters after the first compound member.
        self.assertIn("split<=compound+1 && split<len", HELPERS)
        self.assertNotIn("split<=compound+2 && split<len", HELPERS)

    def test_persistent_kobo_seam_diagnostic(self):
        self.assertIn('/mnt/onboard/.adds/kobalt/hu-hyphen-diagnostic.log', HELPERS)
        self.assertIn('fopen(', HELPERS)
        self.assertIn('fprintf(huDiag,', HELPERS)
        self.assertIn('fclose(huDiag)', HELPERS)
        self.assertIn('HU-SEAM prefix=', HELPERS)

    def test_width_patch_checks_formatted_text_flags(self):
        self.assertIn("bool huReplacement = (fmt->m_flags[wstart+i] & LCHAR_HUNGARIAN_HYPH_REPLACE)", WIDTH)
        self.assertIn("if (huCompound == i + 1) huReplacement = false;", WIDTH)
        self.assertIn("huCandidateWidth += huReplacementWidth(", WIDTH)
        self.assertIn("huCandidateWidth > max_extent", WIDTH)
        self.assertIn("fmt->m_text+wstart", WIDTH)
        self.assertLess(ORDER.index("crengine-hungarian-extended.patch"),
                        ORDER.index("crengine-hungarian-width.patch"))

if __name__ == "__main__":
    unittest.main()
