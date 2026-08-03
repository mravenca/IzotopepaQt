#include "developeroptions.h"
#include "gamewidget.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Izotopepa Complete Edition");
    app.setApplicationVersion("3.0");

    const DeveloperOptions options = parseDeveloperOptions();

    GameWidget widget(options);
    widget.show();
    return app.exec();
}
