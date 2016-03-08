#include "bookexporter.h"
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QProgressBar>
#include <QSettings>

#include <zip.h>

#define DS QDir::separator()

class BookExporterPrivate {
    Q_DECLARE_PUBLIC(BookExporter)
public:
    BookExporterPrivate(BookExporter * parent) : q_ptr(parent), errCode(0) { }
    BookExporter * const q_ptr;
    QString filename;
    QString tmpPath;

    QString buildPath;
    int errCode;

    struct BookInfo {
        int language;
        int outputType;
        bool leftToRight;
        QDate releaseDate;
        QString author;
        QString description;
        QString isbn;
        QString name;
        QString publisher;
    };

    QString language(int i) {
        QStringList languageAbbrev({"en", "pt", "es", "eo", "de", "fr", "it", "ja", "zh", "ru", "ar", "he"});
        return languageAbbrev.at(i);
    }

    int pageCount() {
        int i=0;
        QString chaptersPath(tmpPath + DS + "chapters");
        QDir d(chaptersPath);
        QStringList chapters = d.entryList(QDir::NoDot|QDir::NoDotAndDotDot|QDir::Dirs);
        for (const QString& s : chapters) {
            QDir cd(chaptersPath + DS + s);
            i+= cd.entryList(QDir::NoDot|QDir::NoDotAndDotDot|QDir::Files).size();
        }
        return i;
    }

    BookInfo getInfo() {
        QSettings s(tmpPath + DS + "content.ini", QSettings::IniFormat);
        BookInfo b;
        b.language = s.value("cboLanguage", 0).toInt();
        b.outputType = s.value("cboOutputType", 0).toInt();
        b.leftToRight = s.value("chkLR", false).toBool();
        b.releaseDate = s.value("dteDate", QDate::currentDate()).toDate();
        b.author = s.value("txtAuthor", QString()).toString();
        b.description = s.value("txtDescription", QString()).toString();
        b.isbn = s.value("txtISBN", QString()).toString();
        b.name = s.value("txtName", QString()).toString();
        b.publisher = s.value("txtPublisher", QString()).toString();
        return b;
    }

    bool appendCover(QString& xml) {
        QDir d(tmpPath);
        QStringList fc = d.entryList({"cover*"});
        if (fc.size()) {
            QFileInfo fi(tmpPath + DS + fc.first());
            xml.append(QString("    <Page Image=\"0\" ImageSize=\"%1\" Type=\"FrontCover\" />\n").arg(fi.size()));
            return true;
        }
        return false;
    }

    void appendPages(QString& xml, int start) {
        QSettings s(tmpPath + DS + "content.ini", QSettings::IniFormat);
        QStringList folders = s.value("Chapters/folders", "").toString().split("::");
        if (folders.size() == 1 && folders.first() == "") folders.clear();
        for (const QString& fd : folders) {
            QString dd(tmpPath + DS + "chapters" + DS + fd);
            QDir d(dd);
            QStringList files = d.entryList(QDir::NoDot|QDir::NoDotAndDotDot|QDir::Files, QDir::Name|QDir::LocaleAware);
            for (const QString& f : files) {
                QFileInfo fi(dd + DS + f);
                xml.append(QString("    <Page Image=\"%1\" ImageSize=\"%2\" />\n").arg(start).arg(fi.size()));
                start++;
            }
        }
    }

