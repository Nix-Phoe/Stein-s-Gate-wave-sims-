#pragma once

#include "Vec3.hpp"

namespace wavesim {

// ============================================================================
// Trajectory — interface abstraite représentant le mouvement de l'émetteur
// au cours du temps.
//
// On expose deux méthodes séparées, position(t) et velocity(t), plutôt
// qu'une seule, parce que :
//   1. Le canal de propagation a besoin des DEUX indépendamment (distance
//      à partir de la position, vitesse radiale à partir de la vitesse).
//   2. Pour chaque trajectoire, la vitesse est la dérivée ANALYTIQUE de la
//      position (calculée à la main), pas une différence finie
//      (position(t+dt) - position(t)) / dt. Une dérivée numérique
//      introduirait une erreur d'approximation et dépendrait du pas de
//      temps choisi — ce qu'on veut éviter pour un calcul physique exact.
//
// C'est une classe abstraite pure (aucune donnée, aucune implémentation) :
// le polymorphisme classique en C++, une vtable par sous-classe. Alternative
// moderne possible : std::variant + std::visit (pas de vtable, dispatch à la
// compilation). On garde l'héritage ici car c'est le patron le plus lisible
// pour "ajouter un nouveau type de trajectoire" sans toucher au code existant
// (principe ouvert/fermé) — pertinent à mentionner en entrevue.
// ============================================================================
class Trajectory {
public:
    virtual ~Trajectory() = default;

    // Position de l'émetteur à l'instant t (secondes depuis le début de la
    // simulation). Retour en mètres.
    virtual Vec3 position(double t) const = 0;

    // Vitesse instantanée de l'émetteur à l'instant t, en m/s.
    virtual Vec3 velocity(double t) const = 0;
};

} // namespace wavesim
