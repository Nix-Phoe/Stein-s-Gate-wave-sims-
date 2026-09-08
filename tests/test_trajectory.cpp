#include "doctest.h"
#include "core/LinearTrajectory.hpp"
#include "core/CircularTrajectory.hpp"

using namespace wavesim;

// Pourquoi ce test : c'est la définition même du MRU. Si position(t) ou
// velocity(t) est faux ici, le canal de propagation calculera une vitesse
// radiale fausse sur TOUS les scénarios linéaires.
TEST_CASE("LinearTrajectory : position et vitesse sur un cas connu") {
    // Émetteur qui part de (0,0,100) et avance à 50 m/s selon +x.
    LinearTrajectory traj(Vec3(0.0, 0.0, 100.0), Vec3(50.0, 0.0, 0.0));

    // Après 2 secondes : x = 0 + 50*2 = 100 m.
    Vec3 p = traj.position(2.0);
    CHECK(p.x == doctest::Approx(100.0));
    CHECK(p.y == doctest::Approx(0.0));
    CHECK(p.z == doctest::Approx(100.0)); // altitude inchangée

    // La vitesse est constante quel que soit t (propriété du MRU).
    Vec3 v0 = traj.velocity(0.0);
    Vec3 v10 = traj.velocity(10.0);
    CHECK(v0.x == doctest::Approx(50.0));
    CHECK(v10.x == doctest::Approx(50.0));
}

// Pourquoi ce test : vérifie que la vitesse retournée est bien la dérivée
// ANALYTIQUE de la position (et pas une approximation). On le fait en
// vérifiant deux propriétés géométriques connues du cercle :
//   1. La norme de la vitesse est constante et égale à la vitesse tangentielle.
//   2. Au point de départ (theta=0), le mouvement est perpendiculaire au
//      rayon, donc la vitesse est purement selon +y (si le rayon est selon +x).
TEST_CASE("CircularTrajectory : norme de vitesse constante et direction tangente") {
    Vec3 center(0.0, 0.0, 50.0); // altitude 50 m
    double radius = 200.0;
    double tangentialSpeed = 30.0;
    CircularTrajectory traj(center, radius, tangentialSpeed, /*initialAngleRad=*/0.0);

    // À t=0, theta=0 : position attendue = centre + (R, 0, 0).
    Vec3 p0 = traj.position(0.0);
    CHECK(p0.x == doctest::Approx(200.0));
    CHECK(p0.y == doctest::Approx(0.0));
    CHECK(p0.z == doctest::Approx(50.0)); // altitude constante

    // À theta=0, la vitesse doit être tangente au cercle : direction (0, +1, 0)
    // multipliée par la vitesse tangentielle (dérivée de (cos,sin) en theta=0
    // donne (-sin(0), cos(0)) = (0, 1)).
    Vec3 v0 = traj.velocity(0.0);
    CHECK(v0.x == doctest::Approx(0.0));
    CHECK(v0.y == doctest::Approx(30.0));

    // La norme de la vitesse doit rester égale à la vitesse tangentielle à
    // tout instant (mouvement UNIFORME) — on vérifie à un instant quelconque.
    Vec3 vAtT = traj.velocity(1.234);
    CHECK(vAtT.norm() == doctest::Approx(tangentialSpeed));

    // La distance au centre doit aussi rester constante = rayon, à tout instant.
    Vec3 pAtT = traj.position(1.234);
    Vec3 toCenter = pAtT - center;
    CHECK(toCenter.norm() == doctest::Approx(radius));
}

TEST_CASE("CircularTrajectory : cas limite rayon nul ne plante pas (division par zéro évitée)") {
    CircularTrajectory traj(Vec3(1.0, 2.0, 3.0), /*radius=*/0.0, /*tangentialSpeed=*/10.0);
    Vec3 p = traj.position(5.0);
    // Rayon nul : l'émetteur reste immobile au centre.
    CHECK(p.x == doctest::Approx(1.0));
    CHECK(p.y == doctest::Approx(2.0));
    CHECK(p.z == doctest::Approx(3.0));
    Vec3 v = traj.velocity(5.0);
    CHECK(v.norm() == doctest::Approx(0.0));
}
