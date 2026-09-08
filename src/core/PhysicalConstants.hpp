#pragma once

namespace wavesim {

// Vitesse de la lumière dans le vide [m/s]. Valeur EXACTE (définition du
// système international depuis 1983 : le mètre est défini à partir de
// cette constante), donc pas une valeur mesurée/arrondie.
inline constexpr double kSpeedOfLight = 299792458.0;

// Pi en double précision.
//
// Pourquoi ne pas utiliser M_PI de <cmath> ? Parce que M_PI est une
// extension POSIX, pas une partie du standard C++ — sous MSVC, il n'existe
// que si on définit _USE_MATH_DEFINES *avant* d'inclure <cmath>. C'est un
// piège de portabilité classique quand on compile le même code avec
// MinGW/GCC (où M_PI marche "par accident") et MSVC (où il ne marche pas
// par défaut). Définir notre propre constante l'évite complètement.
inline constexpr double kPi = 3.14159265358979323846;

} // namespace wavesim
