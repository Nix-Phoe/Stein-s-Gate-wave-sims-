#include "doctest.h"
#include "core/SignalGenerator.hpp"
#include "core/PhysicalConstants.hpp"

#include <cmath>
#include <vector>

using namespace wavesim;

namespace {

// Petit résultat de canal "fabriqué à la main" pour tester SignalGenerator
// isolément, sans passer par PropagationChannel (déjà testé séparément).
// NOTE : SignalGenerator travaille en BANDE DE BASE (voir SignalGenerator.hpp),
// donc c'est le champ dopplerShiftHz qui pilote la fréquence du signal généré
// ici, pas receivedFrequencyHz (la porteuse absolue, non utilisée pour
// l'affichage temporel).
PropagationResult makeChannelResult(double dopplerShiftHz, double receivedPowerWatts) {
    PropagationResult r;
    r.dopplerShiftHz = dopplerShiftHz;
    r.receivedPowerWatts = receivedPowerWatts;
    return r;
}

} // namespace

// Pourquoi ce test : le signal généré est A*cos(phi) + bruit. Sans bruit,
// |cos(phi)| <= 1 donc |s(t)| ne doit jamais dépasser l'amplitude A. Si ce
// test échoue, ça signale presque certainement une erreur d'unité dans le
// calcul de phase (ex: oubli du facteur 2*pi).
TEST_CASE("SignalGenerator : sans bruit, l'amplitude du signal ne dépasse jamais A = sqrt(P)") {
    PropagationResult r = makeChannelResult(/*deltaF=*/100.0, /*P=*/4.0); // A = sqrt(4) = 2
    SignalGenerator generator; // pas de config = pas de bruit

    const double dt = 0.0001;
    for (int i = 0; i < 1000; ++i) {
        double s = generator.sample(r, dt);
        CHECK(std::abs(s) <= 2.0 + 1e-9);
    }
}

// Pourquoi ce test : vérifie que la phase est bien ACCUMULÉE (intégrée dans
// le temps) et non recalculée à partir de zéro à chaque appel. Après un
// nombre de pas correspondant exactement à une période complète du signal,
// la phase doit être un multiple de 2*pi, donc le signal doit revenir à son
// amplitude maximale (cos = 1).
TEST_CASE("SignalGenerator : la phase accumulée redonne le signal maximal après une période complète") {
    const double deltaFHz = 100.0; // période = 10 ms
    const double dt = 0.001;          // 1 ms par pas -> 10 pas pour 1 période
    PropagationResult r = makeChannelResult(deltaFHz, /*P=*/4.0); // A = 2

    SignalGenerator generator;
    double lastSample = 0.0;
    for (int i = 0; i < 10; ++i) {
        lastSample = generator.sample(r, dt);
    }
    // Après exactement 10 pas de 1 ms à 100 Hz : phase = 2*pi*100*0.001*10 = 2*pi.
    CHECK(lastSample == doctest::Approx(2.0).epsilon(1e-6));
}

// Pourquoi ce test : reset() doit remettre la phase à zéro. On accumule
// d'abord une phase qui N'EST PAS un multiple de 2*pi (pour ne pas tomber
// sur un cas ambigu où cos() vaudrait 1 par coïncidence), puis on vérifie
// qu'un appel juste après reset() avec dt=0 (donc sans avancer la phase)
// redonne bien le signal au maximum (phase=0 -> cos(0)=1).
TEST_CASE("SignalGenerator : reset() remet la phase accumulée à zéro") {
    PropagationResult r = makeChannelResult(/*deltaF=*/100.0, /*P=*/4.0); // A = 2
    SignalGenerator generator;

    // 3 pas de 1 ms à 100 Hz : phase = 2*pi*100*0.001*3 = 0.6*pi (pas un
    // multiple de 2*pi, donc cos(0.6*pi) != 1).
    for (int i = 0; i < 3; ++i) {
        generator.sample(r, 0.001);
    }

    generator.reset();
    // dt = 0 : la phase n'avance pas, elle doit être exactement 0 après reset.
    double s = generator.sample(r, 0.0);
    CHECK(s == doctest::Approx(2.0));
}

// Pourquoi ce test : vérifie que le bruit ajouté a bien l'écart-type prédit
// par le SNR configuré. On utilise une décalage Doppler de 0 Hz pour que la
// partie "signal propre" reste constante (= amplitude) à chaque échantillon :
// ça permet d'isoler le bruit par soustraction et de mesurer son écart-type
// empirique, sans dépendre de l'algorithme exact utilisé par
// std::normal_distribution (qui peut varier d'une bibliothèque standard à
// l'autre) — on teste une PROPRIÉTÉ STATISTIQUE, pas des valeurs exactes.
TEST_CASE("SignalGenerator : l'écart-type du bruit correspond au SNR configuré") {
    const double amplitude = 1.0; // P = 1 W -> A = 1
    PropagationResult r = makeChannelResult(/*deltaF=*/0.0, /*P=*/1.0);

    const double snrDb = 10.0; // SNR linéaire = 10
    SignalGenerator::Config config;
    config.snrDb = snrDb;
    config.randomSeed = 1234;
    SignalGenerator generator(config);

    const int sampleCount = 20000;
    double sumSquaredNoise = 0.0;
    for (int i = 0; i < sampleCount; ++i) {
        double s = generator.sample(r, 0.0); // deltaF=0 -> partie propre toujours = amplitude
        double noise = s - amplitude;
        sumSquaredNoise += noise * noise;
    }
    double empiricalVariance = sumSquaredNoise / sampleCount;
    double empiricalStdDev = std::sqrt(empiricalVariance);

    // Théorie : SNR_lin = P_signal / P_bruit = 10 -> P_bruit = 1/10 = 0.1
    // -> sigma attendu = sqrt(0.1) ~= 0.3162
    const double expectedStdDev = std::sqrt(1.0 / std::pow(10.0, snrDb / 10.0));

    // Tolérance de 10% : test statistique sur 20000 échantillons, pas une
    // égalité exacte (la variance empirique fluctue naturellement).
    CHECK(empiricalStdDev == doctest::Approx(expectedStdDev).epsilon(0.1));
}
