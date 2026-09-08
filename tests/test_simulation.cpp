#include "doctest.h"
#include "core/Simulation.hpp"
#include "core/LinearTrajectory.hpp"

#include <memory>

using namespace wavesim;

namespace {

Simulation makeSimpleSimulation(double timeStepSeconds) {
    auto traj = std::make_unique<LinearTrajectory>(Vec3(1000.0, 0.0, 0.0), Vec3(-10.0, 0.0, 0.0));
    Transmitter tx(std::move(traj), /*carrierFrequencyHz=*/100.0, /*transmitPowerWatts=*/1.0);
    Receiver rx(Vec3(0.0, 0.0, 0.0));
    return Simulation(std::move(tx), rx, timeStepSeconds);
}

} // namespace

// Pourquoi ce test : Simulation est la classe qui orchestre la boucle
// temporelle — c'est là qu'une erreur "off-by-one" est facile à introduire
// (avancer le temps avant ou après avoir enregistré l'échantillon). On
// vérifie explicitement que le premier point d'historique correspond à
// t=0 (pas t=dt), et que le temps courant avance bien de dt à chaque appel.
TEST_CASE("Simulation::step() enregistre l'historique à t=0, t=dt, t=2dt, ...") {
    Simulation sim = makeSimpleSimulation(0.1);

    CHECK(sim.currentTimeSeconds() == doctest::Approx(0.0));
    CHECK(sim.history().empty());

    sim.step();
    REQUIRE(sim.history().size() == 1);
    CHECK(sim.history()[0].timeSeconds == doctest::Approx(0.0));
    CHECK(sim.currentTimeSeconds() == doctest::Approx(0.1));

    sim.step();
    REQUIRE(sim.history().size() == 2);
    CHECK(sim.history()[1].timeSeconds == doctest::Approx(0.1));
    CHECK(sim.currentTimeSeconds() == doctest::Approx(0.2));
}

// Pourquoi ce test : reset() doit remettre à zéro TOUT l'état mutable
// (temps, historique, ET la phase interne du SignalGenerator). Si on
// oubliait de réinitialiser la phase du signal, deux exécutions
// consécutives de la même simulation donneraient des signaux différents
// après reset — un bug subtil qui ne casserait ni le temps ni l'historique,
// donc facile à manquer sans un test dédié.
TEST_CASE("Simulation::reset() redonne exactement le même historique qu'un départ frais") {
    Simulation sim = makeSimpleSimulation(0.1);

    sim.step();
    sim.step();
    sim.step();
    double signalBeforeReset = sim.history()[0].receivedSignal;

    sim.reset();
    CHECK(sim.currentTimeSeconds() == doctest::Approx(0.0));
    CHECK(sim.history().empty());

    sim.step();
    REQUIRE(sim.history().size() == 1);
    // Même émetteur, même récepteur, même dt, phase remise à zéro par
    // reset() -> le premier échantillon doit être identique à celui de la
    // toute première exécution.
    CHECK(sim.history()[0].receivedSignal == doctest::Approx(signalBeforeReset));
}

// Pourquoi ce test : vérifie que Simulation transmet correctement les
// informations au canal de propagation (pas de décalage de temps ni
// d'inversion émetteur/récepteur introduits par la boucle d'orchestration).
// On calcule la distance attendue à la main pour le premier point (t=0).
TEST_CASE("Simulation : le premier échantillon d'historique correspond à un calcul manuel à t=0") {
    Simulation sim = makeSimpleSimulation(0.5);
    sim.step();

    const SimulationSample& sample = sim.history()[0];
    // Émetteur en (1000,0,0), récepteur en (0,0,0) -> distance = 1000 m à t=0.
    CHECK(sample.channel.distanceMeters == doctest::Approx(1000.0));
    // Vitesse (-10,0,0) : approche directe -> vitesse radiale = +10 m/s.
    CHECK(sample.channel.radialVelocityMps == doctest::Approx(10.0));
    CHECK(sample.transmitterPosition.x == doctest::Approx(1000.0));
}
