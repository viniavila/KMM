#ifndef PROJARCHIVE_H
#define PROJARCHIVE_H

#include <QString>
#include <zip.h>

class ProjArchive {
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

    static void saveToFile(const QString& filePath, const QString& tmpPath);
    static void loadFromFile(const QString& filePath, const QString& tmpPath);

private:
    const char* m_path;
    IOMode m_io;
    int errCode;
    bool m_isValid;
    zip_t* za;
};

#endif // PROJARCHIVE_H
