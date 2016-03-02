#ifndef MANGAEDITOR_H
#define MANGAEDITOR_H

#include <QWidget>

namespace Ui {
class MangaEditor;
}

class MangaEditorPrivate;
class MangaEditor : public QWidget {
    Q_OBJECT
    Q_DECLARE_PRIVATE(MangaEditor)

public:
    explicit MangaEditor(QWidget *parent = 0);
    ~MangaEditor();

private:
    Ui::MangaEditor *ui;
    MangaEditorPrivate * const d_ptr;
};

#endif // MANGAEDITOR_H
