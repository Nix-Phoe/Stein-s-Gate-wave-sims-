#include "doctest.h"
#include "core/PropagationChannel.hpp"
#include "core/LinearTrajectory.hpp"
#include "core/PhysicalConstants.hpp"

#include <memory>

using namespace wavesim;

namespace {

// Petite fabrique pour éviter de répéter la construction d'un Transmitter
// avec trajectoire linéaire dans chaque test.
Transmitter makeLinearTransmitter(Vec3 initialPosition, Vec3 velocity,
                                   double carrierFrequencyHz = 2.4e9,
                                   double transmitPowerWatts = 1.0) {
    auto traj = std::make_unique<LinearTrajectory>(initialPosition, velocity);
    return Transmitter(std::move(traj), carrierFrequencyHz, transmitPowerWatts);
}

} // namespace

// Pourquoi ce test : vérifie le calcul de distance sur un cas qu'on peut
// poser à la main (triangle 3-4-5 mis à l'échelle x100). Un émetteur
// immobile permet aussi de vérifier que vitesse radiale = 0 et Doppler = 0
// quand rien ne bouge — le cas de base dont tout le reste est une variation.
TEST_CASE("PropagationChannel : distance sur un cas connu, émetteur immobile") {
    Receiver rx(Vec3(0.0, 0.0, 0.0));
    Transmitter tx = makeLinearTransmitter(Vec3(300.0, 400.0, 0.0), Vec3(0.0, 0.0, 0.0));

    PropagationChannel channel;
    PropagationResult result = channel.compute(tx, rx, /*t=*/0.0);

    CHECK(result.distanceMeters == doctest::Approx(500.0));
    CHECK(result.radialVelocityMps == doctest::Approx(0.0));
    CHECK(result.dopplerShiftHz == doctest::Approx(0.0));
}

// Pourquoi ce test : c'est LE test de la convention de signe. En approche
// directe (l'émetteur vole droit vers le récepteur), la vitesse radiale doit
// être égale à la vitesse totale (projection complète, pas partielle), et le
// Doppler doit être POSITIF (fréquence perçue plus haute qu'émise) — c'est
// l'intuition physique de base (klaxon d'ambulance qui approche = son aigu).
TEST_CASE("PropagationChannel : approche directe -> vitesse radiale positive et Doppler positif") {
    Receiver rx(Vec3(0.0, 0.0, 0.0));
    // Émetteur à 1000 m sur l'axe x, qui se dirige droit vers l'origine à 50 m/s.
    Transmitter tx = makeLinearTransmitter(Vec3(1000.0, 0.0, 0.0), Vec3(-50.0, 0.0, 0.0));

    PropagationChannel channel;
    PropagationResult result = channel.compute(tx, rx, /*t=*/0.0);

    // Approche directe -> toute la vitesse est radiale.
    CHECK(result.radialVelocityMps == doctest::Approx(50.0));
    CHECK(result.dopplerShiftHz > 0.0);
    // Vérification numérique de la formule : delta_f = f0 * v_r / c.
    const double expectedShift = 2.4e9 * 50.0 / kSpeedOfLight;
    CHECK(result.dopplerShiftHz == doctest::Approx(expectedShift));
}

// Pourquoi ce test : symétrique du précédent, pour l'éloignement. Si on
// n'avait que le test d'approche, un bug de signe constant (toujours
// positif, par exemple à cause d'un fabs() oublié) passerait inaperçu.
TEST_CASE("PropagationChannel : éloignement direct -> vitesse radiale négative et Doppler négatif") {
    Receiver rx(Vec3(0.0, 0.0, 0.0));
    // Même émetteur, mais qui s'éloigne (vitesse vers +x, en partant de +x).
    Transmitter tx = makeLinearTransmitter(Vec3(1000.0, 0.0, 0.0), Vec3(50.0, 0.0, 0.0));

    PropagationChannel channel;
    PropagationResult result = channel.compute(tx, rx, /*t=*/0.0);

    CHECK(result.radialVelocityMps == doctest::Approx(-50.0));
    CHECK(result.dopplerShiftHz < 0.0);
}

// Pourquoi ce test : c'est le cas limite au coeur de la "courbe en S" du
// Doppler (survol) : quand l'émetteur se déplace perpendiculairement à
// l'axe émetteur-récepteur (au point de passage au plus près), la vitesse
// radiale doit être EXACTEMENT nulle, donc le Doppler aussi. Ce point est
// l'instant où la fréquence perçue traverse la porteuse dans la courbe en S.
TEST_CASE("PropagationChannel : vitesse perpendiculaire à l'axe -> vitesse radiale nulle") {
    Receiver rx(Vec3(0.0, 0.0, 0.0));
    // Émetteur situé sur l'axe y (donc décalé de l'axe de son propre
    // déplacement, qui est selon x) : au carré de son passage au plus près.
    Transmitter tx = makeLinearTransmitter(Vec3(0.0, 500.0, 100.0), Vec3(50.0, 0.0, 0.0));

    PropagationChannel channel;
    PropagationResult result = channel.compute(tx, rx, /*t=*/0.0);

    CHECK(result.radialVelocityMps == doctest::Approx(0.0).epsilon(1e-9));
    CHECK(result.dopplerShiftHz == doctest::Approx(0.0).epsilon(1e-6));
}

// Pourquoi ce test : Friis prédit une décroissance en 1/d² de la puissance.
// On vérifie cette proportion directement (P(2d) = P(d)/4) plutôt qu'une
// valeur absolue, pour isoler la LOI de décroissance de la valeur exacte de
// la constante multiplicative (qui dépend elle de lambda, testée à part).
TEST_CASE("PropagationChannel : la puissance reçue décroît en 1/distance^2") {
    Receiver rx(Vec3(0.0, 0.0, 0.0));
    Transmitter txNear = makeLinearTransmitter(Vec3(1000.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0));
    Transmitter txFar = makeLinearTransmitter(Vec3(2000.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0));

    PropagationChannel channel;
    PropagationResult resultNear = channel.compute(txNear, rx, 0.0);
    PropagationResult resultFar = channel.compute(txFar, rx, 0.0);

    // Distance doublée -> puissance divisée par 4 (2^2).
    CHECK(resultFar.receivedPowerWatts == doctest::Approx(resultNear.receivedPowerWatts / 4.0));
}

// Pourquoi ce test : le délai de propagation est une simple division par c,
// mais c'est aussi l'endroit le plus facile de se tromper d'unité (m vs km,
// s vs microsecondes). On vérifie sur deux distances très différentes.
TEST_CASE("PropagationChannel : cohérence du délai de propagation tau = d / c") {
    Receiver rx(Vec3(0.0, 0.0, 0.0));

    Transmitter txClose = makeLinearTransmitter(Vec3(300.0, 0.0, 0.0), Vec3(0.0, 0.0, 0.0));
    Transmitter txFar = makeLinearTransmitter(Vec3(3.0e5, 0.0, 0.0), Vec3(0.0, 0.0, 0.0)); // 300 km

    PropagationChannel channel;
    PropagationResult resultClose = channel.compute(txClose, rx, 0.0);
    PropagationResult resultFar = channel.compute(txFar, rx, 0.0);

    // 300 m / c ~= 1.0006 microseconde
    CHECK(resultClose.propagationDelaySeconds == doctest::Approx(300.0 / kSpeedOfLight));
    // 300 km / c = 1 ms (valeur repère facile à vérifier de tête : c'est
    // d'ailleurs à peu près le délai Terre-satellite géostationnaire / 100).
    CHECK(resultFar.propagationDelaySeconds == doctest::Approx(1.0e-3).epsilon(1e-3));
}
