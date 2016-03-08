#include "kmm.h"
#include "ui_kmm.h"
#include "about.h"
#include "mangaeditor.h"
#include "preferences.h"
#include "projarchive.h"

#include <iostream>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

class kmmPrivate {
    Q_DECLARE_PUBLIC(kmm)
public:
    kmmPrivate(kmm * parent) : q_ptr(parent), tabCounter(0)
    {
        qsrand(QDateTime::currentMSecsSinceEpoch());
    }

    kmm * const q_ptr;
    int tabCounter;
    QProgressBar* progressBar;
    QStatusBar* statusBar;

    void newProject(QTabWidget* tbW) {
        QWidget * tab = new MangaEditor;
        tabCounter++;
        QString tbTitle(q_ptr->tr("new-project-") + QLocale().toString(tabCounter));
        tbW->setUpdatesEnabled(false);
        tbW->addTab(tab, tbTitle);
        tbW->setUpdatesEnabled(true);
        tbW->setCurrentWidget(tab);
        tab->setFocus();
    }

    void openProject(QTabWidget *tbW) {
        QString filter("Kindle Manga Maker Project (*.kmp)");
        QString fName = QFileDialog::getOpenFileName(
                    q_ptr,
                    kmm::tr("Select Kindle Manga Maker Project File..."),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                    filter, &filter);
        if (!fName.isEmpty() && QFileInfo(fName).exists()) {
            fName = fName.right(4) == ".kmp" ? fName : fName+".kmp";
            MangaEditor * tab = new MangaEditor(fName, 0);
            tab->setProjectFile(fName);
            QString tbTitle(QFileInfo(fName).fileName());
            tbW->setUpdatesEnabled(false);
            tbW->addTab(tab, tbTitle);
            tbW->setUpdatesEnabled(true);
            tbW->setCurrentWidget(tab);
            tab->setFocus();
        }
    }

    bool saveAsProject(QTabWidget* tbW) {
        return saveTabAs(tbW, tbW->currentIndex());
    }

    bool saveTabAs(QTabWidget* tbW, int index) {
        bool rValue = false;
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->widget(index));
        QString filter("Kindle Manga Maker Project (*.kmp)");
        QString fName = QFileDialog::getSaveFileName(
                    q_ptr,
                    kmm::tr("Save Kindle Manga Maker Project..."),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                    filter, &filter);
        if (!fName.isEmpty()) {
            fName = fName.right(4) == ".kmp" ? fName : fName+".kmp";
            tab->setProjectFile(fName);
            QFileInfo fi(fName);
            if (fi.exists()) QDir().remove(fName);
            tab->saveProject(fName);
            tab->setTabModified(false);
            tbW->setTabText(index, fi.fileName());
            rValue = true;
        }
        return rValue;
    }

    bool saveProject(QTabWidget* tbW) {
        return saveTab(tbW, tbW->currentIndex());
    }

    bool saveTab(QTabWidget* tbW, int index) {
        bool rValue = false;
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->widget(index));
        if (tab->projectFile().isEmpty()) {
            rValue = saveTabAs(tbW, index);
        }
        else {
            QString fName = tab->projectFile();
            QFileInfo fi(fName);
            if (fi.exists()) QDir().remove(fName);
            tab->saveProject(fName);
            tab->setTabModified(false);
            tbW->setTabText(index, fi.fileName());
            rValue = true;
        }
        return rValue;
    }

    void preferences() {
        Preferences dlg(q_ptr);
        if (dlg.exec()) {

        }
    }

    bool tryCloseTab(QTabWidget* tbW, int index) {
        bool rValue = false;
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->widget(index));
        if (tab->tabModified()) {
            QMessageBox msg(QMessageBox::Question,
                            kmm::tr("Close Tab"),
                            kmm::tr("This tab has modified information. Do you want to save before closing?"),
                            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                            q_ptr);
            msg.setEscapeButton(QMessageBox::Cancel);
            QMessageBox::StandardButton bt = static_cast<QMessageBox::StandardButton>(msg.exec());
            switch (bt) {
            case QMessageBox::Yes:
                if (!saveProject(tbW)) return rValue;
                break;
            case QMessageBox::Cancel:
                return rValue;
            default:
                break;
            }
        }
        return true;
    }

    bool closeTab(QTabWidget *tbW, int index) {
        bool rValue = false;
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->widget(index));
        if (tryCloseTab(tbW, index)) {
            tbW->removeTab(index);
            rValue = true;
            delete tab;
            if (!tbW->count())
                newProject(tbW);
        }
        return rValue;
    }

    bool quitProgram(QTabWidget* tbW) {
        bool quit = false;

        QList<int> tabsToSave;
        for (int i=0; i<tbW->count(); ++i) {
            MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->widget(i));
            if (tab->tabModified())
                tabsToSave << i;
        }

        if (tabsToSave.size()) {
            QMessageBox msg(QMessageBox::Question,
                            kmm::tr("Quit Program"),
                            kmm::tr("There are tabs with modified information. Do you want to save before closing?"),
                            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                            q_ptr);
            msg.setEscapeButton(QMessageBox::Cancel);
            QMessageBox::StandardButton bt = static_cast<QMessageBox::StandardButton>(msg.exec());
            switch (bt) {
            case QMessageBox::Yes:
                for (int i : tabsToSave) {
                    if (!saveTab(tbW, i)) return quit;
                }
                break;
            case QMessageBox::Cancel:
                return quit;
            default:
                break;
            }
        }

        return true;
    }

    void about() {
        About dlg(QPixmap(":/icons/manga-icon.png"),
                  "© 2016 Vinícius de Ávila Jorge",
                  "",
                  "vinicius.avila.jorge@gmail.com",
                  "<p>Program License: <a href='http://www.gnu.org/licenses/gpl.txt'>GNU GPL version 3</a></p>"
                  "<p>Program Icon created by <a href='http://www.iconarchive.com/show/mega-pack-1-icons-by-ncrow/CDisplay-Manga-icon.html'>ncrow</a> and licensed under <a href='http://creativecommons.org/licenses/by-nc-nd/4.0/'>Creative Commons BY-NC-ND 4.0</a></p>"
                  "<p><a href='http://www.nih.at/libzip/index.html'>Libzip website</a></p>",
                  q_ptr);
        dlg.setWindowModality(Qt::WindowModal);
        dlg.setWebsite("https://www.bitbucket.org/viniavila/kindle-manga-maker", "Visit KMM oficial website!");
        dlg.addButtonLicense(kmm::tr("libZip LICENSE"),
                             kmm::tr("<p>Copyright (C) 1999-2014 Dieter Baron and Thomas Klausner</p>"
                                     "<p>The authors can be contacted at &lt;<a href='mailto:libzip@nih.at'>libzip@nih.at</a>&gt;</p>"
                                     "<p>Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:</p>"
                                     "<p>1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.</p>"
                                     "<p>2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.</p>"
                                     "<p>3. The names of the authors may not be used to endorse or promote products derived from this software without specific prior written permission.</p>"
                                     "<p>THIS SOFTWARE IS PROVIDED BY THE AUTHORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>"));
        dlg.exec();
    }

    void exportCBZ(QTabWidget* tbW) {
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->currentWidget());
        if (tab->analyzeChapterPictures()) {
            QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            QString filter("Comic Book Archive - zip compression (*.cbz)");
            QString fname = QFileDialog::getSaveFileName(q_ptr, kmm::tr("Export ebook to CBZ..."), dir, filter, &filter);
            if (!fname.isEmpty()) {
                if (fname.right(4) != ".cbz") fname.append(".cbz");
                // Generate the output file
            }
        }
    }

    void exportEPUB(QTabWidget* tbW) {
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->currentWidget());
        if (tab->analyzeChapterPictures()) {
            QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            QString filter("Eletronic Publication format (*.epub)");
            QString fname = QFileDialog::getSaveFileName(q_ptr, kmm::tr("Export ebook to EPUB..."), dir, filter, &filter);
            if (!fname.isEmpty()) {
                if (fname.right(5) != ".epub") fname.append(".epub");
                // Generate the output file
            }
        }
    }

    void exportMOBI(QTabWidget* tbW) {
        MangaEditor* tab = qobject_cast<MangaEditor*>(tbW->currentWidget());
        if (tab->analyzeChapterPictures()) {
            QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            QString filter("Mobipocket e-book format (*.mobi)");
            QString fname = QFileDialog::getSaveFileName(q_ptr, kmm::tr("Export ebook to MOBI..."), dir, filter, &filter);
            if (!fname.isEmpty()) {
                if (fname.right(5) != ".mobi") fname.append(".mobi");
                // Generate the output file
            }
        }
    }

};

