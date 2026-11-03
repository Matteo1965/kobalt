// Standalone native StarDict test: synthetic HU->DE/HU->ES and reverse directions.
// Uses no external dictionary downloads, only the real production StarDict engine.
#include "dictionaries/stardict.h"
#include "support/main_thread.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QtEndian>
#include <cstdlib>
#include <cstdio>

namespace kobalt::mainthread {
// Standalone headless fixture has no interactive reader screen. The production
// strict main-thread monitor is tested separately.
BlockingOperation::BlockingOperation(const char *name, bool enabled)
    : operation(name), reason(nullptr), observe(false) { Q_UNUSED(enabled); }
BlockingOperation::~BlockingOperation() {}
}

static void require(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}
static void save(const QString &path, const QByteArray &bytes) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "fixture write");
}
static void appendBE32(QByteArray &out, quint32 v) {
    char bytes[4];
    qToBigEndian<quint32>(v, reinterpret_cast<uchar *>(bytes));
    out.append(bytes, 4);
}
static void fixture(const QString &root, const QString &id) {
    const QString dir = QDir(root).filePath(id);
    require(QDir().mkpath(dir), "fixture directory");
    const QByteArray word = QStringLiteral("kabát").toUtf8();
    const QByteArray html = "<p>test dictionary definition</p>";
    QByteArray index = word + QByteArray(1, '\0');
    appendBE32(index, 0);
    appendBE32(index, quint32(html.size()));
    const QString prefix = QDir(dir).filePath(id);
    save(prefix + ".idx", index);
    save(prefix + ".dict", html);
    save(prefix + ".ifo", "StarDict's dict ifo file\nversion=2.4.2\nwordcount=1\nidxfilesize=" +
                          QByteArray::number(index.size()) + "\nsametypesequence=h\n");
    QString error;
    require(kobalt::StarDict::prepare(dir, id, &error), qPrintable(error));
}
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir dir;
    require(dir.isValid(), "temporary directory");
    for (const char *target : {"de", "es"}) {
        const QString id = QStringLiteral("hu-") + QString::fromLatin1(target);
        fixture(dir.path(), id);
        kobalt::StarDict dictionary;
        QString error;
        require(dictionary.open(QDir(dir.path()).filePath(id), id, &error), qPrintable(error));
        require(dictionary.lookup(QStringLiteral("kabát")).html == "<p>test dictionary definition</p>",
                "exact dictionary lookup must work");
        require(dictionary.lookup(QStringLiteral("kabátja")).html.isEmpty(),
                "Hungarian possessive fallback must be absent before source enablement");
        dictionary.setHungarianSource(true);
        require(dictionary.lookup(QStringLiteral("kabátja")).html == "<p>test dictionary definition</p>",
                "Hungarian-source possessive lookup must resolve via lemma");
        dictionary.setHungarianSource(false);
        require(dictionary.lookup(QStringLiteral("kabátja")).html.isEmpty(),
                "disabling Hungarian source must remove morphology fallback");
    }
    for (const char *source : {"de", "es"}) {
        const QString id = QString::fromLatin1(source) + QStringLiteral("-hu");
        fixture(dir.path(), id);
        kobalt::StarDict dictionary;
        QString error;
        require(dictionary.open(QDir(dir.path()).filePath(id), id, &error), qPrintable(error));
        require(dictionary.lookup(QStringLiteral("kabát")).html == "<p>test dictionary definition</p>",
                "non-Hungarian-source exact lookup");
        require(dictionary.lookup(QStringLiteral("kabátja")).html.isEmpty(),
                "Hungarian target language must not enable morphology");
    }
    std::puts("PASS: synthetic HU->DE, HU->ES, DE->HU, ES->HU StarDict source-language gating");
}
