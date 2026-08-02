#include "gamewidget.h"
#include <QApplication>
int main(int argc,char**argv){QApplication app(argc,argv);app.setApplicationName("Izotopepa Complete Edition");GameWidget w;w.show();return app.exec();}
