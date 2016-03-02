#include "kmm.h"
#include "ui_kmm.h"

#include <QDateTime>
#include "mangaeditor.h"
#include "preferences.h"

class kmmPrivate {
    Q_DECLARE_PUBLIC(kmm)
public:
    kmmPrivate(kmm * parent) : q_ptr(parent), tabCounter(0)
    {
        qsrand(QDateTime::currentMSecsSinceEpoch());
    }

    kmm * const q_ptr;
    int tabCounter;

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

    void preferences() {
        Preferences dlg(q_ptr);
        if (dlg.exec()) {

        }
    }

    void closeTab(QTabWidget* tbW, int index) {
        //TODO: Try to save the project first
        QWidget * tab = tbW->widget(index);
        tbW->removeTab(index);
        delete tab;
        if (!tbW->count())
            newProject(tbW);
    }

};


kmm::kmm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::kmm),
    d_ptr(new kmmPrivate(this))
{
    ui->setupUi(this);
    d_ptr->newProject(ui->kmm_main_tab_widget);

    connect(ui->actNew, &QAction::triggered, [=](){ d_ptr->newProject(ui->kmm_main_tab_widget); });
    connect(ui->actClose, &QAction::triggered, [=](){ d_ptr->closeTab(ui->kmm_main_tab_widget, ui->kmm_main_tab_widget->currentIndex()); });
    connect(ui->actPreferences, &QAction::triggered, [=](){ d_ptr->preferences(); });
    connect(ui->actAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actQuit, &QAction::triggered, this, &kmm::close);

    connect(ui->kmm_main_tab_widget, &QTabWidget::tabCloseRequested, [=](int index) { d_ptr->closeTab(ui->kmm_main_tab_widget, index); });
}

kmm::~kmm() {
    delete d_ptr;
    delete ui;
}
