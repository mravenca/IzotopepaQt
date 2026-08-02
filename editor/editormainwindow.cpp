#include "editormainwindow.h"
#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>

EditorMainWindow::EditorMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1200, 760);

    canvas_ = new LevelCanvas(this);

    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidget(canvas_);
    scrollArea_->setWidgetResizable(false);
    setCentralWidget(scrollArea_);

    QAction *open = new QAction("&Open...", this);
    open->setShortcut(QKeySequence::Open);
    connect(open, &QAction::triggered, this,
            [this] { openLevel(); });

    QAction *reload = new QAction("&Reload", this);
    reload->setShortcut(QKeySequence::Refresh);
    connect(reload, &QAction::triggered, this,
            [this] { reloadLevel(); });

    QAction *zoomIn = new QAction("Zoom &in", this);
    zoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(zoomIn, &QAction::triggered, this,
            [this] { setZoom(canvas_->zoom() * 1.25); });

    QAction *zoomOut = new QAction("Zoom &out", this);
    zoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOut, &QAction::triggered, this,
            [this] { setZoom(canvas_->zoom() / 1.25); });

    QAction *actual = new QAction("&Actual size", this);
    actual->setShortcut(QKeySequence("Ctrl+0"));
    connect(actual, &QAction::triggered, this,
            [this] { setZoom(1.0); });

    QMenu *file = menuBar()->addMenu("&File");
    file->addAction(open);
    file->addAction(reload);

    QMenu *view = menuBar()->addMenu("&View");
    view->addAction(zoomIn);
    view->addAction(zoomOut);
    view->addAction(actual);

    QToolBar *toolbar = addToolBar("Main");
    toolbar->addAction(open);
    toolbar->addAction(reload);
    toolbar->addSeparator();
    toolbar->addAction(zoomIn);
    toolbar->addAction(zoomOut);
    toolbar->addAction(actual);

    statusLabel_ = new QLabel(this);
    statusBar()->addPermanentWidget(statusLabel_);

    openDefaultLevel();
}

void EditorMainWindow::openLevel()
{
    const QString start =
        document_.fileName().isEmpty()
            ? QDir::current().filePath("assets/levels")
            : QFileInfo(document_.fileName()).absolutePath();

    const QString fileName = QFileDialog::getOpenFileName(
        this, "Open level", start,
        "Izotopepa JSON levels (*.json);;All files (*)");

    if (fileName.isEmpty())
        return;

    QString error;
    if (!document_.load(fileName, &error)) {
        QMessageBox::critical(this, "Cannot open level", error);
        return;
    }

    canvas_->setDocument(&document_);
    canvas_->adjustSize();
    updateTitle();
}

void EditorMainWindow::reloadLevel()
{
    if (document_.fileName().isEmpty()) {
        openLevel();
        return;
    }

    QString error;
    if (!document_.load(document_.fileName(), &error)) {
        QMessageBox::critical(this, "Cannot reload level", error);
        return;
    }

    canvas_->setDocument(&document_);
    canvas_->adjustSize();
    updateTitle();
}

void EditorMainWindow::openDefaultLevel()
{
    const QString fileName =
        QDir::current().filePath("assets/levels/level1.json");

    if (QFileInfo::exists(fileName)) {
        QString error;
        document_.load(fileName, &error);
        canvas_->setDocument(&document_);
        canvas_->adjustSize();
    }

    updateTitle();
}

void EditorMainWindow::setZoom(double zoom)
{
    canvas_->setZoom(zoom);
    canvas_->adjustSize();
    updateTitle();
}

void EditorMainWindow::updateTitle()
{
    if (document_.fileName().isEmpty()) {
        setWindowTitle("Izotopepa Level Editor — no level open");
        statusLabel_->setText("Ctrl+O: open a JSON level");
        return;
    }

    setWindowTitle(
        QString("Izotopepa Level Editor — %1")
            .arg(document_.name()));

    statusLabel_->setText(
        QString("%1 × %2   Zoom %3%")
            .arg(document_.worldSize().width())
            .arg(document_.worldSize().height())
            .arg(int(canvas_->zoom() * 100)));
}
