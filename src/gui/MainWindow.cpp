#include "MainWindow.hpp"

#include "ControlPanel.hpp"
#include "TrajectoryView.hpp"
#include "PlotWidget.hpp"

#include "core/CircularTrajectory.hpp"
#include "core/LinearTrajectory.hpp"
#include "core/PhysicalConstants.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>

#include <algorithm>
#include <memory>

namespace {
// Puissance d'émission fixe : pas exposée comme contrôle utilisateur (la
// consigne du projet liste vitesse / fréquence / altitude / trajectoire /
// SNR — pas de puissance d'émission), donc on choisit une valeur physique
// raisonnable (1 W, un ordre de grandeur réaliste pour un petit émetteur
// radio) et on la garde constante.
constexpr double kTransmitPowerWatts = 1.0;

// Position fixe du récepteur (à l'origine du repère) : le sujet précise
// "le récepteur est fixe au sol" sans demander de contrôle pour sa
// position -> on la fixe une bonne fois pour toutes à l'origine, ce qui
// simplifie aussi la lecture des graphiques (toutes les distances sont
// mesurées depuis (0,0,0)).
const wavesim::Vec3 kReceiverPosition(0.0, 0.0, 0.0);
} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Steins Gate Wave Sim — simulateur Doppler radio"));

    controlPanel_ = new ControlPanel(this);
    trajectoryView_ = new TrajectoryView(this);

    dopplerPlot_ = new PlotWidget(this);
    dopplerPlot_->setTitle(QStringLiteral("Décalage Doppler"));
    dopplerPlot_->setAxisLabels(QStringLiteral("t (s)"), QStringLiteral("Δf (Hz)"));
    dopplerPlot_->setShowZeroLine(true); // le signe du Doppler est l'info clé (approche/éloignement)

    powerPlot_ = new PlotWidget(this);
    powerPlot_->setTitle(QStringLiteral("Puissance reçue"));
    powerPlot_->setAxisLabels(QStringLiteral("t (s)"), QStringLiteral("P (dBm)"));

    signalPlot_ = new PlotWidget(this);
    signalPlot_->setTitle(QStringLiteral("Signal reçu (bande de base)"));
    signalPlot_->setAxisLabels(QStringLiteral("t (s)"), QStringLiteral("amplitude"));
    signalPlot_->setShowZeroLine(true);

    // --- Disposition : colonne de gauche = vue 2D + contrôles, colonne de
    // droite = les 3 graphiques empilés ---
    auto* leftColumn = new QVBoxLayout;
    leftColumn->addWidget(trajectoryView_, /*stretch=*/1);
    leftColumn->addWidget(controlPanel_, /*stretch=*/0);

    auto* rightColumn = new QVBoxLayout;
    rightColumn->addWidget(dopplerPlot_, 1);
    rightColumn->addWidget(powerPlot_, 1);
    rightColumn->addWidget(signalPlot_, 1);

    auto* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftColumn, 1);
    mainLayout->addLayout(rightColumn, 2);

    auto* central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
    resize(1100, 700);

    timer_ = new QTimer(this);
    timer_->setInterval(kTimerIntervalMs);
    connect(timer_, &QTimer::timeout, this, &MainWindow::onTimerTick);

    connect(controlPanel_, &ControlPanel::startPauseClicked, this, &MainWindow::onStartPauseClicked);
    connect(controlPanel_, &ControlPanel::resetClicked, this, &MainWindow::onResetClicked);

    rebuildSimulation();
}

