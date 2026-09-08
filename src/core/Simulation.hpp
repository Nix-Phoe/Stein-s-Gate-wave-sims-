#pragma once

#include <vector>

#include "PropagationChannel.hpp"
#include "Receiver.hpp"
#include "SignalGenerator.hpp"
#include "Transmitter.hpp"

namespace wavesim {

// ============================================================================
// SimulationSample — un point d'historique complet : tout ce qu'on veut
// pouvoir tracer sur un graphique pour un instant t donné.
// ============================================================================
struct SimulationSample {
    double timeSeconds = 0.0;
    Vec3 transmitterPosition;      // pour la vue 2D et la trace de trajectoire
    PropagationResult channel;     // distance, vitesse radiale, Doppler, délai, puissance
    double receivedSignal = 0.0;   // échantillon du signal temporel s(t)
};

// ============================================================================
// Simulation — orchestre le moteur physique : avance le temps par pas
// fixe, appelle PropagationChannel puis SignalGenerator à chaque pas, et
// conserve l'historique complet pour que l'UI puisse tracer les courbes.
//
// C'est la seule classe qui possède un ÉTAT DE SIMULATION à proprement
// parler (temps courant, historique). Le Transmitter et le Receiver lui
// sont fournis (elle ne les construit pas elle-même) : c'est de
// l'injection de dépendances simple, qui permet de reconfigurer le
// scénario (nouvelle trajectoire, nouvelle fréquence...) sans toucher à
// la classe Simulation elle-même.
// ============================================================================
class Simulation {
public:
    Simulation(Transmitter transmitter, Receiver receiver, double timeStepSeconds,
               SignalGenerator::Config signalConfig = SignalGenerator::Config{});

    // Avance la simulation d'UN pas de temps (timeStepSeconds). Calcule le
    // nouveau PropagationResult et le nouvel échantillon de signal, les
    // ajoute à l'historique. C'est la méthode appelée en boucle par l'UI
    // (ex: sur un timer Qt) ou par les tests.
    void step();

    // Remet la simulation à zéro : temps -> 0, historique vidé, phase du
    // signal réinitialisée. Ne change PAS l'émetteur/récepteur configurés.
    void reset();

    double currentTimeSeconds() const { return currentTimeSeconds_; }
    double timeStepSeconds() const { return timeStepSeconds_; }
    const std::vector<SimulationSample>& history() const { return history_; }

    const Transmitter& transmitter() const { return transmitter_; }
    const Receiver& receiver() const { return receiver_; }

private:
    Transmitter transmitter_;
    Receiver receiver_;
    double timeStepSeconds_;

    PropagationChannel channel_;      // sans état, réutilisable tel quel
    SignalGenerator signalGenerator_; // avec état (phase) -> membre, pas local

    double currentTimeSeconds_ = 0.0;
    std::vector<SimulationSample> history_;
};

} // namespace wavesim
