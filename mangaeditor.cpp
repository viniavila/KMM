#include "mangaeditor.h"
#include "ui_mangaeditor.h"
#include "imageviewer.h"
#include "projarchive.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QSettings>
#include <QStandardPaths>
#include <QUuid>

#define DS QDir::separator()

class MangaEditorPrivate {
    Q_DECLARE_PUBLIC(MangaEditor)
public:
    MangaEditorPrivate(MangaEditor * parent) : q_ptr(parent), tabModified(false)
    {
        excludedWidgets << "qt_spinbox_lineedit";
    }

    void initMangaEditor(Ui::MangaEditor *ui, const QString& filename = QString()) {
        ui->setupUi(q_ptr);
        q_ptr->setFocusProxy(ui->txtName);
        ui->dteDate->setDate(QDate::currentDate());
        ui->tblChapters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->tblChapters->verticalHeader()->setVisible(false);

        addLanguages(ui->cboLanguage);

        if (filename.isEmpty())
            createTemporaryFolder();
        else
            createTemporaryFolder(filename);

        if (!filename.isEmpty()) {
            loadDataToUI(ui->GeneralData);

            // Load Cover
            QStringList files = QDir(tmpPath).entryList({"cover-image.*"});
            if (files.size()) {
                coverPath = tmpPath + DS + files.at(0);
                ui->btnViewCover->setEnabled(true);
                ui->btnRemoveCover->setEnabled(true);
            }

            // Load Chapters
            QSettings s(settingsPath, QSettings::IniFormat);
            QStringList titles = s.value("Chapters/title", "").toString().split("::");
            QStringList folders = s.value("Chapters/folders", "").toString().split("::");
            QStringList toTOC = s.value("Chapters/toTOC", "").toString().split("::");
            for (int i=0; i<titles.size(); ++i) {
                ui->tblChapters->insertRow(i);
                QTableWidgetItem * item = new QTableWidgetItem(titles.at(i));
                item->setCheckState(toTOC.at(i)=="0"?Qt::Unchecked:Qt::Checked);
                item->setData(Qt::UserRole, folders.at(i));
                ui->tblChapters->setItem(i, 0, item);
            }

        }

        connectWidgetSignals(ui->GeneralData);
        setAllValuesToTemp(ui->GeneralData);

        q_ptr->connect(ui->btnISBN, &QPushButton::clicked, [=](){ ui->txtISBN->setText(generateISBN()); });
        q_ptr->connect(ui->btnSelectCover, &QPushButton::clicked, [=](){ selectCover(ui); });
        q_ptr->connect(ui->btnViewCover, &QPushButton::clicked, [=](){ viewCover(); });
        q_ptr->connect(ui->btnRemoveCover, &QPushButton::clicked, [=](){ removeCover(ui); });
        q_ptr->connect(ui->btnSelectPicFolder, &QPushButton::clicked, [=](){ selectPictureFolder(ui); });
        q_ptr->connect(ui->btnAddChapter, &QPushButton::clicked, [=](){ addChapter(ui); });
        q_ptr->connect(ui->btnViewChapter, &QPushButton::clicked, [=]() { viewChapterImages(ui); });
        q_ptr->connect(ui->btnRemoveChapter, &QPushButton::clicked, [=](){ removeChapter(ui); });
        q_ptr->connect(ui->btnUpArrow, &QPushButton::clicked, [=](){ chapterOrderUp(ui); });
        q_ptr->connect(ui->btnDownArrow, &QPushButton::clicked, [=](){ chapterOrderDown(ui); });

        q_ptr->connect(ui->tblChapters, &QTableWidget::cellChanged, [=](int r, int c){ updateChapterInformation(ui, r, c); });
    }

