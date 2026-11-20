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

    def test_shared_link_and_word_count_types(self):
        shared_links = (ROOT / "src/engine/links.h").read_text()
        shared_counts = (ROOT / "src/engine/word_counts.h").read_text()
        document = (ROOT / "src/engine/crengine/engine_document.h").read_text()
        for name in ("enum class LinkKind", "struct PageLink",
                     "struct LinkHit", "struct LinkDetails"):
            self.assertIn(name, shared_links)
        for name in ("struct WordCountChapter", "struct DocumentWordCounts"):
            self.assertIn(name, shared_counts)
        self.assertIn('#include "engine/links.h"', document)
        self.assertIn('#include "engine/word_counts.h"', document)
        for old, new in (("reader_links.h", "links.h"),
                         ("reader_word_counts.h", "word_counts.h")):
            shim = (ROOT / "src/engine/crengine" / old).read_text()
            self.assertIn('#include "engine/' + new + '"', shim)

    def test_engine_data_types_independent_of_crengine(self):
        types = (ROOT / "src/engine/types.h").read_text()
        document = (ROOT / "src/engine/crengine/engine_document.h").read_text()
        self.assertIn('#include "engine/types.h"', document)
        for declaration in ("enum class OpeningStage", "struct ChapterProgress",
                            "struct SearchHit", "struct SearchBatch", "struct TextRange",
                            "struct WordSelection", "struct SelectionEndpoint",
                            "struct AnnotationGeometry", "struct AnnotationToken",
                            "struct AnnotationPageEdges"):
            self.assertIn(declaration, types)
            self.assertNotIn(declaration, document)

    def test_idle_engine_steps_yield_to_user_requests(self):
        worker_header = (ROOT / "src/engine/crengine/engine_worker.h").read_text()
        worker_code = (ROOT / "src/engine/crengine/engine_worker.cpp").read_text()
        self.assertIn("void scheduleEngineIdleSteps(", worker_header)
        self.assertIn("QTimer::singleShot(delay, engineWorkerForIdle()", worker_code)
        self.assertIn("engineWorkRequests() != task->requestGeneration", worker_code)
        self.assertIn("queueIdleAttempt(task)", worker_code)
        self.assertIn("const qint64 quiet = engineWorkQuietMilliseconds()", worker_code)

    def test_hybrid_cooperative_preemption_api(self):
        worker = (ROOT / "src/engine/crengine/engine_worker.h").read_text()
        self.assertIn("class EngineIdleCheckpoint", worker)
        self.assertIn("engineWorkRequests() != requestGeneration_", worker)
        self.assertIn("engineWorkQuietMilliseconds() >= minimumQuiet_", worker)


if __name__ == "__main__":
    unittest.main()
