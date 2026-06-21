#include <DISPatch/MainWindow.h>

#include <QtWidgets/QApplication>

auto main(int argc, char *argv[]) -> int
{
    QApplication app(argc, argv);

    dispatch::MainWindow window;
    window.show();

    return QApplication::exec();
}
