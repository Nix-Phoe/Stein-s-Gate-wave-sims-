#include "doctest.h"
#include "core/Vec3.hpp"

using namespace wavesim;

// Pourquoi ce test : la distance et la vitesse radiale reposent entièrement
// sur dot() et norm(). Si ces deux opérations de base sont fausses, TOUT le
// reste du simulateur (Doppler, puissance, délai) sera faux silencieusement.
// On vérifie donc dot/norm sur un cas qu'on peut calculer à la main.
TEST_CASE("Vec3 : norme et produit scalaire sur le triangle 3-4-5") {
    // Triangle rectangle classique : un vecteur (3,4,0) a une norme de 5.
    Vec3 v(3.0, 4.0, 0.0);
    CHECK(v.norm() == doctest::Approx(5.0));

    // Produit scalaire de v avec lui-même = norme au carré = 25.
    CHECK(v.dot(v) == doctest::Approx(25.0));

    // Deux vecteurs orthogonaux ont un produit scalaire nul.
    Vec3 a(1.0, 0.0, 0.0);
    Vec3 b(0.0, 1.0, 0.0);
    CHECK(a.dot(b) == doctest::Approx(0.0));
}

TEST_CASE("Vec3 : addition, soustraction, multiplication par scalaire") {
    Vec3 a(1.0, 2.0, 3.0);
    Vec3 b(4.0, 5.0, 6.0);

    Vec3 sum = a + b;
    CHECK(sum.x == doctest::Approx(5.0));
    CHECK(sum.y == doctest::Approx(7.0));
    CHECK(sum.z == doctest::Approx(9.0));

    Vec3 diff = b - a;
    CHECK(diff.x == doctest::Approx(3.0));
    CHECK(diff.y == doctest::Approx(3.0));
    CHECK(diff.z == doctest::Approx(3.0));

    Vec3 scaled = a * 2.0;
    CHECK(scaled.x == doctest::Approx(2.0));
    CHECK(scaled.y == doctest::Approx(4.0));
    CHECK(scaled.z == doctest::Approx(6.0));
}
