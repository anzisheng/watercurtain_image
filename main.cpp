#include <QApplication>
#include "WaterCurtainWidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    WaterCurtainWidget widget;
    widget.setWindowTitle("Water Curtain - BMP Controlled Fountain");
    widget.resize(1024, 768);

    // ≥¢ ‘º”‘ÿBMPÕº∆¨
    widget.loadCurtainTexture("water_curtain.bmp");

    widget.show();

    return app.exec();
}