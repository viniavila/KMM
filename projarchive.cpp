#include "projarchive.h"
#include <zip.h>
#include <QDir>

class ProjArchivePrivate {
    Q_DECLARE_PUBLIC(ProjArchive)
public:
    ProjArchivePrivate(ProjArchive* parent, const QString& path, ProjArchive::IOMode io) :
        q_ptr(parent),
        path(path.toUtf8().data()),
        io(io),
        errCode(0),
        isValid(false)
    {
        if (io == ProjArchive::InputMode) za = zip_open(this->path, ZIP_CHECKCONS, &errCode);
        else za = zip_open(this->path, ZIP_CREATE|ZIP_EXCL, &errCode);

        if (errCode) {
            // TODO: throw error
        }
        else {
            isValid = true;
            if (io == ProjArchive::InputMode) {
                // Check if "content.ini" and folder "chapters/" exists
                int iCi = zip_name_locate(za, "content.ini", 0);
                int iCh = zip_name_locate(za, "chapters/", 0);
                if (iCi == -1 || iCh == -1) {
                    isValid = false;
                    zip_close(za);
                    za = NULL;
                }
            }
        }

    }

    ~ProjArchivePrivate() {
        if (za != NULL) zip_close(za);
    }

    void addRecursiveDir(const QString& dirPath, const QString& arkD = QString()) {
        QDir dir(dirPath);

        // List and add all files in the dir
        QStringList files = dir.entryList(QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name|QDir::LocaleAware);
        for (const QString& f : files) {
            const char* path = QString(dirPath + "/" + f).toUtf8().data();
            zip_source_t * zs = zip_source_file(za, path, 0, -1);
            QString tag(arkD+f);
            if (zs == NULL || zip_file_add(za, tag.toUtf8().data(), zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
                // Throw error adding the file
                zip_source_free(zs);
            }
        }

        // List and add each dir recursively
        QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Readable|QDir::Executable, QDir::Name|QDir::LocaleAware);
        for (const QString& d : dirs) {
            QString tag(arkD+d+"/");
            zip_dir_add(za, tag.toUtf8().data(), ZIP_FL_ENC_UTF_8);
            QString nDirPath(dirPath+"/"+d);
            addRecursiveDir(nDirPath, tag);
        }
    }

    ProjArchive * const q_ptr;
    const char* path;
    ProjArchive::IOMode io;
    int errCode;
    bool isValid;
    zip_t* za;
};

ProjArchive::ProjArchive(const QString& path, IOMode io) :
    d_ptr(new ProjArchivePrivate(this, path, io))
{

}

ProjArchive::~ProjArchive() {
    delete d_ptr;
}

void ProjArchive::save(const QString &tmpPath) {
    d_ptr->addRecursiveDir(tmpPath);
}

void ProjArchive::extract(const QString &tmpPath) const {
    for (int i=0; i<zip_get_num_entries(d_ptr->za, ZIP_FL_UNCHANGED); ++i) {
        QString fName(zip_get_name(d_ptr->za, i, ZIP_FL_ENC_UTF_8));
        if (fName.right(1) == "/") {
            fName = tmpPath + "/" + fName;
            if (!QDir(fName).exists())
                QDir().mkpath(fName);
        }
        else {
            struct zip_stat stat;
            zip_file_t* zf = zip_fopen_index(d_ptr->za, i, ZIP_FL_UNCHANGED);
            zip_stat_init(&stat);
            zip_stat_index(d_ptr->za, i, ZIP_FL_ENC_UTF_8, &stat);
            char* buffer = new char[stat.size];
            const zip_int64_t read = zip_fread(zf, buffer, stat.size);
            if (read <= 0) {
                // Throw error
            }
            else {
                QByteArray array(buffer, stat.size);
                QFileInfo fi(tmpPath + "/" + fName);

                if (!fi.absoluteDir().exists())
                    fi.dir().mkpath(fi.absoluteDir().absolutePath());

                QFile f(fi.absoluteFilePath());
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(array);
                    f.close();
                }
            }
            zip_fclose(zf);
            delete[] buffer;
        }
    }
}

bool ProjArchive::isValid() const {
    return d_ptr->isValid;
}

ProjArchive::IOMode ProjArchive::ioMode() const {
    return d_ptr->io;
}

void ProjArchive::saveToFile(const QString& filePath, const QString& tmpPath) {
    ProjArchive ark(filePath, OutputMode);
    if (ark.isValid())
        ark.save(tmpPath);
}

void ProjArchive::loadFromFile(const QString& filePath, const QString& tmpPath) {
    ProjArchive ark(filePath, InputMode);
    if (ark.isValid())
        ark.extract(tmpPath);
}