kmm::kmm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kmm),
    d_ptr(new kmmPrivate(this))
{
    ui->setupUi(this);
    d_ptr->progressBar = new QProgressBar;
    d_ptr->progressBar->setObjectName("status_bar_qprogressbar");
    d_ptr->progressBar->setVisible(false);
    ui->kmm_qstatusbar->addPermanentWidget(d_ptr->progressBar);
    d_ptr->statusBar = ui->kmm_qstatusbar;

    d_ptr->newProject(ui->kmm_main_tab_widget);

    connect(ui->actNew, &QAction::triggered, [=](){ d_ptr->newProject(ui->kmm_main_tab_widget); });
    connect(ui->actOpen, &QAction::triggered, [=](){ d_ptr->openProject(ui->kmm_main_tab_widget); });
    connect(ui->actSave, &QAction::triggered, [=](){ d_ptr->saveProject(ui->kmm_main_tab_widget); });
    connect(ui->actSaveAs, &QAction::triggered, [=](){ d_ptr->saveAsProject(ui->kmm_main_tab_widget); });
    connect(ui->actClose, &QAction::triggered, [=](){ d_ptr->closeTab(ui->kmm_main_tab_widget, ui->kmm_main_tab_widget->currentIndex()); });
    connect(ui->actCBZ, &QAction::triggered, [=](){ d_ptr->exportCBZ(ui->kmm_main_tab_widget); });
    connect(ui->actEPUB, &QAction::triggered, [=](){ d_ptr->exportEPUB(ui->kmm_main_tab_widget); });
    connect(ui->actMOBI, &QAction::triggered, [=](){ d_ptr->exportMOBI(ui->kmm_main_tab_widget); });
    connect(ui->actPreferences, &QAction::triggered, [=](){ d_ptr->preferences(); });
    connect(ui->actAbout, &QAction::triggered, [=](){ d_ptr->about(); });
    connect(ui->actAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actQuit, &QAction::triggered, this, &kmm::close);
    connect(ui->kmm_main_tab_widget, &QTabWidget::tabCloseRequested, [=](int index) { d_ptr->closeTab(ui->kmm_main_tab_widget, index); });
}

kmm::~kmm() {
    delete d_ptr;
    delete ui;
}

void kmm::closeEvent(QCloseEvent *e) {
    if (d_ptr->quitProgram(ui->kmm_main_tab_widget))
        e->accept();
    else
        e->ignore();
}
