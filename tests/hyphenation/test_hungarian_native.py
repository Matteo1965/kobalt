#!/usr/bin/env python3
"""Compile and execute the actual Hungarian helper functions from the CRengine patch.

Checks the flags/restore decisions, not EPUB rendering or CRengine line breaking.
"""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
PATCH = ROOT / "dependencies/patches/crengine-hungarian-extended.patch"
BEGIN = "+static lChar32 huAsciiLower("
END = "+// Extra width of the *rendered prefix*"

CASES = [
    ("meggyullad", 3), ("meggyaláz", 3), ("meggyón", 3),
    ("arccsont", 3), ("kisszék", 3), ("színnyomás", 4),
    ("okosszemüveg", 4), ("tánccsoport", 4),
]
DOUBLED = ["asszonnyal", "könnyek", "eddzük", "fröccsenés", "rozzsal"]

class NativeHungarianHyphenation(unittest.TestCase):
    def test_compiled_real_helper_flags(self):
        patch = PATCH.read_text(encoding="utf-8")
        start = patch.index(BEGIN)
        end = patch.index(END, start)
        extracted = "\n".join(line[1:] for line in patch[start:end].splitlines()
                              if line.startswith("+"))
        prefix = """#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
using lChar32 = char32_t;
using lUInt16 = uint16_t;
constexpr lUInt16 LCHAR_ALLOW_HYPH_WRAP_AFTER = 0x0008;
constexpr lUInt16 LCHAR_HUNGARIAN_HYPH_REPLACE = 0x0080;
"""
        # Read the actual bit from lvfnt.h rather than assuming it.
        # LCHAR_ALLOW_HYPH_WRAP_AFTER is fixed by upstream at 0x0008.
        suffix = """
int main() {
    struct Compound { const char32_t *word; int split; };
    const Compound compounds[] = {
        {U"meggyullad",3},{U"meggyaláz",3},{U"meggyón",3},
        {U"arccsont",3},{U"kisszék",3},{U"színnyomás",4},
        {U"okosszemüveg",4},{U"tánccsoport",4},
    };
    int failed=0;
    for (const auto &c: compounds) {
        int len=std::char_traits<char32_t>::length(c.word);
        std::vector<lUInt16> flags(len,0);
        // Simulate a neighboring incorrect base-engine candidate.
        if(c.split<len) flags[c.split] = LCHAR_ALLOW_HYPH_WRAP_AFTER;
        huAddExtendedHyphenation(c.word,len,flags.data(),2,2);
        bool good=(flags[c.split-1] & LCHAR_ALLOW_HYPH_WRAP_AFTER)
             && !(flags[c.split-1] & LCHAR_HUNGARIAN_HYPH_REPLACE)
             && !(flags[c.split] & LCHAR_ALLOW_HYPH_WRAP_AFTER);
        if(!good) { std::cerr << "compound flags error at split "<<c.split<<"\\n"; ++failed; }
    }
    for(const auto &word: {U"asszonnyal",U"könnyek",U"eddzük",U"fröccsenés",U"rozzsal"}) {
        int len=std::char_traits<char32_t>::length(word);
        std::vector<lUInt16> flags(len,0);
        huAddExtendedHyphenation(word,len,flags.data(),2,2);
        bool found=false;
        for(int i=0;i<len;++i) {
            if((flags[i] & (LCHAR_ALLOW_HYPH_WRAP_AFTER|LCHAR_HUNGARIAN_HYPH_REPLACE))
               == (LCHAR_ALLOW_HYPH_WRAP_AFTER|LCHAR_HUNGARIAN_HYPH_REPLACE))
                found=true;
        }
        if(!found) { std::cerr<<"no restored double-consonant break\\n"; ++failed; }
    }
    return failed ? 1 : 0;
}
"""
        with tempfile.TemporaryDirectory() as tmp:
            source = pathlib.Path(tmp) / "native.cpp"
            binary = pathlib.Path(tmp) / "native"
            source.write_text(prefix + extracted + suffix, encoding="utf-8")
            build = subprocess.run(["g++", "-std=c++17", "-Wall", "-Wextra",
                                    str(source), "-o", str(binary)],
                                   capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            test = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(test.returncode, 0, test.stderr)

if __name__ == "__main__":
    unittest.main()
