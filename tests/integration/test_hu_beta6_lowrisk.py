"""First-stage beta6 feature availability checks.

This checks upstream integration points. Full acceptance requires device tests
after the Hungarian Edition rendering and lookup patches are ported.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[2]

def source(path):
    return (root / path).read_text(encoding="utf-8")

reading = source("src/app/reader/views/reading_page.cpp")
viewer = source("src/app/reader/views/image_viewer.cpp")
bookmarks = source("src/app/reader/window/reader_build.cpp")
sync = source("src/app/reader/settings/kosync_schedule_sheet.cpp")
sleep = source("src/app/reader/views/sleep_screen.cpp")
assert "rrPinnedBookmarks" in bookmarks, "Pinned bookmark tray missing"
assert "rrImageZoomOut" in viewer and "zoomBy(" in viewer, "Image zoom viewer missing"
assert "scheduleMinutes()" in sync and "rrKosyncInterval" in sync, "KOSync scheduling missing"
assert "void paintBookmark(" in sleep, "New sleep bookmark indicator missing"
assert "QColor(128, 128, 128)" in reading, "Hungarian Edition grayscale bookmark missing"
assert "render::paintBookmarkRibbon" in reading, "Reading-page bookmark renderer missing"
print("PASS: beta6 pinned bookmarks, image zoom, KOSync schedule, sleep marker, Hungarian gray ribbon")
