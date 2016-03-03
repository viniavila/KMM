#include "projarchive.h"
#include <QDebug>
#include <QDir>

ProjArchive::ProjArchive(const QString& path, IOMode io) :
    m_path(path.toUtf8().data()),
    m_io(io),
    errCode(0),
    m_isValid(false)
{
    if (m_io == InputMode) za = zip_open(m_path, ZIP_CHECKCONS, &errCode);
    else za = zip_open(m_path, ZIP_CREATE|ZIP_EXCL, &errCode);

    if (errCode) {
        // TODO: throw error
    }
    else {
        m_isValid = true;
        if (m_io == InputMode) {
            // Check if "content.ini" and folder "chapters/" exists
            int iCi = zip_name_locate(za, "content.ini", 0);
            int iCh = zip_name_locate(za, "chapters/", 0);
            if (iCi == -1 || iCh == -1) {
                m_isValid = false;
                zip_close(za);
                za = NULL;
            }
        }
    }
}

ProjArchive::~ProjArchive() {
    if (za != NULL) zip_close(za);
}

void ProjArchive::save(const QString &tmpPath) {
    // add files from tmpPath to archive
    QDir dir(tmpPath);
    QStringList files = dir.entryList(QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name|QDir::LocaleAware);
    for (const QString& f : files) {
        const char* path = QString(tmpPath + QDir::separator() + f).toUtf8().data();
        zip_source_t * zs = zip_source_file(za, path, 0, -1);
        if (zs == NULL || zip_file_add(za, f.toUtf8().data(), zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
            // Error adding the file
            zip_source_free(zs);
        }
    }
    QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Readable|QDir::Executable, QDir::Name|QDir::LocaleAware);
    if (dirs.isEmpty()) dirs << "chapters";
    for (const QString& d : dirs) {
        zip_dir_add(za, QString(d+"/").toUtf8().data(), ZIP_FL_ENC_UTF_8);
        // TODO enter dir and add dirs and files recursively
    }
}

void ProjArchive::extract(const QString &tmpPath) const {
    qDebug() << zip_get_num_files(za);
    for (int i=0; i<zip_get_num_files(za); ++i)
        qDebug() << i << zip_get_name(za, i, 0);



    // extract files from archive to tmpPath
}

bool ProjArchive::isValid() const {
    return m_isValid;
}

ProjArchive::IOMode ProjArchive::ioMode() const {
    return m_io;
}

void ProjArchive::saveToFile(const QString& filePath, const QString& tmpPath) {
    ProjArchive ark(filePath, OutputMode);
    ark.save(tmpPath);
}

void ProjArchive::loadFromFile(const QString& filePath, const QString& tmpPath) {
    ProjArchive ark(filePath, InputMode);
    ark.extract(tmpPath);
}
