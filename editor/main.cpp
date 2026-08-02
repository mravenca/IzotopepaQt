#include "editormainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    EditorMainWindow window;
    window.show();
    return app.exec();
}
