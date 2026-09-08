#pragma once

#include <cmath>

namespace wavesim {

// ============================================================================
// Vec3 — vecteur 3D minimal.
//
// Toutes les positions et vitesses de la simulation sont exprimées dans un
// repère cartésien 3D en unités SI : mètres (m) pour les positions,
// mètres/seconde (m/s) pour les vitesses.
//
// Ce n'est volontairement PAS une bibliothèque d'algèbre linéaire générique
// (pas de matrices, pas de quaternions) : on n'a besoin que des opérations
// de base pour calculer une distance et une projection de vitesse. Ajouter
// plus serait de la sur-ingénierie pour ce projet.
// ============================================================================
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vec3() = default;
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    // --- Opérations de base ---

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    // Multiplication par un scalaire (ex: position + vitesse * temps).
    Vec3 operator*(double scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    // Produit scalaire : u·v = |u||v|cos(theta).
    // Utilisé pour projeter la vitesse de l'émetteur sur l'axe émetteur-récepteur
    // (voir PropagationChannel::compute pour la vitesse radiale).
    double dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Norme euclidienne : ||v|| = sqrt(v·v).
    // Utilisée pour la distance émetteur-récepteur : d = ||r_rx - r_tx||.
    double norm() const {
        return std::sqrt(dot(*this));
    }
};

} // namespace wavesim
