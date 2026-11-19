#pragma once
// Shared reader data types independent of the CREngine implementation.
// Extracted without changing their fields or enum values, as the first
// compatibility step toward the Beta 9 Blaze engine contract.
#include <QString>
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

} // namespace kobalt