    void loadDataToUI(QWidget * parent) {
        QSettings s(settingsPath, QSettings::IniFormat);
        for (QWidget* widget : parent->findChildren<QWidget*>()) {
            if (widget->inherits("QLineEdit") && !excludedWidgets.contains(widget->objectName())) {
                QLineEdit* cw = qobject_cast<QLineEdit*>(widget);
                cw->setText(s.value(cw->objectName(), QString()).toString());
                continue;
            }
            if (widget->inherits("QComboBox") && !excludedWidgets.contains(widget->objectName())) {
                QComboBox* cw = qobject_cast<QComboBox*>(widget);
                cw->setCurrentIndex(s.value(cw->objectName(), -1).toInt());
                continue;
            }

            if (widget->inherits("QCheckBox") && !excludedWidgets.contains(widget->objectName())) {
                QCheckBox* cw = qobject_cast<QCheckBox*>(widget);
                cw->setChecked(s.value(cw->objectName(), false).toBool());
                continue;
            }

            if (widget->inherits("QPlainTextEdit") && !excludedWidgets.contains(widget->objectName())) {
                QPlainTextEdit* cw = qobject_cast<QPlainTextEdit*>(widget);
                cw->setPlainText(s.value(cw->objectName(), QString()).toString());
                continue;
            }

            if (widget->inherits("QDateEdit") && !excludedWidgets.contains(widget->objectName())) {
                QDateEdit* cw = qobject_cast<QDateEdit*>(widget);
                cw->setDate(s.value(cw->objectName(), QDate::currentDate()).toDate());
                continue;
            }
        }
    }

    void createTemporaryFolder() {
        QByteArray ba = QString("%1").arg(qrand()).toUtf8();
        tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + DS + QString("KMM-");
        tmpPath.append(QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex());
        QDir().mkdir(tmpPath);
        settingsPath = tmpPath + DS + QString("content.ini");
        chaptersPath = tmpPath + DS + QString("chapters");
        QDir().mkdir(chaptersPath);
    }

    void createTemporaryFolder(const QString& filename) {
        QByteArray ba = QString("%1").arg(qrand()).toUtf8();
        tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + DS + QString("KMM-");
        tmpPath.append(QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex());
        QDir().mkdir(tmpPath);
        ProjArchive::loadFromFile(filename, tmpPath);
        settingsPath = tmpPath + DS + QString("content.ini");
        chaptersPath = tmpPath + DS + QString("chapters");
    }

    void removeTemporaryFolder() {
        QDir(tmpPath).removeRecursively();
    }

    void setValueToTemp(const QLineEdit * widget, bool setModified = true) {
        QSettings s(settingsPath, QSettings::IniFormat);
        s.setValue(widget->objectName(), widget->text());
        if (setModified) setIsModifiedTab(true);
    }

    void setValueToTemp(const QComboBox * widget, bool setModified = true) {
        QSettings s(settingsPath, QSettings::IniFormat);
        s.setValue(widget->objectName(), widget->currentIndex());
        if (setModified) setIsModifiedTab(true);
    }

    void setValueToTemp(const QCheckBox * widget, bool setModified = true) {
        QSettings s(settingsPath, QSettings::IniFormat);
        s.setValue(widget->objectName(), widget->isChecked());
        if (setModified) setIsModifiedTab(true);
    }

    void setValueToTemp(const QPlainTextEdit * widget, bool setModified = true) {
        QSettings s(settingsPath, QSettings::IniFormat);
        s.setValue(widget->objectName(), widget->toPlainText());
        if (setModified) setIsModifiedTab(true);
    }

    void setValueToTemp(const QDateEdit * widget, bool setModified = true) {
        QSettings s(settingsPath, QSettings::IniFormat);
        s.setValue(widget->objectName(), widget->date());
        if (setModified) setIsModifiedTab(true);
    }

    void setAllValuesToTemp(QWidget* parent) {
        for (QWidget* widget : parent->findChildren<QWidget*>()) {
            if (widget->inherits("QLineEdit") && !excludedWidgets.contains(widget->objectName())) {
                const QLineEdit* cw = qobject_cast<const QLineEdit*>(widget);
                setValueToTemp(cw, false);
                continue;
            }
            if (widget->inherits("QComboBox") && !excludedWidgets.contains(widget->objectName())) {
                const QComboBox* cw = qobject_cast<const QComboBox*>(widget);
                setValueToTemp(cw, false);
                continue;
            }

            if (widget->inherits("QCheckBox") && !excludedWidgets.contains(widget->objectName())) {
                const QCheckBox* cw = qobject_cast<const QCheckBox*>(widget);
                setValueToTemp(cw, false);
                continue;
            }

            if (widget->inherits("QPlainTextEdit") && !excludedWidgets.contains(widget->objectName())) {
                const QPlainTextEdit* cw = qobject_cast<const QPlainTextEdit*>(widget);
                setValueToTemp(cw, false);
                continue;
            }

            if (widget->inherits("QDateEdit") && !excludedWidgets.contains(widget->objectName())) {
                const QDateEdit* cw = qobject_cast<const QDateEdit*>(widget);
                setValueToTemp(cw, false);
                continue;
            }
        }
    }

