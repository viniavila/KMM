#ifndef PROJARCHIVE_H
#define PROJARCHIVE_H

#include <QString>

class QProgressBar;

class ProjArchivePrivate;
class ProjArchive {
    Q_DECLARE_PRIVATE(ProjArchive)
public:
    enum IOMode {
        InputMode,
        OutputMode
    };

    ProjArchive(const QString& filePath, IOMode io = InputMode);
    ~ProjArchive();
    void save(const QString& tmpPath);
    void extract(const QString& tmpPath) const;
    bool isValid() const;
    IOMode ioMode() const;
    void setProgressBar(QProgressBar* bar);

    static void saveToFile(const QString& filePath, const QString& tmpPath);
    static void loadFromFile(const QString& filePath, const QString& tmpPath);

private:
    ProjArchivePrivate * const d_ptr;
};

#endif // PROJARCHIVE_H
