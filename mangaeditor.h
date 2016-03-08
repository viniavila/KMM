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
    MangaEditor(QWidget *parent = 0);
    MangaEditor(const QString& filename, QWidget *parent = 0);
    ~MangaEditor();

    void saveProject(const QString& filename);

    void setProjectFile(const QString&);
    QString projectFile() const;

    void setTabModified(bool);
    bool tabModified() const;

    QString tempPath() const;

    bool analyzeChapterPictures();

private:
    Ui::MangaEditor *ui;
    MangaEditorPrivate * const d_ptr;
};

#endif // MANGAEDITOR_H
