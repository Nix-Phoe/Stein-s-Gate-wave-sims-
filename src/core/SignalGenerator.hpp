#pragma once

#include <optional>
#include <random>

#include "PropagationChannel.hpp"

namespace wavesim {

// ============================================================================
// SignalGenerator — construit, échantillon par échantillon, une
// représentation EN BANDE DE BASE du signal reçu, à partir du résultat du
// canal de propagation.
//
//   s(t) = A(t) * cos(phi(t)) + bruit(t)
//
// où :
//   A(t)   = sqrt(P_rx(t))   [unité d'amplitude arbitraire ; P ∝ A² pour
//                              une onde, donc l'amplitude est la racine de
//                              la puissance reçue calculée par le canal]
//   phi(t) = phase accumulée à partir du DÉCALAGE DOPPLER, pas de la
//            fréquence porteuse absolue (voir "pourquoi la bande de base"
//            ci-dessous).
//
// -- Pourquoi accumuler la phase plutôt que calculer cos(2*pi*f(t)*t) ? --
// La fréquence instantanée change continuellement (effet Doppler). Si on
// substituait directement f(t) dans cos(2*pi*f(t)*t), la phase et
// l'amplitude sauteraient de façon incohérente à chaque instant où f
// change : le "signal" obtenu n'aurait plus de sens physique (il ne serait
// plus continu). La bonne approche est d'intégrer la fréquence instantanée
// dans le temps :
//     phi(t) = phi(t-dt) + 2*pi*f(t)*dt        [rad]
// C'est une intégration d'Euler explicite de la relation fondamentale
// f_instantanee = (1/2*pi) * dphi/dt. C'est exactement ce que fait un
// oscillateur contrôlé en fréquence (VCO) ou un synthétiseur numérique
// (NCO) en électronique — donc PAS une approximation arbitraire, c'est le
// modèle standard d'un signal à fréquence instantanée variable.
// Cette classe garde donc un ÉTAT (la phase accumulée) entre deux appels :
// contrairement à PropagationChannel, sample() n'est pas une fonction pure.
//
// -- Pourquoi la BANDE DE BASE (baseband), et pas la porteuse absolue ? --
// PropagationChannel calcule f_rx = f_porteuse + delta_f avec une porteuse
// RÉALISTE (ex: 2.4 GHz). Or par Nyquist-Shannon, afficher/échantillonner
// une onde à 2.4 GHz demanderait un pas de temps de l'ordre de la
// picoseconde — impossible à l'échelle d'une simulation (millisecondes) ou
// d'un graphique à l'écran. La solution n'est pas d'inventer un hack
// d'affichage : c'est exactement le problème que résout la RÉCEPTION
// RADIO RÉELLE, qui ne numérise jamais la porteuse brute non plus ! Un
// récepteur superhétérodyne (ou un SDR) mélange toujours le signal reçu
// avec un oscillateur local à la fréquence porteuse nominale AVANT
// échantillonnage, pour ne garder que la fréquence intermédiaire — ici,
// le décalage Doppler lui-même :
//     f_bande_de_base(t) = f_rx(t) - f_porteuse = delta_f(t)
// C'est ce que fait cette classe : elle génère phi(t) à partir de
// delta_f(t) = channelResult.dopplerShiftHz, pas de f_rx(t) en entier. Le
// résultat est un signal dont la fréquence (quelques Hz à quelques
// centaines de Hz pour des vitesses/fréquences réalistes) est directement
// observable sur un graphique avec un pas de temps de simulation normal
// (ex: 20 ms). La fréquence porteuse ABSOLUE réelle (2.4 GHz) reste
// utilisée telle quelle par PropagationChannel pour le calcul physique
// (Friis, Doppler en Hz affiché en chiffres) — seule sa représentation
// TEMPORELLE affichée est en bande de base, exactement comme un vrai
// récepteur. C'est expliqué plus en détail dans le README.
// ============================================================================
class SignalGenerator {
public:
    struct Config {
        // Rapport signal/bruit en dB. std::nullopt = bruit désactivé.
        // On utilise optional plutôt qu'un booléen + un double séparés :
        // ça rend l'intention explicite dans le type ("il n'y a peut-être
        // pas de bruit") au lieu d'une convention arbitraire comme snr<0.
        std::optional<double> snrDb = std::nullopt;

        // Graine du générateur aléatoire. Fixée par défaut (pas une graine
        // basée sur l'horloge) pour que les tests soient déterministes et
        // reproductibles.
        unsigned int randomSeed = 42;
    };

    // NOTE : pas de valeur par défaut "= Config{}" ici. C'est tentant, mais
    // ill-formé au sens du standard : Config est une classe IMBRIQUÉE, et
    // ses initialiseurs par défaut de membres (NSDMI, ex: "= std::nullopt")
    // ne sont utilisables qu'une fois la classe ENGLOBANTE (SignalGenerator)
    // complète. Or un argument par défaut de constructeur de SignalGenerator
    // est lui-même évalué avant que SignalGenerator soit complète -> on
    // obtient une dépendance circulaire que GCC/Clang rejettent à la
    // compilation (MSVC l'accepte par tolérance, ce qui masque le problème
    // si on ne teste que sur un seul compilateur). D'où les deux
    // constructeurs séparés ci-dessous plutôt qu'un seul avec défaut.
    explicit SignalGenerator(Config config);

    // Équivalent à SignalGenerator(Config{}) : construit Config{} dans le
    // .cpp, où SignalGenerator (et donc Config) est déjà entièrement défini
    // -> aucun problème d'ordre de complétude.
    SignalGenerator();

    // Remet la phase accumulée à zéro. À appeler quand on redémarre/réinitialise
    // la simulation, sinon la phase continuerait à s'accumuler depuis un état
    // périmé.
    void reset();

    // Calcule un échantillon du signal reçu EN BANDE DE BASE (voir
    // commentaire de classe), en avançant la phase interne de dt secondes.
    // channelResult doit provenir de PropagationChannel::compute au même
    // instant t.
    double sample(const PropagationResult& channelResult, double dt);

    bool noiseEnabled() const { return config_.snrDb.has_value(); }

private:
    Config config_;
    double accumulatedPhaseRad_ = 0.0;
    std::mt19937 randomEngine_;
    std::normal_distribution<double> standardNormal_{0.0, 1.0};
};

} // namespace wavesim
