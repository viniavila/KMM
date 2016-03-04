#include "about.h"
#include "ui_about.h"
#include <QMessageBox>
#include <QPushButton>

About::About(const QPixmap& icon, const QString& copyright, const QString& website, const QString& email, const QString& license, QWidget *parent) :
    About(parent)
{
    ui->lblIcon->setPixmap(icon);
    ui->lblCopyright->setText(copyright);
    ui->lblWebsite->setText(website.isEmpty()?QString():QString("<a href=\"" + website + "\">" + website + "</a>"));
    ui->lblEmail->setText(email.isEmpty()?QString():QString("<a href=\"mailto:"+email+"\">"+email+"</a>"));
    ui->lblLicense->setText(license);
}

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    QString nv = qApp->applicationDisplayName() + " " + qApp->applicationVersion();
    ui->lblNameVersion->setText(nv);

    setWindowTitle(ui->lblNameVersion->text());

    QFont f;
    f.setBold(true);
    f.setPointSizeF(1.4*f.pointSizeF());
    ui->lblNameVersion->setFont(f);

    for (QLabel* l : findChildren<QLabel*>()) {
        l->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::LinksAccessibleByKeyboard);
        l->setOpenExternalLinks(true);
    }
}

About::~About() {
    delete ui;
}

void About::setIcon(const QPixmap &icon) {
    ui->lblIcon->setPixmap(icon);
}

void About::setCopyright(const QString &copyright) {
    ui->lblCopyright->setText(copyright);
}

void About::setWebsite(const QString &link, const QString &text) {
    QString wl;
    if (!link.isEmpty())
        wl = "<a href=\"" + link + "\">" + (text.isEmpty()?link:text) + "</a>";
    ui->lblWebsite->setText(wl);
}

void About::setEmail(const QString &email) {
    ui->lblEmail->setText(email.isEmpty()?QString():QString("<a href=\"mailto:"+email+"\">"+email+"</a>"));
}

void About::setLicense(const QString &license) {
    ui->lblLicense->setText(license);
}

void About::addButtonLicense(const QString& caption, const QString& text) {
    QPushButton *btn = new QPushButton(caption);
    ui->licBtnLayout->insertWidget(ui->licBtnLayout->count()-1, btn, 0, Qt::AlignLeft|Qt::AlignVCenter);
    connect(btn, &QPushButton::clicked, [=](){ QMessageBox(QMessageBox::NoIcon, caption, text, QMessageBox::Close, this).exec();  });
}

QString About::copyright() const {
    return ui->lblCopyright->text();
}

QString About::websiteText() const {
    QString wb = ui->lblWebsite->text();
    wb.remove(QRegularExpression("((<a href=\".*\">)|(<\\/a>))"));
    return wb;
}

QString About::websiteLink() const {
    QString wb = ui->lblWebsite->text();
    wb.remove(QRegularExpression("((<a href=\")|(\">.*<\\/a>))"));
    return wb;
}

QString About::email() const {
    QString em = ui->lblEmail->text();
    em.remove(QRegularExpression("((<a href=\".*\">)|(<\\/a>))"));
    return em;
}

QString About::license() const {
    return ui->lblLicense->text();
}
