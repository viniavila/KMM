#ifndef MANGAEDITOR_H
#define MANGAEDITOR_H

#include <QWidget>

namespace Ui {
class MangaEditor;
}

class MangaEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MangaEditor(QWidget *parent = 0);
    ~MangaEditor();

private:
    Ui::MangaEditor *ui;
};

#endif // MANGAEDITOR_H
