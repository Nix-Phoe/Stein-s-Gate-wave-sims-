#include <QApplication>

#include "MainWindow.hpp"

// Point d'entrée de l'application graphique. Volontairement minimal : toute
// la logique vit dans MainWindow (et, en dessous, dans wavesim::Simulation,
// qui ne connaît rien de Qt).
int main(int argc, char** argv) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
