#pragma once
#include <QByteArray>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QVector>

namespace kobalt {
// Repair conservative, publisher-separated drop caps while the original XHTML
// structure still exists. CREngine may wrap or split <p> nodes during parsing,
// making a post-parse repair unreliable on real Calibre EPUB chapters.
// Do not alter books with non-UTF8 content, links, nested following paragraphs,
// IDs on the affected paragraphs, or nontrivial inline markup.
inline QByteArray withShortDropcapContinuations(const QByteArray &bytes)
{
    if (!bytes.contains("inic") && !bytes.contains("initial")
        && !bytes.contains("dropcap") && !bytes.contains("drop-cap"))
        return bytes;
    QString source = QString::fromUtf8(bytes);
    if (source.toUtf8() != bytes)
        return bytes;
    static const QRegularExpression pair(
            QStringLiteral(R"RX((<p\b[^>]*>\s*<span\b[^>]*\bclass\s*=\s*["'][^"']*(?:inic|initial|drop-?cap)[^"']*["'][^>]*>[^<]{1,3}</span>[^<]{1,64})</p>(\s*)<p\b[^>]*>([^<]{8,2000})</p>)RX"),
            QRegularExpression::CaseInsensitiveOption);
    struct Fix {
        int begin;
        int length;
        QString replacement;
    };
    QVector<Fix> fixes;
    auto match = pair.globalMatch(source);
    while (match.hasNext() && fixes.size() < 128) {
        const auto found = match.next();
        const auto first = found.captured(1);
        const auto second = found.captured(3);
        // Do not coalesce titled/anchored blocks, and require a genuinely
        // short introductory paragraph rather than ordinary running text.
        if (first.contains(QRegularExpression(QStringLiteral(R"RX(\bid\s*=)RX"),
                                               QRegularExpression::CaseInsensitiveOption))
            || first.size() > 180 || second.trimmed().size() < 8)
            continue;
        fixes.append({found.capturedStart(), found.capturedLength(),
                      first + QStringLiteral("<br/>") + second + QStringLiteral("</p>")});
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