void MainWindow::rebuildSimulation() {
    timer_->stop();
    running_ = false;
    controlPanel_->setRunning(false);

    // --- Construction de la trajectoire selon le choix de l'utilisateur ---
    const double altitude = controlPanel_->altitudeMeters();
    const double speed = controlPanel_->speedMps();
    std::unique_ptr<wavesim::Trajectory> trajectory;

    if (controlPanel_->trajectoryKind() == ControlPanel::TrajectoryKind::Linear) {
        // Émetteur qui approche en ligne droite selon +x, à altitude
        // constante, en passant au plus près du récepteur à (0,0,altitude)
        // -> distance minimale = altitude, exactement au-dessus du
        // récepteur. Point de départ fixe à 300 m avant le survol : plus
        // la vitesse est élevée, plus le survol est rapide (cohérent
        // physiquement), sans avoir besoin d'un contrôle supplémentaire
        // pour la distance de départ.
        trajectory = std::make_unique<wavesim::LinearTrajectory>(
            wavesim::Vec3(-300.0, 0.0, altitude), wavesim::Vec3(speed, 0.0, 0.0));
    } else {
        const double radius = controlPanel_->radiusMeters();
        // Centre décalé de +radius sur l'axe x : avec un angle initial de
        // pi radians, position(0) = centre + radius*(cos(pi), sin(pi), 0)
        // = (radius - radius, 0, altitude) = (0, 0, altitude), c'est-à-dire
        // exactement au-dessus du récepteur au démarrage (survol immédiat,
        // puis l'émetteur s'éloigne avant de revenir un tour plus tard).
        trajectory = std::make_unique<wavesim::CircularTrajectory>(
            wavesim::Vec3(radius, 0.0, altitude), radius, speed, wavesim::kPi);
    }

    wavesim::Transmitter transmitter(std::move(trajectory), controlPanel_->carrierFrequencyHz(),
                                      kTransmitPowerWatts);
    wavesim::Receiver receiver(kReceiverPosition);

    wavesim::SignalGenerator::Config signalConfig;
    if (controlPanel_->noiseEnabled()) {
        signalConfig.snrDb = controlPanel_->snrDb();
    }
    // signalConfig.snrDb reste à std::nullopt (valeur par défaut) sinon ->
    // pas de bruit, cohérent avec la case à cocher décochée par défaut.

    const double timeStepSeconds = kTimerIntervalMs / 1000.0;
    simulation_ = std::make_unique<wavesim::Simulation>(std::move(transmitter), receiver,
                                                          timeStepSeconds, signalConfig);

    trajectoryView_->setReceiverPosition(QPointF(kReceiverPosition.x, kReceiverPosition.y));

    // Un premier pas immédiat pour que l'utilisateur voie l'état initial
    // (t=0) avant même d'appuyer sur "Démarrer".
    simulation_->step();
    refreshViews();
}

void MainWindow::onStartPauseClicked() {
    running_ = !running_;
    controlPanel_->setRunning(running_);
    if (running_) {
        timer_->start();
    } else {
        timer_->stop();
    }
}

void MainWindow::onResetClicked() {
    rebuildSimulation();
}

void MainWindow::onTimerTick() {
    simulation_->step();
    refreshViews();
}

void MainWindow::refreshViews() {
    const std::vector<wavesim::SimulationSample>& history = simulation_->history();
    if (history.empty()) {
        return;
    }

    // --- Vue 2D : trace complète (coût négligeable, voir TrajectoryView) ---
    QVector<QPointF> trace;
    trace.reserve(static_cast<int>(history.size()));
    for (const wavesim::SimulationSample& sample : history) {
        trace.append(QPointF(sample.transmitterPosition.x, sample.transmitterPosition.y));
    }
    const wavesim::SimulationSample& latest = history.back();
    trajectoryView_->setTransmitterState(
        QPointF(latest.transmitterPosition.x, latest.transmitterPosition.y), trace);

    // --- Graphiques temporels : fenêtre glissante des kDisplayWindowSamples
    // derniers points (voir le commentaire de kDisplayWindowSamples dans le
    // header pour le "pourquoi") ---
    const int startIndex =
        std::max(0, static_cast<int>(history.size()) - kDisplayWindowSamples);

    QVector<QPointF> dopplerPoints;
    QVector<QPointF> powerPoints;
    QVector<QPointF> signalPoints;
    dopplerPoints.reserve(static_cast<int>(history.size()) - startIndex);
    powerPoints.reserve(static_cast<int>(history.size()) - startIndex);
    signalPoints.reserve(static_cast<int>(history.size()) - startIndex);

    for (int i = startIndex; i < static_cast<int>(history.size()); ++i) {
        const wavesim::SimulationSample& sample = history[static_cast<size_t>(i)];
        dopplerPoints.append(QPointF(sample.timeSeconds, sample.channel.dopplerShiftHz));
        powerPoints.append(QPointF(sample.timeSeconds, sample.channel.receivedPowerDbm));
        signalPoints.append(QPointF(sample.timeSeconds, sample.receivedSignal));
    }

    dopplerPlot_->setData(dopplerPoints);
    powerPlot_->setData(powerPoints);
    signalPlot_->setData(signalPoints);
}
