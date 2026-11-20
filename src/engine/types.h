#pragma once
// Shared reader data types independent of the CREngine implementation.
// Extracted without changing their fields or enum values, as the first
// compatibility step toward the Beta 9 Blaze engine contract.
#include <QString>
#include <QPoint>
#include <QRect>
#include <QVector>
#include <functional>

namespace kobalt {
enum class OpeningStage { Content, Text, Styles, Pages, Ready };
using OpeningProgress = std::function<void(OpeningStage, int)>;

struct ChapterProgress
{
    QString title;
    int page = 0, pages = 0;
};

struct SearchHit
{
    QString start, end, before, text, after;
    int page = -1;
};

struct SearchBatch
{
    QVector<SearchHit> hits;
    int nextPage = 0;
    bool done = false, limited = false;
    // A restarted scan replaces results from the previous layout.
    bool reset = false;
};

struct TextRange
{
    QString start, end, text;
    QString chosenEnd = {};
    QString chosenStart = {};
    bool sentenceStartPolicy = false;
    bool sentenceEndPolicy = false;
};

struct WordSelection
{
    QString text, start, end;
    QVector<QRect> rectangles;
};

struct SelectionEndpoint
{
    // True terminal edge span in viewport pixels, never a clipped range edge.
    QPoint first, last;
    // Axis unit normal pointing away from selected text. Caps point inward.
    QPoint outward;
    bool visible = false; // False for endpoints outside the current page/viewport.
};

struct AnnotationGeometry
{
    QVector<QRect> rectangles;
    SelectionEndpoint start, end;
    bool vertical = false; // Current page's writing axis, independent of range shape.
};

struct AnnotationToken : WordSelection
{
    // Source boundaries for automatic paired punctuation. No whitespace is included.
    QString startBeforePunctuation, endAfterPunctuation;
    bool punctuation = false;
};

struct AnnotationPageEdges
{
    AnnotationToken first, last;
    SelectionEndpoint start, end;
    int columns = 1;
    bool vertical = false;
    bool continuesBefore = false, continuesAfter = false;
};

} // namespace kobalt
