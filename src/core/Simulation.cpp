#include "Simulation.hpp"

namespace wavesim {

Simulation::Simulation(Transmitter transmitter, Receiver receiver, double timeStepSeconds,
                        SignalGenerator::Config signalConfig)
    : transmitter_(std::move(transmitter)),
      receiver_(receiver),
      timeStepSeconds_(timeStepSeconds),
      signalGenerator_(signalConfig) {}

void Simulation::step() {
    // 1. Calcule l'état du canal à l'instant courant (avant d'avancer le temps).
    PropagationResult result = channel_.compute(transmitter_, receiver_, currentTimeSeconds_);

    // 2. Calcule l'échantillon de signal correspondant. On avance la phase
    // interne du générateur de timeStepSeconds_ (voir SignalGenerator pour
    // le "pourquoi" de l'intégration de phase).
    double signalSample = signalGenerator_.sample(result, timeStepSeconds_);

    // 3. Enregistre le point d'historique complet.
    history_.push_back(SimulationSample{
        currentTimeSeconds_,
        transmitter_.position(currentTimeSeconds_),
        result,
        signalSample,
    });

    // 4. Avance le temps pour le prochain appel à step().
    currentTimeSeconds_ += timeStepSeconds_;
}

void Simulation::reset() {
    currentTimeSeconds_ = 0.0;
    history_.clear();
    signalGenerator_.reset();
}

} // namespace wavesim
