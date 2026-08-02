#pragma once
#include "levelcanvas.h"
#include "leveldocument.h"
#include <QMainWindow>

class QLabel;
class QScrollArea;

class EditorMainWindow : public QMainWindow
{
public:
    explicit EditorMainWindow(QWidget *parent = nullptr);

private:
    void openLevel();
    void reloadLevel();
    void openDefaultLevel();
    void updateTitle();
    void setZoom(double zoom);

    LevelDocument document_;
    LevelCanvas *canvas_ = nullptr;
    QScrollArea *scrollArea_ = nullptr;
    QLabel *statusLabel_ = nullptr;
};