    QString comicInfoXML() {
        QString xml;
        BookInfo b = getInfo();
        int pages = pageCount();
        QString languageISO = language(b.language);
        QString readingDirection = b.leftToRight ? "No" : "YesAndRightToLeft";

        // Starting
        xml.append("<?xml version=\"1.0\"?>\n"
                   "<ComicInfo xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
        xml.append("  <Title>"+b.name+"</Title>\n");
//        xml.append("  <Series></Series>\n");
//        xml.append("  <Number></Number>\n");
//        xml.append("  <Volume></Volume>\n");
        xml.append(QString("  <Year>%1</Year>\n").arg(b.releaseDate.year()));
        xml.append(QString("  <Month>%1</Month>\n").arg(b.releaseDate.month()));
        xml.append(QString("  <Day>%1</Day>\n").arg(b.releaseDate.day()));
        xml.append("  <Writer>"+b.author+"</Writer>\n");
        xml.append("  <Publisher>"+b.publisher+"</Publisher>\n");
        xml.append(QString("  <PageCount>%1</PageCount>\n").arg(pages));
        xml.append("  <LanguageISO>"+languageISO+"</LanguageISO>\n");
        xml.append("  <Manga>"+readingDirection+"</Manga>\n");
        xml.append("  <Pages>\n");

        // Append Cover
        int st = 0;
        if (appendCover(xml)) st++;

        // Append Pages
        appendPages(xml, st);

        // Finishing
        xml.append("  </Pages>\n");
        xml.append("</ComicInfo>");

        return xml;
    }

};

BookExporter::BookExporter(const QString& filename, const QString tmpPath) :
    d_ptr(new BookExporterPrivate(this))
{
    d_ptr->filename = filename;
    d_ptr->tmpPath = tmpPath;
    d_ptr->buildPath = tmpPath + DS + "build";
    QDir().mkpath(d_ptr->buildPath);
}

BookExporter::~BookExporter() {
    QDir(d_ptr->buildPath).removeRecursively();
    delete d_ptr;
}

bool BookExporter::generateCBZ(const QString& filename, const QString& tmpPath, QProgressBar* bar) {
    BookExporter be(filename, tmpPath);
    BookExporterPrivate *d = be.d_ptr;

    // Initializing progressbar
    if (bar) {
        bar->setVisible(true);
        bar->setMinimum(0);
        bar->setMaximum(d->pageCount());
        bar->setValue(0);
    }

    // Creating the file
    zip_t* za = zip_open(filename.toUtf8().data(), ZIP_CREATE|ZIP_EXCL, &d->errCode);
    if (d->errCode) {
        // SOME ERROR OCCURRED CREATING FILE
    }
    else {

        // Append the ComicInfo.xml
        QString xml = d->comicInfoXML();
        QByteArray xmlb = xml.toUtf8();
        zip_source_t* zs = zip_source_buffer(za, xmlb.data(), xml.size(), 0);
        if (zs == NULL || zip_file_add(za, "ComicInfo.xml", zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
            // ERROR ADDING FILE
            zip_source_free(zs);
        }

        int pCount = 0;

        // Append the cover
        QDir dd(tmpPath);
        QStringList fc = dd.entryList({"cover*"});
        if (fc.size()) {
            QString cv(tmpPath + DS + fc.first());
            QFileInfo fi(cv);
            QString tag(QString("a%1_cover.%2").arg(pCount,4,10,QChar('0')).arg(fi.suffix()));
            pCount++;
            zip_source_t* zs = zip_source_file(za, cv.toUtf8().data(), 0, -1);
            if (zs == NULL || zip_file_add(za, tag.toUtf8().data(), zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
                // ERROR ADDING COVER FILE
                zip_source_free(zs);
            }
        }

        zip_close(za);

        // Append each folder
        QSettings s(tmpPath + DS + "content.ini", QSettings::IniFormat);
        QStringList folders = s.value("Chapters/folders", "").toString().split("::");
        if (folders.size() == 1 && folders.first() == "") folders.clear();
        int i=0;
        for (const QString& fd : folders) {
            za = zip_open(filename.toUtf8().data(), 0, &d->errCode);
            QString tag(QString("c%1").arg(i,4,10,QChar('0')));
            zip_dir_add(za, tag.toUtf8().data(), ZIP_FL_ENC_UTF_8);

            // Append files inside folders
            QString dd(tmpPath + DS + "chapters" + DS + fd);
            QDir d(dd);
            QStringList files = d.entryList(QDir::NoDot|QDir::NoDotAndDotDot|QDir::Files, QDir::Name|QDir::LocaleAware);
            for (const QString& f : files) {
                QString fp(dd + DS + f);
                QFileInfo fi(fp);
                QString ftag(QString("%1/a%2.%3").arg(tag).arg(pCount,4,10,QChar('0')).arg(fi.suffix()));
                pCount++;
                zip_source_t* zs = zip_source_file(za, fp.toUtf8().data(), 0, -1);
                if (zs == NULL || zip_file_add(za, ftag.toUtf8().data(), zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
                    // ERROR ADDING FILE
                    zip_source_free(zs);
                }
                if (bar) {
                    bar->setValue(bar->value()+1);
                    QApplication::processEvents();
                }
            }

            i++;
            zip_close(za);
        }

    }

    if (bar) {
        bar->setVisible(false);
    }

    return true;
}

bool BookExporter::generateEPUB(const QString& filename, const QString& tmpPath, QProgressBar* bar) {
    BookExporter be(filename, tmpPath);
    BookExporterPrivate *d = be.d_ptr;

    return true;
}

bool BookExporter::generateMOBI(const QString& filename, const QString& tmpPath, QProgressBar* bar) {
    BookExporter be(filename, tmpPath);
    BookExporterPrivate *d = be.d_ptr;

    return true;
}
