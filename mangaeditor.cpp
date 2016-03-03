#include "mangaeditor.h"
#include "ui_mangaeditor.h"
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QUuid>

#define DS QDir::separator()

class MangaEditorPrivate {
    Q_DECLARE_PUBLIC(MangaEditor)
public:
    MangaEditorPrivate(MangaEditor * parent) : q_ptr(parent), tabModified(false)
    {

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
        s.setValue(widget->objectName(), widget->date().toString("yyyy-MM-dd"));
        if (setModified) setIsModifiedTab(true);
    }

    void connectWidgetSignals() {
        for (QWidget* widget : q_ptr->findChildren<QWidget*>()) {
            if (widget->inherits("QLineEdit")) {
                const QLineEdit* cw = qobject_cast<const QLineEdit*>(widget);
                cw->connect(cw, &QLineEdit::textChanged, [=](const QString&){ setValueToTemp(cw); });
                continue;
            }
            if (widget->inherits("QComboBox")) {
                const QComboBox* cw = qobject_cast<const QComboBox*>(widget);
                cw->connect(cw, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int){ setValueToTemp(cw); });
                continue;
            }

            if (widget->inherits("QCheckBox")) {
                const QCheckBox* cw = qobject_cast<const QCheckBox*>(widget);
                cw->connect(cw, &QCheckBox::toggled, [=](bool){ setValueToTemp(cw); });
                continue;
            }

            if (widget->inherits("QPlainTextEdit")) {
                const QPlainTextEdit* cw = qobject_cast<const QPlainTextEdit*>(widget);
                cw->connect(cw, &QPlainTextEdit::textChanged, [=](){ setValueToTemp(cw); });
                continue;
            }

            if (widget->inherits("QDateEdit")) {
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

    MangaEditor * const q_ptr;
    QString tmpPath;
    QString chaptersPath;
    QString settingsPath;
    bool tabModified;
};

MangaEditor::MangaEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MangaEditor),
    d_ptr(new MangaEditorPrivate(this))
{
    ui->setupUi(this);
    ui->dteDate->setDate(QDate::currentDate());
    d_ptr->addLanguages(ui->cboLanguage);
    setFocusProxy(ui->txtName);

    d_ptr->createTemporaryFolder();
    d_ptr->connectWidgetSignals();

    connect(ui->btnISBN, &QPushButton::clicked, [=](){ ui->txtISBN->setText(d_ptr->generateISBN()); });

    d_ptr->setValueToTemp(ui->cboOutputType, false);
    d_ptr->setValueToTemp(ui->dteDate, false);
    d_ptr->setValueToTemp(ui->cboLanguage, false);

}

MangaEditor::~MangaEditor() {
    d_ptr->removeTemporaryFolder();
    delete d_ptr;
    delete ui;
}
