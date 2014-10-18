#include "igem_mainwindow.h"
#include <QApplication>
#include <QString>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  igem_MainWindow w;
  w.show();

  //make sure to execute closing() before the program is shut down
  QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(closing()));

  return a.exec();
}
