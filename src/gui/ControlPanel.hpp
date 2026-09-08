#pragma once

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;

// ============================================================================
// ControlPanel — tous les contrôles utilisateur : type de trajectoire,
// vitesse, altitude, rayon (trajectoire circulaire), fréquence porteuse,
// bruit (SNR), et les boutons démarrer/pause/réinitialiser.
//
// -- Pourquoi les changements de paramètres ne s'appliquent-ils qu'au
//    prochain "Réinitialiser", et pas en direct pendant que ça tourne ? --
// Le moteur physique (Transmitter, Trajectory) est immuable une fois
// construit : il n'expose pas de setters pour changer la vitesse ou la
// fréquence d'un Transmitter déjà en cours de simulation. On pourrait
// ajouter cette mutabilité, mais ça complexifierait le moteur (setters,
// validation, cohérence de la trajectoire) pour un bénéfice pédagogique
// nul — comprendre "pourquoi la courbe de Doppler a cette forme" ne
// nécessite pas de pouvoir la modifier à la volée. Donc : on change les
// contrôles, on clique "Réinitialiser", une nouvelle Simulation est
// reconstruite avec les nouvelles valeurs. Simple, explicite, sans état
// caché.
// ============================================================================
class ControlPanel : public QWidget {
    Q_OBJECT

public:
    enum class TrajectoryKind { Linear, Circular };

    explicit ControlPanel(QWidget* parent = nullptr);

    TrajectoryKind trajectoryKind() const;
    double speedMps() const;
    double radiusMeters() const;
    double altitudeMeters() const;
    double carrierFrequencyHz() const; // conversion MHz (affiché) -> Hz (interne)
    bool noiseEnabled() const;
    double snrDb() const;

    // Met à jour le texte du bouton démarrer/pause pour refléter l'état
    // réel de la simulation (c'est MainWindow qui décide de l'état, ce
    // widget ne fait qu'afficher).
    void setRunning(bool running);

signals:
    void startPauseClicked();
    void resetClicked();

private:
    QComboBox* trajectoryCombo_;
    QDoubleSpinBox* speedSpin_;
    QDoubleSpinBox* radiusSpin_;
    QDoubleSpinBox* altitudeSpin_;
    QDoubleSpinBox* carrierFrequencyMhzSpin_;
    QCheckBox* noiseCheckBox_;
    QDoubleSpinBox* snrSpin_;
    QPushButton* startPauseButton_;
    QPushButton* resetButton_;
};
