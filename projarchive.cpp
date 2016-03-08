#include "projarchive.h"

#include <iostream>
#include <zip.h>

#include <QDir>
#include <QProgressBar>
#include <QApplication>

#define DS QDir::separator()

class ProjArchivePrivate {
    Q_DECLARE_PUBLIC(ProjArchive)
public:
    ProjArchivePrivate(ProjArchive* parent, const QString& path, ProjArchive::IOMode io) :
        q_ptr(parent),
        path(path),
        io(io),
        errCode(0),
        isValid(false),
        pbar(0)
    {
        if (io == ProjArchive::InputMode) za = zip_open(this->path.toUtf8().data(), ZIP_CHECKCONS, &errCode);
        else za = zip_open(this->path.toUtf8().data(), ZIP_CREATE|ZIP_EXCL, &errCode);

        if (errCode) {
            // Throw Error
            char errMessage[100];
            zip_error_to_str(errMessage, 100, errCode, errno);
            std::cout << "ERROR loading the ZIP file: " << errMessage << std::endl;
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
                    std::cout << "ERROR: File appears not to be a Kindle Manga Project (.kmp) file." << std::endl;
                }
            }
        }

    }

    ~ProjArchivePrivate() {
        if (za != NULL) zip_close(za);
    }

    int countDirsAndFilesRecursively(const QString& dpath) {
        QDir d(dpath);
        int i = d.entryList(QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name|QDir::LocaleAware).count();
        QStringList subdirs = d.entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Readable|QDir::Executable, QDir::Name|QDir::LocaleAware);
        for (const QString& sd : subdirs) {
            QString nDirPath(dpath+ DS + sd);
            i++;
            i += countDirsAndFilesRecursively(nDirPath);
        }
        return i;
    }

    ProjArchive * const q_ptr;
    QString path;
    ProjArchive::IOMode io;
    int errCode;
    bool isValid;
    zip_t* za;
    QProgressBar *pbar;
};

ProjArchive::ProjArchive(const QString& path, IOMode io) :
    d_ptr(new ProjArchivePrivate(this, path, io))
{

}

ProjArchive::~ProjArchive() {
    delete d_ptr;
}

void ProjArchive::save(const QString &tmpPath) {
    if (d_ptr->pbar) {
        d_ptr->pbar->setVisible(true);
        d_ptr->pbar->setMinimum(0);
        d_ptr->pbar->setMaximum(d_ptr->countDirsAndFilesRecursively(tmpPath));
        d_ptr->pbar->setValue(0);
    }

    QStringList paths({tmpPath});
    QStringList tags({""});

    while (paths.count()) {
        QString dirPath = paths.first();
        QString arkD = tags.first();
        QDir dir(dirPath);

        // List and add all files in the dir
        QStringList files = dir.entryList(QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name|QDir::LocaleAware);
        for (const QString& f : files) {
            QString fPath(dirPath + DS + f);
            QString tag = QString(arkD+f);
            zip_source_t * zs = zip_source_file(d_ptr->za, fPath.toUtf8().data(), 0, -1);
            if (zs == NULL || zip_file_add(d_ptr->za, tag.toUtf8().data(), zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
                // Throw error adding the file
                const char* errMessage = zip_strerror(d_ptr->za);
                std::cout << "ERROR Adding the file to ZIP: " << errMessage << std::endl;
                zip_source_free(zs);
            }
            if (d_ptr->pbar) {
                d_ptr->pbar->setValue(d_ptr->pbar->value()+1);
            }
        }

        // List and add all dirs in the dir. Append the dirs to paths
        QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Readable|QDir::Executable, QDir::Name|QDir::LocaleAware);
        for (const QString& d : dirs) {
            QString tag(arkD+d+"/");
            QString nDirPath(dirPath+ DS +d);
            zip_dir_add(d_ptr->za, tag.toUtf8().data(), ZIP_FL_ENC_UTF_8);
            paths << nDirPath;
            tags << tag;
            if (d_ptr->pbar) {
                d_ptr->pbar->setValue(d_ptr->pbar->value()+1);
            }
        }

        paths.removeFirst();
        tags.removeFirst();

        // The only way to flush the added files to disk is closing the archive
        // So, to track the progress, we have to close the archive after add a dir then open it again to receive another dir and its content
        zip_close(d_ptr->za);
        d_ptr->za = zip_open(d_ptr->path.toUtf8().data(), 0, &d_ptr->errCode);
    }

    if (d_ptr->pbar) {
        d_ptr->pbar->setVisible(false);
    }
}

void ProjArchive::extract(const QString &tmpPath) const {
    int nE = zip_get_num_entries(d_ptr->za, ZIP_FL_UNCHANGED);
    if (d_ptr->pbar) {
        d_ptr->pbar->setVisible(true);
        d_ptr->pbar->setMinimum(0);
        d_ptr->pbar->setMaximum(nE-1);
        d_ptr->pbar->setValue(0);
    }
    for (int i=0; i<nE; ++i) {
        QString fName(zip_get_name(d_ptr->za, i, ZIP_FL_ENC_UTF_8));
        if (fName.right(1) == "/") {
            fName = tmpPath + DS + fName;
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
                const char* errMessage = zip_file_strerror(zf);
                std::cout << "ERROR Reading the ZIP file: " << errMessage << std::endl;
            }
            else {
                QFileInfo fi(tmpPath + DS + fName);

                if (!fi.absoluteDir().exists())
                    fi.dir().mkpath(fi.absoluteDir().absolutePath());

                QFile f(fi.absoluteFilePath());
                if (f.open(QIODevice::WriteOnly)) {
                    f.write(buffer, stat.size);
                    f.close();
                }
            }
            zip_fclose(zf);
            delete[] buffer;
        }
        if (d_ptr->pbar) {
            d_ptr->pbar->setValue(d_ptr->pbar->value()+1);
            QApplication::processEvents();
        }
    }
    if (d_ptr->pbar) {
        d_ptr->pbar->setVisible(false);
    }
}

bool ProjArchive::isValid() const {
    return d_ptr->isValid;
}

ProjArchive::IOMode ProjArchive::ioMode() const {
    return d_ptr->io;
}

void ProjArchive::setProgressBar(QProgressBar *bar) {
    d_ptr->pbar = bar;
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
