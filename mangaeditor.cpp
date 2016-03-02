#include "mangaeditor.h"
#include "ui_mangaeditor.h"

MangaEditor::MangaEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MangaEditor)
{
    ui->setupUi(this);
    setFocusProxy(ui->txtName);
}

MangaEditor::~MangaEditor()
{
    delete ui;
}
