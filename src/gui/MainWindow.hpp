#pragma once

#include <QMainWindow>
#include <memory>

#include "core/Simulation.hpp"

class QTimer;
class ControlPanel;
class TrajectoryView;
class PlotWidget;

// ============================================================================
// MainWindow — assemble tous les widgets et fait le pont entre l'UI Qt et
// le moteur physique (wavesim::Simulation).
//
// C'est la SEULE classe de gui/ qui inclut un header de core/. Toutes les
// autres classes GUI (PlotWidget, TrajectoryView, ControlPanel) sont
// agnostiques du moteur physique — MainWindow fait la traduction entre
// les deux mondes (ex: extraire un SimulationSample::channel.dopplerShiftHz
// et le transformer en QPointF pour PlotWidget). C'est le point de
// couplage volontairement unique et centralisé entre "physique" et "Qt".
// ============================================================================
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStartPauseClicked();
    void onResetClicked();
    void onTimerTick();

private:
    // (Re)construit la Simulation à partir des valeurs actuelles du
    // ControlPanel. Appelée au démarrage de l'application et à chaque clic
    // sur "Réinitialiser".
    void rebuildSimulation();

    // Relit l'historique de la simulation et met à jour les 4 vues
    // (trajectoire 2D, Doppler, puissance, signal).
    void refreshViews();

    ControlPanel* controlPanel_;
    TrajectoryView* trajectoryView_;
    PlotWidget* dopplerPlot_;
    PlotWidget* powerPlot_;
    PlotWidget* signalPlot_;

    QTimer* timer_;
    std::unique_ptr<wavesim::Simulation> simulation_;
    bool running_ = false;

    // Pas de temps de la simulation = intervalle du timer UI. Un seul et
    // même nombre pilote la physique ET le rafraîchissement graphique : ça
    // garde un modèle mental simple ("1 tick de timer = 1 pas de
    // simulation"), au prix de ne pas pouvoir accélérer la simulation par
    // rapport au temps réel. Voir le README pour la discussion complète
    // (compromis avec le théorème de Nyquist-Shannon pour la courbe de
    // signal en bande de base).
    static constexpr int kTimerIntervalMs = 20;

    // Nombre de points affichés (fenêtre glissante) sur les graphiques
    // temporels. Le moteur physique garde TOUT l'historique (Simulation
    // n'a pas de notion d'affichage), mais afficher des dizaines de
    // milliers de points sur un graphique de quelques centaines de pixels
    // de large ne sert à rien et ralentit le rendu : c'est une décision
    // d'AFFICHAGE, prise ici dans la couche GUI, pas dans le moteur.
    static constexpr int kDisplayWindowSamples = 400;
};
