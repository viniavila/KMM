#ifndef PICANALYZERESULT_H
#define PICANALYZERESULT_H

#include <QDialog>

namespace Ui {
class PicAnalyzeResult;
}

class PicAnalyzeResultPrivate;
class PicAnalyzeResult : public QDialog {
    Q_OBJECT
    Q_DECLARE_PRIVATE(PicAnalyzeResult)
public:
    PicAnalyzeResult(QWidget *parent = 0);
    PicAnalyzeResult(const QStringList& pictures, int defAction, bool readingLR = false, QWidget *parent = 0);
    ~PicAnalyzeResult();
    bool modified() const;

public slots:
    virtual void accept();

private:
    Ui::PicAnalyzeResult *ui;
    PicAnalyzeResultPrivate * const d_ptr;
};

#endif // PICANALYZERESULT_H
