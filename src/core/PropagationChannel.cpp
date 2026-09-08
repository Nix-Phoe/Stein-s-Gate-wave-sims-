#include "PropagationChannel.hpp"
#include "PhysicalConstants.hpp"

#include <cmath>

namespace wavesim {

PropagationResult PropagationChannel::compute(const Transmitter& transmitter,
                                               const Receiver& receiver, double t) const {
    const Vec3 txPosition = transmitter.position(t);
    const Vec3 txVelocity = transmitter.velocity(t);
    const Vec3 rxPosition = receiver.position();

    // --- Distance ---
    // Vecteur émetteur -> récepteur : u = r_rx - r_tx(t)
    // Distance : d(t) = ||u||                                    [m]
    const Vec3 u = rxPosition - txPosition;
    const double distance = u.norm();

    // --- Vitesse radiale ---
    // On veut la composante de la vitesse de l'émetteur le long de l'axe
    // émetteur-récepteur, avec la convention : v_r > 0 = l'émetteur SE
    // RAPPROCHE du récepteur (distance qui diminue).
    //
    // Dérivation : d(t) = ||r_rx - r_tx(t)||. En posant u(t) = r_rx - r_tx(t)
    // (seul r_tx dépend de t), la dérivée de la norme donne :
    //     d(d)/dt = (u . du/dt) / d = (u . (-v_tx)) / d = -(u . v_tx) / d
    // Comme on définit v_r = -d(d)/dt (positif quand la distance diminue,
    // donc en cas de rapprochement), on obtient :
    //     v_r = (u . v_tx) / d                                    [m/s]
    double radialVelocity = 0.0;
    if (distance > 1e-9) {
        radialVelocity = u.dot(txVelocity) / distance;
    }
    // Cas limite d -> 0 : la direction émetteur-récepteur n'est plus définie
    // (émetteur superposé au récepteur). On force v_r = 0 plutôt que de
    // diviser par une distance quasi nulle (explosion numérique).

    // --- Délai de propagation ---
    // tau(t) = d(t) / c                                           [s]
    //
    // Simplification assumée : on utilise la distance à l'instant PRÉSENT t,
    // pas le "temps retardé" exact (le signal reçu à t a en réalité été émis
    // à t - tau, donc calculé avec la position de l'émetteur à t - tau, pas
    // à t). Résoudre l'équation exacte t_emission = t - d(t_emission)/c est
    // une équation implicite (point fixe). Pour les échelles de ce
    // simulateur (émetteur à vitesse très inférieure à c, sur des distances
    // de l'ordre du kilomètre -> tau de l'ordre de la microseconde), la
    // position de l'émetteur ne change quasiment pas pendant tau : l'erreur
    // introduite par cette approximation est négligeable. Voir le README
    // pour la justification chiffrée.
    const double propagationDelay = distance / kSpeedOfLight;

    // --- Décalage Doppler (non-relativiste) ---
    // Formule classique valable pour v << c (voir README pour la
    // justification : la correction relativiste est en (v/c)^2, de l'ordre
    // de 10^-12 pour des vitesses réalistes, donc invisible ici) :
    //     delta_f = f_porteuse * v_r / c                           [Hz]
    //     f_rx    = f_porteuse + delta_f
    const double carrierFrequency = transmitter.carrierFrequencyHz();
    const double dopplerShift = carrierFrequency * radialVelocity / kSpeedOfLight;
    const double receivedFrequency = carrierFrequency + dopplerShift;

    // --- Puissance reçue (Friis, espace libre) ---
    // lambda = c / f_porteuse                                     [m]
    // P_rx = P_tx * (lambda / (4*pi*d))^2                         [W]
    // (gains d'antenne emetteur/recepteur pris = 1, soit 0 dBi, pour rester
    // simple : voir README pour la discussion de cette hypothèse)
    double receivedPowerWatts;
    if (distance > 1e-9) {
        const double wavelength = kSpeedOfLight / carrierFrequency;
        const double attenuationFactor = wavelength / (4.0 * kPi * distance);
        receivedPowerWatts = transmitter.transmitPowerWatts() * attenuationFactor * attenuationFactor;
    } else {
        // d -> 0 : le modèle de Friis (champ lointain) diverge et n'a plus
        // de sens physique (on entrerait en "champ proche"). On plafonne à
        // la puissance émise plutôt que de renvoyer une puissance infinie.
        receivedPowerWatts = transmitter.transmitPowerWatts();
    }

    // Conversion en dBm, pratique pour l'affichage (échelle logarithmique
    // standard en télécom) : P[dBm] = 10 * log10(P[W] * 1000)
    const double receivedPowerDbm = 10.0 * std::log10(receivedPowerWatts * 1000.0);

    return PropagationResult{
        distance,
        radialVelocity,
        dopplerShift,
        receivedFrequency,
        propagationDelay,
        receivedPowerWatts,
        receivedPowerDbm,
    };
}

} // namespace wavesim
