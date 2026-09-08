#pragma once

#include <memory>
#include "Trajectory.hpp"

namespace wavesim {

// ============================================================================
// Transmitter — l'émetteur mobile. Regroupe :
//   - sa trajectoire (polymorphe : rectiligne, circulaire, ou toute future
//     implémentation de Trajectory),
//   - sa fréquence porteuse [Hz],
//   - sa puissance d'émission [W].
//
// La trajectoire est possédée via std::unique_ptr<Trajectory> : le
// Transmitter est responsable de sa durée de vie (propriété exclusive,
// pas de copie implicite d'un objet polymorphe — ce qui causerait du
// "slicing"). C'est pour ça que Transmitter n'est pas copiable par défaut,
// seulement déplaçable (comportement hérité de unique_ptr).
// ============================================================================
class Transmitter {
public:
    Transmitter(std::unique_ptr<Trajectory> trajectory, double carrierFrequencyHz,
                double transmitPowerWatts)
        : trajectory_(std::move(trajectory)),
          carrierFrequencyHz_(carrierFrequencyHz),
          transmitPowerWatts_(transmitPowerWatts) {}

    Vec3 position(double t) const { return trajectory_->position(t); }
    Vec3 velocity(double t) const { return trajectory_->velocity(t); }

    double carrierFrequencyHz() const { return carrierFrequencyHz_; }
    double transmitPowerWatts() const { return transmitPowerWatts_; }

private:
    std::unique_ptr<Trajectory> trajectory_;
    double carrierFrequencyHz_;
    double transmitPowerWatts_;
};

} // namespace wavesim
