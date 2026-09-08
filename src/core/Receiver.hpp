#pragma once

#include "Vec3.hpp"

namespace wavesim {

// ============================================================================
// Receiver — le récepteur, fixe au sol. Une simple position 3D en mètres.
//
// Pas de trajectoire ici (contrairement à Transmitter) : c'est un choix de
// simplification assumé par l'énoncé du projet ("un récepteur est fixe au
// sol"). Si on voulait un jour un récepteur mobile, il suffirait de lui
// donner la même composition qu'un Transmitter (une Trajectory) — la classe
// est volontairement minimale pour matcher exactement le besoin actuel.
// ============================================================================
class Receiver {
public:
    explicit Receiver(Vec3 position) : position_(position) {}

    Vec3 position() const { return position_; }

private:
    Vec3 position_;
};

} // namespace wavesim
