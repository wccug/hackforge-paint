#include <QApplication>
#include "ColorPickerWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ColorPickerWindow picker;
    picker.show();

    return app.exec();
}
