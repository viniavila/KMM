#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    About(const QPixmap& icon,
          const QString& copyright = QString(),
          const QString& website = QString(),
          const QString& email = QString(),
          const QString& license = QString(),
          QWidget *parent = 0);
    About(QWidget *parent = 0);
    ~About();

    void setIcon(const QPixmap& icon);
    void setCopyright(const QString& copyright);
    void setWebsite(const QString& link, const QString& text = QString());
    void setEmail(const QString& email);
    void setLicense(const QString& license);
    void addButtonLicense(const QString& caption, const QString& text);

    QString copyright() const;
    QString websiteText() const;
    QString websiteLink() const;
    QString email() const;
    QString license() const;

private:
    Ui::About *ui;
};

#endif // ABOUT_H
