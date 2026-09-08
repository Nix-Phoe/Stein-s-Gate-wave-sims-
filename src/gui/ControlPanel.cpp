#include "ControlPanel.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

ControlPanel::ControlPanel(QWidget* parent) : QWidget(parent) {
    trajectoryCombo_ = new QComboBox(this);
    trajectoryCombo_->addItem(QStringLiteral("Rectiligne (survol en ligne droite)"));
    trajectoryCombo_->addItem(QStringLiteral("Circulaire (orbite autour du récepteur)"));

    speedSpin_ = new QDoubleSpinBox(this);
    speedSpin_->setRange(0.1, 300.0);
    speedSpin_->setSuffix(QStringLiteral(" m/s"));
    speedSpin_->setValue(20.0); // ~72 km/h : ordre de grandeur d'un drone ou d'une voiture

    radiusSpin_ = new QDoubleSpinBox(this);
    radiusSpin_->setRange(10.0, 5000.0);
    radiusSpin_->setSuffix(QStringLiteral(" m"));
    radiusSpin_->setValue(200.0);
    radiusSpin_->setToolTip(
        QStringLiteral("Rayon de l'orbite — utilisé seulement si la trajectoire est circulaire."));

    altitudeSpin_ = new QDoubleSpinBox(this);
    altitudeSpin_->setRange(1.0, 10000.0);
    altitudeSpin_->setSuffix(QStringLiteral(" m"));
    altitudeSpin_->setValue(50.0);

    carrierFrequencyMhzSpin_ = new QDoubleSpinBox(this);
    carrierFrequencyMhzSpin_->setRange(1.0, 6000.0);
    carrierFrequencyMhzSpin_->setDecimals(1);
    carrierFrequencyMhzSpin_->setSuffix(QStringLiteral(" MHz"));
    carrierFrequencyMhzSpin_->setValue(100.0);
    carrierFrequencyMhzSpin_->setToolTip(QStringLiteral(
        "Fréquence porteuse RÉELLE utilisée pour calculer le Doppler (en Hz) et\n"
        "l'atténuation de Friis. La courbe \"signal reçu\" affiche le décalage\n"
        "Doppler en bande de base (voir README) : à des vitesses/fréquences trop\n"
        "élevées, delta_f dépasse ce qu'un pas de temps d'affichage peut résoudre\n"
        "(repliement de spectre, Nyquist-Shannon) — la courbe devient illisible,\n"
        "ce n'est pas un bug."));

    noiseCheckBox_ = new QCheckBox(QStringLiteral("Bruit gaussien"), this);
    noiseCheckBox_->setChecked(false);

    snrSpin_ = new QDoubleSpinBox(this);
    snrSpin_->setRange(-10.0, 60.0);
    snrSpin_->setSuffix(QStringLiteral(" dB"));
    snrSpin_->setValue(20.0);
    snrSpin_->setEnabled(false); // activé seulement si noiseCheckBox_ est coché

    connect(noiseCheckBox_, &QCheckBox::toggled, snrSpin_, &QDoubleSpinBox::setEnabled);

    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Trajectoire :"), trajectoryCombo_);
    form->addRow(QStringLiteral("Vitesse :"), speedSpin_);
    form->addRow(QStringLiteral("Rayon (si circulaire) :"), radiusSpin_);
    form->addRow(QStringLiteral("Altitude :"), altitudeSpin_);
    form->addRow(QStringLiteral("Fréquence porteuse :"), carrierFrequencyMhzSpin_);

    auto* noiseRow = new QHBoxLayout;
    noiseRow->addWidget(noiseCheckBox_);
    noiseRow->addWidget(snrSpin_);
    form->addRow(QStringLiteral("SNR :"), noiseRow);

    startPauseButton_ = new QPushButton(QStringLiteral("Démarrer"), this);
    resetButton_ = new QPushButton(QStringLiteral("Réinitialiser"), this);
    auto* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(startPauseButton_);
    buttonRow->addWidget(resetButton_);

    auto* group = new QGroupBox(QStringLiteral("Paramètres du scénario"), this);
    auto* groupLayout = new QVBoxLayout(group);
    groupLayout->addLayout(form);
    groupLayout->addLayout(buttonRow);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(group);

    connect(startPauseButton_, &QPushButton::clicked, this, &ControlPanel::startPauseClicked);
    connect(resetButton_, &QPushButton::clicked, this, &ControlPanel::resetClicked);
}

ControlPanel::TrajectoryKind ControlPanel::trajectoryKind() const {
    return trajectoryCombo_->currentIndex() == 0 ? TrajectoryKind::Linear : TrajectoryKind::Circular;
}

double ControlPanel::speedMps() const {
    return speedSpin_->value();
}

double ControlPanel::radiusMeters() const {
    return radiusSpin_->value();
}

double ControlPanel::altitudeMeters() const {
    return altitudeSpin_->value();
}

double ControlPanel::carrierFrequencyHz() const {
    // Le contrôle affiche des MHz (plus lisible pour l'utilisateur qu'un
    // nombre à 8-9 chiffres en Hz) ; le moteur physique travaille en Hz
    // (unité SI de base) -> conversion ici, au point de passage entre
    // l'UI et le moteur.
    return carrierFrequencyMhzSpin_->value() * 1.0e6;
}

bool ControlPanel::noiseEnabled() const {
    return noiseCheckBox_->isChecked();
}

double ControlPanel::snrDb() const {
    return snrSpin_->value();
}

void ControlPanel::setRunning(bool running) {
    startPauseButton_->setText(running ? QStringLiteral("Pause") : QStringLiteral("Démarrer"));
}
