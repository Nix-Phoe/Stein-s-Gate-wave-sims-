#pragma once

#include <cmath>

#include "Trajectory.hpp"

namespace wavesim {

// ============================================================================
// CircularTrajectory — mouvement circulaire uniforme dans un plan horizontal
// (ex: un drone qui survole le récepteur en tournant à altitude constante).
//
// Paramètres :
//   centre            : centre du cercle, en mètres (sa composante z fixe
//                        l'altitude du vol, constante pendant toute la
//                        simulation).
//   rayon R           : rayon du cercle, en mètres.
//   vitesseTangentielle : vitesse le long du cercle, en m/s (constante,
//                        d'où "mouvement circulaire UNIFORME").
//   angleInitial theta0 : phase de départ, en radians.
//
// Équations :
//
//   omega = vitesseTangentielle / R                    [rad/s]
//   theta(t) = theta0 + omega * t                        [rad]
//
//   r(t) = centre + R * (cos(theta(t)), sin(theta(t)), 0)     [m]
//
//   v(t) = dr/dt = R * omega * (-sin(theta(t)), cos(theta(t)), 0)   [m/s]
//
// v(t) est la dérivée ANALYTIQUE de r(t) (pas une différence finie) : on
// dérive theta(t) = theta0 + omega*t, donc d(cos theta)/dt = -sin(theta)*omega
// et d(sin theta)/dt = cos(theta)*omega, d'où le facteur R*omega commun.
//
// Cas limite R = 0 : le "cercle" dégénère en un point fixe. omega serait
// indéfini (division par R=0) alors qu'il n'a plus de sens physique — on le
// force à 0 dans ce cas (l'émetteur reste immobile au centre).
// ============================================================================
class CircularTrajectory : public Trajectory {
public:
    CircularTrajectory(Vec3 center, double radius, double tangentialSpeed,
                        double initialAngleRad = 0.0)
        : center_(center),
          radius_(radius),
          // Vitesse angulaire omega = v / R. Voir cas limite R=0 en commentaire
          // de classe : on protège contre la division par zéro.
          omega_(radius > 1e-9 ? tangentialSpeed / radius : 0.0),
          theta0_(initialAngleRad) {}

    Vec3 position(double t) const override {
        const double theta = theta0_ + omega_ * t;
        return center_ + Vec3(radius_ * std::cos(theta), radius_ * std::sin(theta), 0.0);
    }

    Vec3 velocity(double t) const override {
        const double theta = theta0_ + omega_ * t;
        const double speedFactor = radius_ * omega_; // = vitesseTangentielle, sauf cas R=0
        return Vec3(-speedFactor * std::sin(theta), speedFactor * std::cos(theta), 0.0);
    }

private:
    Vec3 center_;
    double radius_;
    double omega_;
    double theta0_;
};

} // namespace wavesim
