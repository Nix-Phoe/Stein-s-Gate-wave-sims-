#pragma once

#include "Trajectory.hpp"

namespace wavesim {

// ============================================================================
// LinearTrajectory — mouvement rectiligne uniforme (MRU).
//
// Équations (r0 = position initiale, v0 = vitesse constante, t = temps en s) :
//
//   r(t) = r0 + v0 * t          [m]
//   v(t) = v0                    [m/s]   (indépendant de t)
//
// C'est le cas le plus simple possible, utile pour valider le canal de
// propagation avec des calculs qu'on peut vérifier à la main (ex: émetteur
// qui approche en ligne droite le récepteur à vitesse constante).
// ============================================================================
class LinearTrajectory : public Trajectory {
public:
    LinearTrajectory(Vec3 initialPosition, Vec3 velocity)
        : initialPosition_(initialPosition), velocity_(velocity) {}

    Vec3 position(double t) const override {
        return initialPosition_ + velocity_ * t;
    }

    Vec3 velocity(double /*t*/) const override {
        return velocity_;
    }

private:
    Vec3 initialPosition_;
    Vec3 velocity_;
};

} // namespace wavesim
