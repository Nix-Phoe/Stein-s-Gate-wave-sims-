#pragma once

#include "Receiver.hpp"
#include "Transmitter.hpp"

namespace wavesim {

// ============================================================================
// PropagationResult — tout ce que le canal calcule pour un instant t donné.
// Simple agrégat de données (pas de logique) : c'est ce que PropagationChannel
// produit et ce que SignalGenerator et l'UI consomment.
// ============================================================================
struct PropagationResult {
    double distanceMeters = 0.0;           // d(t)          [m]
    double radialVelocityMps = 0.0;        // v_r(t)        [m/s], positif = rapprochement
    double dopplerShiftHz = 0.0;           // delta_f(t)    [Hz], signé
    double receivedFrequencyHz = 0.0;      // f_rx(t)       [Hz] = f_porteuse + delta_f
    double propagationDelaySeconds = 0.0;  // tau(t)        [s]
    double receivedPowerWatts = 0.0;       // P_rx(t)       [W]
    double receivedPowerDbm = 0.0;         // P_rx(t)       [dBm], pratique pour l'affichage
};

// ============================================================================
// PropagationChannel — calcule l'effet du canal de propagation radio entre
// un émetteur mobile et un récepteur fixe, à un instant t donné.
//
// C'est une classe SANS ÉTAT (aucune donnée membre) : "compute" est une
// fonction pure de (émetteur, récepteur, t). On la garde comme classe plutôt
// que fonction libre pour respecter l'architecture demandée et parce que
// c'est le point d'extension naturel si on veut ajouter un jour un modèle
// de canal plus riche (ex: paramètre d'atténuation atmosphérique, gains
// d'antenne non-unitaires) sans changer la signature appelante.
// ============================================================================
class PropagationChannel {
public:
    PropagationResult compute(const Transmitter& transmitter, const Receiver& receiver,
                               double t) const;
};

} // namespace wavesim
