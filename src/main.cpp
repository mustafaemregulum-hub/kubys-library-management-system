#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication uygulama(argc, argv);

    QApplication::setApplicationName("KUBYS");
    QApplication::setOrganizationName("KUBYS");

    MainWindow pencere;
    pencere.show();

    return uygulama.exec();
}