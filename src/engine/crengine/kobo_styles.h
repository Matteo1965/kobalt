#pragma once
#include <QByteArray>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QVector>

namespace kobalt {
// Join a publisher's short drop-cap lead-in with the next paragraph before
// CREngine parses it. KEPUB adds nested koboSpan elements and IDs inside spans;
// preserve those intact so location anchors continue to work.
inline QByteArray withShortDropcapContinuations(const QByteArray &bytes)
{
    if (!bytes.contains("inic") && !bytes.contains("initial")
        && !bytes.contains("dropcap") && !bytes.contains("drop-cap"))
        return bytes;
    QString source = QString::fromUtf8(bytes);
    if (source.toUtf8() != bytes)
        return bytes;

    static const QRegularExpression paragraphOpen(
            QStringLiteral(R"RX(<p\b)RX"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression pair(
            QStringLiteral(R"RX((<p\b[^>]*>)([\s\S]{1,1200}?)(</p>)(\s*)(<p\b[^>]*>)([\s\S]{8,3000}?)(</p>))RX"),
            QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression initial(
            QStringLiteral(R"RX(^\s*<span\b[^>]*\bclass\s*=\s*["'][^"']*\b(?:inic|initial|initiale|dropcap|drop-cap)\b[^"']*["'])RX"),
            QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression tag(QStringLiteral(R"RX(<[^>]*>)RX"));
    static const QRegularExpression forbiddenMarkup(
            QStringLiteral(R"RX(<\s*/?\s*(?!span\b)[a-z][^>]*>)RX"),
            QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression paragraphId(
            QStringLiteral(R"RX(\bid\s*=)RX"), QRegularExpression::CaseInsensitiveOption);
    struct Fix {
        int begin;
        int length;
        QString replacement;
    };
    QVector<Fix> fixes;
    int offset = 0;
    while (offset < source.size() && fixes.size() < 128) {
        const auto opening = paragraphOpen.match(source, offset);
        if (!opening.hasMatch())
            break;
        const int start = int(opening.capturedStart());
        offset = start + 2; // Check every paragraph, not just disjoint pairs.
        const auto found = pair.match(source, start);
        // Require a match at the exact current paragraph without relying on
        // the deprecated AnchoredMatchOption (Qt 6.11 treats it as an error).
        if (!found.hasMatch() || found.capturedStart() != start)
            continue;
        const auto firstOpen = found.captured(1), firstBody = found.captured(2);
        const auto secondOpen = found.captured(5), secondBody = found.captured(6);
        if (paragraphId.match(firstOpen).hasMatch()
            || paragraphId.match(secondOpen).hasMatch()
            || !initial.match(firstBody).hasMatch()
            || forbiddenMarkup.match(firstBody).hasMatch()
            || forbiddenMarkup.match(secondBody).hasMatch())
            continue;
        QString firstPlain = firstBody;
        firstPlain.remove(tag);
        QString secondPlain = secondBody;
        secondPlain.remove(tag);
        if (firstPlain.trimmed().size() < 8 || firstPlain.trimmed().size() > 64
            || secondPlain.trimmed().size() < 8)
            continue;
        fixes.append({start, int(found.capturedLength()),
                      firstOpen + firstBody + QStringLiteral("<br/>")
                          + secondBody + QStringLiteral("</p>")});
        offset = start + int(found.capturedLength());
    }
    for (int i = fixes.size() - 1; i >= 0; --i)
        source.replace(fixes[i].begin, fixes[i].length, fixes[i].replacement);
    return fixes.isEmpty() ? bytes : source.toUtf8();
}

// Remove only the complete known injection. A reused marker can contain custom CSS.
inline QByteArray withoutKoboStyleHacks(const QByteArray &bytes)
{
    if (!bytes.contains("kobostylehacks")) {
        return bytes;
    }
    const auto text = QString::fromUtf8(bytes);
    // Kobo's injection is UTF-8. Preserve other encodings and invalid input verbatim.
    if (text.toUtf8() != bytes) {
        return bytes;
    }
    static const QRegularExpression original(QStringLiteral(
            "^\\s*div#book-inner\\s+p\\s*,\\s*div#book-inner\\s+div\\s*\\{\\s*font-size\\s*:\\s*1\\.0em\\s*;\\s*\\}"
            "\\s*a\\s*\\{\\s*color\\s*:\\s*black\\s*;\\s*\\}"
            "\\s*a:link\\s*,\\s*a:visited\\s*,\\s*a:hover\\s*,\\s*a:active\\s*\\{\\s*color\\s*:\\s*blue\\s*;\\s*\\}"
            "\\s*div#book-inner\\s+\\*\\s*\\{\\s*margin-top\\s*:\\s*0\\s*!important\\s*;"
            "\\s*margin-bottom\\s*:\\s*0\\s*!important\\s*;\\s*\\}\\s*$"));
    QXmlStreamReader xml(text);
    QList<QPair<int, int>> removals;
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.name() != QStringLiteral("style")
            || !(xml.namespaceUri().isEmpty()
                 || xml.namespaceUri() == QStringLiteral("http://www.w3.org/1999/xhtml"))) {
            continue;
        }
        const auto attributes = xml.attributes();
        const bool marked = attributes.value("id") == QStringLiteral("kobostylehacks")
                || attributes.value("class")
                           .toString()
                           .split(QRegularExpression("\\s+"))
                           .contains(QStringLiteral("kobostylehacks"));
        if (!marked) {
            continue;
        }
        const int start = int(xml.characterOffset());
        const auto css = xml.readElementText();
        const int end = int(text.lastIndexOf('<', int(xml.characterOffset()) - 1));
        if (!xml.hasError() && end >= start && original.match(css).hasMatch()) {
            removals.append({ start, end - start });
        }
    }
    if (xml.hasError() || removals.isEmpty()) {
        return bytes;
    }
    auto cleaned = text;
    for (int i = removals.size() - 1; i >= 0; --i) {
        cleaned.remove(removals[i].first, removals[i].second);
    }
    return cleaned.toUtf8();
}
} // namespace kobalt