    void connectWidgetSignals(QWidget* parent) {
        for (QWidget* widget : parent->findChildren<QWidget*>()) {
            if (widget->inherits("QLineEdit") && !excludedWidgets.contains(widget->objectName())) {
                const QLineEdit* cw = qobject_cast<const QLineEdit*>(widget);
                cw->connect(cw, &QLineEdit::textChanged, [=](const QString&){ setValueToTemp(cw); });
                continue;
            }
            if (widget->inherits("QComboBox") && !excludedWidgets.contains(widget->objectName())) {
                const QComboBox* cw = qobject_cast<const QComboBox*>(widget);
                cw->connect(cw, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int){ setValueToTemp(cw); });
                continue;
            }

            if (widget->inherits("QCheckBox") && !excludedWidgets.contains(widget->objectName())) {
                const QCheckBox* cw = qobject_cast<const QCheckBox*>(widget);
                cw->connect(cw, &QCheckBox::toggled, [=](bool){ setValueToTemp(cw); });
                continue;
            }

            if (widget->inherits("QPlainTextEdit") && !excludedWidgets.contains(widget->objectName())) {
                const QPlainTextEdit* cw = qobject_cast<const QPlainTextEdit*>(widget);
                cw->connect(cw, &QPlainTextEdit::textChanged, [=](){ setValueToTemp(cw); });
                continue;
            }

            if (widget->inherits("QDateEdit") && !excludedWidgets.contains(widget->objectName())) {
                const QDateEdit* cw = qobject_cast<const QDateEdit*>(widget);
                cw->connect(cw, &QDateEdit::dateChanged, [=](){ setValueToTemp(cw); });
                continue;
            }
        }
    }

    void setIsModifiedTab(bool b) {
        tabModified = b;
        QTabWidget* widget = q_ptr->window()->findChild<QTabWidget*>("kmm_main_tab_widget");
        int index = widget->indexOf(q_ptr);
        QString tabText = widget->tabText(index);
        if (b) {
            if (tabText.right(4) != " [*]")
                tabText.append(" [*]");
        }
        else {
            if (tabText.right(4) == " [*]")
                tabText.remove(" [*]");
        }
        widget->setTabText(index, tabText);
    }

    QString generateISBN() {
        QString uuid = QUuid::createUuid().toString();
        uuid.remove('{').remove('}');
        return uuid;
    }

    void addLanguages(QComboBox* cb) {
//        (PT, EN, SP, EO, DE, FR, IT, JP, CN, RS, AR, HB)
        QLocale l;
        QList<QLocale::Language> languages;
        languages << QLocale::English << QLocale::Portuguese << QLocale::Spanish << QLocale::Esperanto
                  << QLocale::German << QLocale::French << QLocale::Italian << QLocale::Japanese
                  << QLocale::Chinese << QLocale::Russian << QLocale::Arabic << QLocale::Hebrew;
        QStringList languageAbbrev({"en", "pt", "es", "eo", "de", "fr", "it", "ja", "zh", "ru", "ar", "he"});

        for (int i=0; i<qMin(languages.size(),languageAbbrev.size()); ++i)
            cb->addItem(QLocale::languageToString(languages.at(i)), languageAbbrev.at(i));

        cb->setCurrentIndex(0);
    }

    void selectCover(Ui::MangaEditor *ui) {
        QList<QByteArray> supported = QImageReader::supportedImageFormats();
        QString filter("All supported images (");
        for (const QByteArray& ext : supported)
            filter.append("*.").append(ext).append(" ");
        filter.replace(filter.size()-1, 1, ")");
        QString fname = QFileDialog::getOpenFileName(q_ptr, MangaEditor::tr("Select Cover"),
                                                     QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                     filter, &filter);
        if (!fname.isEmpty()) {
            QFile f(fname);
            QFileInfo fi(fname);
            QString ext = fi.suffix();
            coverPath = tmpPath + DS + "cover-image." + ext;
            if (QFileInfo(coverPath).exists())
                QDir(tmpPath).remove(coverPath);
            f.copy(coverPath);
            ui->btnViewCover->setEnabled(true);
            ui->btnRemoveCover->setEnabled(true);
            setIsModifiedTab(true);
        }
    }

    void removeCover(Ui::MangaEditor *ui) {
        QDir tmp(tmpPath);
        QStringList files = tmp.entryList({"cover-image.*"});
        if (files.size()) {
            tmp.remove(files.at(0));
            coverPath.clear();
            ui->btnViewCover->setEnabled(false);
            ui->btnRemoveCover->setEnabled(false);
            setIsModifiedTab(true);
        }
    }

    void viewCover() {
        QDir tmp(tmpPath);
        QStringList files = tmp.entryList({"cover-image.*"});
        if (files.size()) {
            ImageViewer dlg(coverPath, q_ptr);
            q_ptr->connect(&dlg, &ImageViewer::imageRotated, [=](){ setIsModifiedTab(true); });
            dlg.setWindowTitle(MangaEditor::tr("View Cover"));
            dlg.exec();
        }
    }

    void selectPictureFolder(Ui::MangaEditor *ui) {
        QString folder = QFileDialog::getExistingDirectory(q_ptr,
                                                           MangaEditor::tr("Select Chapter Folder"),
                                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        ui->txtPicFolder->setText(folder);
    }

    void addChapter(Ui::MangaEditor *ui) {
        QString title = ui->chkChapterTitle->isChecked() ? ui->txtChapterTitle->text() : "";
        QString origFolder = ui->txtPicFolder->text();
        if (origFolder.isEmpty()) return;

        // Create Chapter Temporary Folder
        QByteArray ba = QString("%1").arg(qrand()).toUtf8();
        QString chapterPath = chaptersPath + DS;
        QString chapterFolder = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
        chapterPath.append(chapterFolder);
        QDir().mkdir(chapterPath);

        // Copy just the pics to Chapter Temporary Folder
        QDir dOrig(origFolder);
        QStringList files = dOrig.entryList(QDir::Files|QDir::NoDotAndDotDot|QDir::Readable, QDir::Name|QDir::LocaleAware);
        for (const QString& f : files) {
            QString fPath(origFolder + DS + f);
            QImageReader reader(fPath);
            if (reader.format().isEmpty()) continue; // Ignore the file if it is not an image.
            QFile file(fPath);
            file.copy(chapterPath + DS + f);
        }

        // Add Basic Information to content.ini
        QSettings s(settingsPath, QSettings::IniFormat);
        QString titles = s.value("Chapters/title", "").toString();
        QString folders = s.value("Chapters/folders", "").toString();
        QString toTOC = s.value("Chapters/toTOC", "").toString();
        titles.append((titles.isEmpty() ? "" : "::") + title);
        folders.append((folders.isEmpty() ? "" : "::") + chapterFolder);
        toTOC.append((toTOC.isEmpty() ? "" : "::") + QString(title.isEmpty()?"0":"1"));
        s.setValue("Chapters/title", titles);
        s.setValue("Chapters/folders" , folders);
        s.setValue("Chapters/toTOC", toTOC);

        // Update the table in the UI
        int row = ui->tblChapters->rowCount();
        ui->tblChapters->insertRow(row);
        QTableWidgetItem * item = new QTableWidgetItem(title);
        item->setCheckState(title.isEmpty()?Qt::Unchecked:Qt::Checked);
        item->setData(Qt::UserRole, chapterFolder);
        ui->tblChapters->setItem(row, 0, item);

        // Finish
        ui->txtPicFolder->clear();
        ui->txtChapterTitle->clear();
        ui->chkChapterTitle->setChecked(false);
        ui->btnSelectPicFolder->setFocus();
        setIsModifiedTab(true);
    }

    void removeChapter(Ui::MangaEditor *ui) {
        // TODO: Confirm before delete

        // Get the Selected line
        QList<QTableWidgetItem*> selected = ui->tblChapters->selectedItems();
        if (selected.isEmpty()) return;
        QTableWidgetItem *item = selected.first();
        int row = item->row();

        // Delete the folder from Temporary path
        QString chapterFolder = chaptersPath + DS + item->data(Qt::UserRole).toString();
        QDir d(chapterFolder);
        d.removeRecursively();

        // Remove Information from content.ini
        QSettings s(settingsPath, QSettings::IniFormat);
        QStringList titles = s.value("Chapters/title", "").toString().split("::");
        QStringList folders = s.value("Chapters/folders", "").toString().split("::");
        QStringList toTOC = s.value("Chapters/toTOC", "").toString().split("::");
        titles.removeAt(row);
        folders.removeAt(row);
        toTOC.removeAt(row);
        s.setValue("Chapters/title", titles.join("::"));
        s.setValue("Chapters/folders" , folders.join("::"));
        s.setValue("Chapters/toTOC", toTOC.join("::"));

        // Update the table in the UI
        ui->tblChapters->removeRow(row);

        // Finish
        ui->tblChapters->selectRow(row-1);
        ui->tblChapters->setFocus();
        setIsModifiedTab(true);
    }

    void viewChapterImages(Ui::MangaEditor *ui) {
        QList<QTableWidgetItem*> selected = ui->tblChapters->selectedItems();
        if (selected.isEmpty()) return;
        QTableWidgetItem *item = selected.first();
        QString chapterFolder = chaptersPath + DS + item->data(Qt::UserRole).toString();

        ImageViewer dlg(chapterFolder, q_ptr);
        q_ptr->connect(&dlg, &ImageViewer::imageRotated, [=](){ setIsModifiedTab(true); });
        dlg.setWindowTitle(item->data(Qt::DisplayRole).toString());
        dlg.exec();
    }

    void updateChapterInformation(Ui::MangaEditor *ui, int row, int col=0) {
        // Remove Informatzion from content.ini
        QSettings s(settingsPath, QSettings::IniFormat);
        QStringList titles = s.value("Chapters/title", "").toString().split("::");
        QStringList folders = s.value("Chapters/folders", "").toString().split("::");
        QStringList toTOC = s.value("Chapters/toTOC", "").toString().split("::");
        titles.replace(row, ui->tblChapters->item(row, col)->text());
        folders.replace(row, ui->tblChapters->item(row, col)->data(Qt::UserRole).toString());
        toTOC.replace(row, ui->tblChapters->item(row, col)->checkState()==Qt::Checked?"1":"0");
        s.setValue("Chapters/title", titles.join("::"));
        s.setValue("Chapters/folders", folders.join("::"));
        s.setValue("Chapters/toTOC", toTOC.join("::"));
        setIsModifiedTab(true);
    }

    void chapterOrderUp(Ui::MangaEditor *ui) {
        QList<QTableWidgetItem*> items = ui->tblChapters->selectedItems();
        if (items.isEmpty() || ui->tblChapters->rowCount() == 1) return;
        QTableWidgetItem *item = items.first();
        if (item->row() == 0) return;

        int row = item->row();
        QTableWidgetItem *item_down = new QTableWidgetItem(* ui->tblChapters->item(row, 0));
        QTableWidgetItem *item_up = new QTableWidgetItem(* ui->tblChapters->item(row-1, 0));
        ui->tblChapters->setItem(row, 0, item_up);
        ui->tblChapters->setItem(row-1, 0, item_down);
        ui->tblChapters->selectRow(row-1);
    }

    void chapterOrderDown(Ui::MangaEditor *ui) {
        QList<QTableWidgetItem*> items = ui->tblChapters->selectedItems();
        if (items.isEmpty() || ui->tblChapters->rowCount() == 1) return;
        QTableWidgetItem *item = items.first();
        if (item->row() == ui->tblChapters->rowCount()-1) return;

        int row = item->row();
        QTableWidgetItem *item_up = new QTableWidgetItem(* ui->tblChapters->item(row, 0));
        QTableWidgetItem *item_down = new QTableWidgetItem(* ui->tblChapters->item(row+1, 0));
        ui->tblChapters->setItem(row, 0, item_down);
        ui->tblChapters->setItem(row+1, 0, item_up);
        ui->tblChapters->selectRow(row+1);
    }

    MangaEditor * const q_ptr;
    QString tmpPath;
    QString chaptersPath;
    QString settingsPath;
    QString coverPath;
    bool tabModified;
    QString projectFile;
    QStringList excludedWidgets;
};

MangaEditor::MangaEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MangaEditor),
    d_ptr(new MangaEditorPrivate(this))
{
    d_ptr->initMangaEditor(ui);
}

MangaEditor::MangaEditor(const QString& filename, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MangaEditor),
    d_ptr(new MangaEditorPrivate(this))
{
    d_ptr->initMangaEditor(ui, filename);
}

MangaEditor::~MangaEditor() {
    d_ptr->removeTemporaryFolder();
    delete d_ptr;
    delete ui;
}

void MangaEditor::setProjectFile(const QString& s) {
    if (d_ptr->projectFile != s)
        d_ptr->projectFile = s;
}

QString MangaEditor::projectFile() const {
    return d_ptr->projectFile;
}

void MangaEditor::setTabModified(bool b) {
    if (d_ptr->tabModified != b)
        d_ptr->setIsModifiedTab(b);
}

bool MangaEditor::tabModified() const {
    return d_ptr->tabModified;
}

QString MangaEditor::tempPath() const {
    return d_ptr->tmpPath;
}
