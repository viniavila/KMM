#include "bookexporter.h"
#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QProcess>
#include <QProgressBar>
#include <QSettings>
#include <QUuid>

#include "runexternalprocess.h"

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

    struct Chapter {
        QString title;
        QString original_folder;
        bool toTOC;
        int index;
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

    QList<Chapter> getChapters() {
        QSettings s(tmpPath + DS + "content.ini", QSettings::IniFormat);
        QList<Chapter> l;

        QStringList titles = s.value("Chapters/title", "").toString().split("::");
        QStringList folders = s.value("Chapters/folders", "").toString().split("::");
        QStringList toTOC = s.value("Chapters/toTOC", "").toString().split("::");
        if (titles.size() == 1 && titles.first() == "") titles.clear();
        if (folders.size() == 1 && folders.first() == "") folders.clear();
        if (toTOC.size() == 1 && toTOC.first() == "") toTOC.clear();

        for (int i=0; i<qMin(titles.size(), qMin(folders.size(), toTOC.size())); ++i) {
            Chapter ch;
            ch.title = titles.at(i);
            ch.original_folder = folders.at(i);
            ch.toTOC = toTOC.at(i) == "1" ? true : false;
            l << ch;
        }

        return l;
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

    void appendTOC(QString& xml) {
        QSettings s(tmpPath + DS + "content.ini", QSettings::IniFormat);
        QStringList titles = s.value("Chapters/title", "").toString().split("::");
        QStringList toTOC = s.value("Chapters/title", "").toString().split("::");

        if (titles.size() == 1 && titles.first() == "") titles.clear();
        if (toTOC.size() == 1 && toTOC.first() == "") toTOC.clear();

        int cnt = 1;
        if (titles.size() && toTOC.size()) {
            for (int i=0; i<qMin(titles.size(), toTOC.size()); ++i) {
                if (toTOC.at(i).toInt()) {
                    xml.append(QString("    <itemref idref=\"c%1\" />\n").arg(cnt,4,10,QChar('0')));
                    cnt++;
                }
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

    QString containerXML() {
        QString xml;
        xml.append("<?xml version=\"1.0\"?>\n");
        xml.append("<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n");
        xml.append("<rootfiles>\n");
        xml.append("<rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n");
        xml.append("</rootfiles>\n");
        xml.append("</container>");
        return xml;
    }

    QString contentOPF() {
        BookInfo bi = getInfo();
        QString languageISO = language(bi.language);

        QString xml;

        xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?""?>\n");
        xml.append("<package xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"BookID\" version=\"2.0\" >\n");
        xml.append("  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n");

        xml.append("    <dc:title>"+ bi.name +"</dc:title>\n");
        xml.append("    <dc:creator opf:role=\"aut\">"+ bi.author +"</dc:creator>\n");
        xml.append("    <dc:language>"+ languageISO +"</dc:language>\n");
        xml.append("    <dc:publisher>"+ bi.publisher +"</dc:publisher>\n");
        xml.append("    <dc:identifier id=\"BookID\" opf:scheme=\"UUID\">"+ bi.isbn +"</dc:identifier>\n");

        // TODO: ADD MORE TAGS

        xml.append("  </metadata>\n");
        xml.append("  <manifest>\n");
        xml.append("    <item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\" />\n");

        xml.append("    <item id=\"chapter01\" href=\"html/c001/p0001.xhtml\" media-type=\"application/xhtml+xml\" />\n");
        xml.append("    <item id=\"chapter02\" href=\"html/c001/p0002.xhtml\" media-type=\"application/xhtml+xml\" />\n");
        // TODO: ADD MORE PAGES (html/cXXX/pXXXX.xhtml)

        xml.append("    <item id=\"chapter01\" href=\"html/c001/images/p0001.jpg\" media-type=\"image/jpeg\" />\n");
        xml.append("    <item id=\"chapter02\" href=\"html/c001/images/p0002.jpg\" media-type=\"image/png\" />\n");
        // TODO: ADD MORE PICTURES (html/cXXX/images/pXXXX.jpg

        xml.append("  </manifest>\n");
        xml.append("  <spine toc=\"ncx\">\n");
        appendTOC(xml);
        xml.append("  </spine>\n");
        xml.append("</package>");

        return xml;
    }

    QString createTocNcx() {
        QString xml;

        xml.append("<?xml version='1.0' encoding='UTF-8'?>\n");
        xml.append("<!DOCTYPE ncx PUBLIC '-//NISO//DTD ncx 2005-1//EN' 'http://www.daisy.org/z3986/2005/ncx-2005-1.dtd'>\n");
        xml.append("<ncx version=\"2005-1\" xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" xml:lang=\"en-US\">\n");
        xml.append("  <head>\n");
        xml.append("    <meta content=\"\" name=\"dtb:uid\"/>\n");
        xml.append("    <meta content=\"\" name=\"dtb:depth\"/>\n");
        xml.append("    <meta content=\"0\" name=\"dtb:totalPageCount\"/>\n");
        xml.append("    <meta content=\"0\" name=\"dtb:maxPageNumber\"/>\n");
        xml.append("    <meta content=\"true\" name=\"generated\"/>\n");
        xml.append("  </head>\n");
        xml.append("  <docTitle>\n");
        xml.append("    <text/>\n");
        xml.append("  </docTitle>\n");
        xml.append("  <navMap>\n");

//        QList<TocPoint> tocPoints; // Get TOC points from content.ini (first = Title, second = html Page)

//        int pOrder = 1;
//        for (const TocPoint& p : tocPoints) {
//            xml.append(QString("    <navPoint playOrder=\"%1\" id=\"toc-%1\">\n").arg(pOrder));
//            xml.append("      <navLabel>\n");
//            xml.append("        <text>"+ p.title +"</text>\n");
//            xml.append("      </navLabel>\n");
//            xml.append("      <content src=\""+ p.source +"\"/>\n");
//            xml.append("    </navPoint>\n");
//            pOrder++;
//        }

        xml.append("  </navMap>\n");
        xml.append("</ncx>");

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
        // Add META-INF folder
        if (zip_dir_add(za, "META-INF/", ZIP_FL_ENC_UTF_8) < 0) {
            // ERROR ADDING META-INF folder
        }

        // Add OEBPS folder
        if (zip_dir_add(za, "OEBPS/", ZIP_FL_ENC_UTF_8) < 0) {
            // ERROR ADDING OEBPS folder
        }

        // Add mimetype file
        QByteArray mmt("application/epub+zip");
        zip_source_t* zs = zip_source_buffer(za, mmt.data(), mmt.size(), 0);
        if (zs == NULL || zip_file_add(za, "mimetype", zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
            // ERROR ADDING mimetype FILE
            zip_source_free(zs);
        }

        // Add container.xml file
        QByteArray cXml = d->containerXML().toUtf8();
        zs = zip_source_buffer(za, cXml.data(), cXml.size(), 0);
        if (zs == NULL || zip_file_add(za, "META-INF/container.xml", zs, ZIP_FL_OVERWRITE|ZIP_FL_ENC_UTF_8) < 0) {
            // ERROR ADDING container.xml FILE
            zip_source_free(zs);
        }

        // TODO: Add content.opf file


        // TODO: Add toc.ncx file


    }

    if (bar) {
        bar->setVisible(false);
    }

    return true;
}

bool BookExporter::generateMOBI(const QString& filename, const QString& tmpPath, QProgressBar* bar) {
    BookExporter be(filename, tmpPath);
    BookExporterPrivate *d = be.d_ptr;

    QSettings s;
    QString kindlegen = s.value("KINDLEGEN_PATH", QString()).toString();

    if (kindlegen.isEmpty()) {
        // TODO: Not possible generate MOBI files. No KindleGen path was set.
        return false;
    }
    QFileInfo kfi(kindlegen);
    if (kfi.exists()) {
        if (kfi.isFile()) {
            if (!kfi.isExecutable()) {
                qDebug() << "not exec";
                // TODO: Not possible generate MOBI files. Specified KindleGen path is not an executable file
                return false;
            }
        }
        else {
            qDebug() << "not file";
            // TODO: Not possible generate MOBI files. Specified KindleGen path is not a file
            return false;
        }
    }
    else {
        qDebug() << "not exists";
        // TODO: Not possible generate MOBI files. Specified KindleGen path does not exists
        return false;
    }
    qDebug() << kindlegen << "t2";

    // Initializing progressbar
    if (bar) {
        bar->setVisible(true);
        bar->setMinimum(0);
        bar->setMaximum(2*d->pageCount());
        bar->setValue(0);
    }

    // Make html  and pictures folder
    QString htmlPath(d->buildPath + DS + "html");
    QString imgPath(d->buildPath + DS + "images");
    QDir().mkpath(htmlPath);
    QDir().mkpath(imgPath);

    // Copying files to imgPath folder
    QList<BookExporterPrivate::Chapter> chList = d->getChapters();
    int idxCh = 1;
    for (BookExporterPrivate::Chapter& ch : chList) {
        ch.index = idxCh;
        QString chLabel(QString("c%1").arg(idxCh, 3, 10, QChar('0')));
        QString chOrigFolder(tmpPath + DS + "chapters" + DS + ch.original_folder);

        QDir origD(chOrigFolder);
        QStringList files = origD.entryList(QDir::NoDot|QDir::NoDotAndDotDot|QDir::Files, QDir::Name|QDir::LocaleAware);
        int idxPg = 1;
        for (const QString& ofile : files) {
            QString pgLabel(QString("p%1").arg(idxPg, 4, 10, QChar('0')));
            // Copy the file
            QFile f(chOrigFolder + DS + ofile);
            QFileInfo fi(chOrigFolder + DS + ofile);
            f.copy(imgPath + DS + chLabel + "-" + pgLabel + "." + fi.suffix());
            idxPg++;
            if (bar) {
                bar->setValue(bar->value()+1);
                QApplication::processEvents();
            }
        }

        idxCh++;
    }

    // Generate each html in htmlPath folder
    QDir imgD(imgPath);
    QStringList images = imgD.entryList(QDir::NoDot|QDir::NoDotAndDotDot|QDir::Files, QDir::Name|QDir::LocaleAware);
    for (const QString& ifile : images) {
        QFileInfo fi(imgPath + DS + ifile);

        QByteArray html;
        html.append("<!DOCTYPE html>\n");
        html.append("<html>\n");
        html.append("  <head>\n");
        html.append("    <title>"+ fi.completeBaseName() +"</title>\n");
        html.append("  </head>\n");
        html.append("  <body>\n");
        html.append("    <div>\n");
        html.append("      <img src=\"../images/"+ ifile +"\"/>\n");
        html.append("    </div>\n");
        html.append("  </body>\n");
        html.append("</html>\n");

        QFile f(htmlPath + DS + fi.completeBaseName() + ".html");
        if (f.open(QIODevice::WriteOnly)) {
            f.write(html);
            f.close();
        }
        else
            return false;

        if (bar) {
            bar->setValue(bar->value()+1);
            QApplication::processEvents();
        }

    }

    // Generate toc.ncx
    QByteArray ncx;
    ncx.append("<?xml version='1.0' encoding='UTF-8'?>\n");
    ncx.append("<!DOCTYPE ncx PUBLIC '-//NISO//DTD ncx 2005-1//EN' 'http://www.daisy.org/z3986/2005/ncx-2005-1.dtd'>\n");
    ncx.append("<ncx version=\"2005-1\" xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" xml:lang=\"en-US\">\n");
    ncx.append("  <head>\n");
    ncx.append("    <meta content=\"\" name=\"dtb:uid\"/>\n");
    ncx.append("    <meta content=\"\" name=\"dtb:depth\"/>\n");
    ncx.append("    <meta content=\"0\" name=\"dtb:totalPageCount\"/>\n");
    ncx.append("    <meta content=\"0\" name=\"dtb:maxPageNumber\"/>\n");
    ncx.append("    <meta content=\"true\" name=\"generated\"/>\n");
    ncx.append("  </head>\n");
    ncx.append("  <docTitle>\n");
    ncx.append("    <text/>\n");
    ncx.append("  </docTitle>\n");
    ncx.append("  <navMap>\n");
    int pOrder=1;
    for (const BookExporterPrivate::Chapter& ch : chList) {
        ncx.append(QString("    <navPoint playOrder=\"%1\" id=\"toc-%1\">\n").arg(pOrder));
        ncx.append("      <navLabel>\n");
        ncx.append("        <text>"+ ch.title +"</text>\n");
        ncx.append("      </navLabel>\n");
        ncx.append("      <content src=\""+ QString("html/c%1-p0001.html").arg(ch.index, 3, 10, QChar('0')) +"\"/>\n");
        ncx.append("    </navPoint>\n");
        pOrder++;
    }
    ncx.append("  </navMap>\n");
    ncx.append("</ncx>");
    QFile fNcx(d->buildPath + DS + "toc.ncx");
    if (fNcx.open(QIODevice::WriteOnly)) {
        fNcx.write(ncx);
        fNcx.close();
    }
    else
        return false;

    // Copy cover image to build folder
    QString coverPath;
    QStringList cfile = QDir(tmpPath).entryList({"cover-image*"});
    if (!cfile.isEmpty()) {
        coverPath = d->buildPath + DS + cfile.first();
        QFile cf(tmpPath + DS + cfile.first());
        cf.copy(coverPath);
    }

    // Generate content.opf in the build folder
    BookExporterPrivate::BookInfo bi = d->getInfo();
    QString lng = d->language(bi.language);
    QByteArray opf;
    opf.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    opf.append("<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\""+ QUuid::createUuid().toString() +"\">\n");
    opf.append("  <metadata xmlns:opf=\"http://www.idpf.org/2007/opf\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n");
    opf.append("    <meta content=\"comic\" name=\"book-type\"/>\n");
    opf.append("    <meta content=\"true\" name=\"zero-gutter\"/>\n");
    opf.append("    <meta content=\"true\" name=\"zero-margin\"/>\n");
    opf.append("    <meta content=\"true\" name=\"fixed-layout\"/>\n");
    opf.append("    <meta content=\"KindleMangaMaker-1.0\" name=\"generator\"/>\n");
    opf.append("    <dc:title>"+ bi.name  +"</dc:title>\n");
    opf.append("    <dc:language>"+ lng +"</dc:language>\n");
    opf.append("    <dc:creator>"+ bi.author +"</dc:creator>\n");
    opf.append("    <dc:publisher>"+ bi.publisher +"</dc:publisher>\n");
    opf.append("    <dc:date>"+ bi.releaseDate.toString("yyyy-MM-dd") +"</dc:date>\n");
    opf.append("    <dc:description>"+ bi.description +"</dc:description>\n");
    opf.append("    <meta content=\"none\" name=\"orientation-lock\"/>\n");
    opf.append("    <meta content=\""+ QString(bi.leftToRight?"horizontal-lr":"horizontal-rl") +"\" name=\"primary-writing-mode\"/>\n");
    opf.append("    <meta content=\"1280x1280\" name=\"original-resolution\"/>\n");
    opf.append("    <meta content=\"false\" name=\"region-mag\"/>\n");
    opf.append("    <meta content=\"cover-image\" name=\"cover\"/>\n");
    opf.append("    <dc:source>KM2/"+ QString("%1").arg(QDateTime::currentMSecsSinceEpoch(), 0, 16) +"/linux</dc:source>\n");
    opf.append("  </metadata>\n");
    opf.append("  <manifest>\n");
    opf.append("    <item href=\"toc.ncx\" id=\"ncx\" media-type=\"application/x-dtbncx+xml\"/>\n");

    if (!coverPath.isEmpty()) {
        QFileInfo fi(coverPath);
        opf.append("    <item href=\""+ fi.fileName() +"\" id=\"cover-image\" media-type=\"image/jpg\"/>\n");
    }

    QStringList htmlFiles = QDir(htmlPath).entryList({"*.html"}, QDir::NoFilter, QDir::Name|QDir::LocaleAware);
    pOrder = 1;
    for (const QString& hf : htmlFiles) {
        opf.append(QString("    <item href=\"html/%1\" id=\"item-%2\" media-type=\"application/xhtml+xml\"/>\n").arg(hf).arg(pOrder, 4, 10, QChar('0')));
        pOrder++;
    }
    opf.append("  </manifest>\n");
    opf.append("  <spine toc=\"ncx\">\n");
    for (int i=1; i<pOrder; ++i)
        opf.append(QString("    <itemref idref=\"item-%1\" linear=\"yes\"/>\n").arg(i, 4, 10, QChar('0')));
    opf.append("  </spine>\n");
    opf.append("</package>");


    QFile fOpf(d->buildPath + DS + "content.opf");
    if (fOpf.open(QIODevice::WriteOnly)) {
        fOpf.write(opf);
        fOpf.close();
    }
    else
        return false;


    if (bar) {
        bar->setVisible(false);
    }

    // Run the kindlegen utility!
    RunExternalProcess p(kindlegen, {"-dont_append_source", "-locale", "en", "content.opf"}, d->buildPath);
    p.exec();

    // Copy the generated file to destiny
    QFile f(d->buildPath + DS + "content.mobi");
    f.copy(filename);

    return true;
}
