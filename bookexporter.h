#ifndef BOOKEXPORTER_H
#define BOOKEXPORTER_H

#include <QString>

class QProgressBar;

class BookExporterPrivate;
class BookExporter {
    Q_DECLARE_PRIVATE(BookExporter)
public:
    static bool generateCBZ(const QString& filename, const QString& tmpPath, QProgressBar* bar = 0);
    static bool generateEPUB(const QString& filename, const QString& tmpPath, QProgressBar* bar = 0);
    static bool generateMOBI(const QString& filename, const QString& tmpPath, QProgressBar* bar = 0);

private:
    BookExporter(const QString& filename, const QString tmpPath);
    ~BookExporter();
    BookExporterPrivate * const d_ptr;
};

#endif // BOOKEXPORTER_H
