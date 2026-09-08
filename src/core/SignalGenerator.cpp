#include "SignalGenerator.hpp"
#include "PhysicalConstants.hpp"

#include <cmath>

namespace wavesim {

SignalGenerator::SignalGenerator(Config config)
    : config_(config), randomEngine_(config.randomSeed) {}

// Délégation vers le constructeur ci-dessus : à cet endroit du fichier,
// SignalGenerator (et son type imbriqué Config) est déjà complètement
// défini, donc "Config{}" ne pose aucun problème (voir le commentaire du
// header sur les initialiseurs par défaut de membres d'une classe imbriquée).
SignalGenerator::SignalGenerator() : SignalGenerator(Config{}) {}

void SignalGenerator::reset() {
    accumulatedPhaseRad_ = 0.0;
}

double SignalGenerator::sample(const PropagationResult& channelResult, double dt) {
    // --- Accumulation de la phase, EN BANDE DE BASE ---
    // On intègre le décalage Doppler (delta_f), pas la fréquence porteuse
    // absolue f_rx : voir le commentaire de classe ("pourquoi la bande de
    // base") pour la justification complète (Nyquist + comportement d'un
    // vrai récepteur superhétérodyne).
    // phi(t) = phi(t-dt) + 2*pi*delta_f(t)*dt
    accumulatedPhaseRad_ += 2.0 * kPi * channelResult.dopplerShiftHz * dt;

    // --- Amplitude ---
    // P ∝ A²  =>  A = sqrt(P)   (unité d'amplitude arbitraire, cohérente
    // avec l'unité de puissance choisie pour P_rx, ici des Watts).
    const double amplitude = std::sqrt(channelResult.receivedPowerWatts);

    double signalValue = amplitude * std::cos(accumulatedPhaseRad_);

    // --- Bruit gaussien optionnel, paramétré par le SNR (rapport signal/bruit) ---
    if (config_.snrDb.has_value()) {
        // SNR_lineaire = 10^(SNR_dB / 10) = P_signal / P_bruit
        const double snrLinear = std::pow(10.0, config_.snrDb.value() / 10.0);
        // => P_bruit = P_signal / SNR_lineaire
        const double noisePower = channelResult.receivedPowerWatts / snrLinear;
        // Écart-type d'un bruit blanc gaussien centré de puissance P_bruit :
        // sigma = sqrt(P_bruit) (la "puissance" d'un bruit centré est sa variance).
        const double noiseStdDev = std::sqrt(noisePower);
        signalValue += noiseStdDev * standardNormal_(randomEngine_);
    }

    return signalValue;
}

} // namespace wavesim
