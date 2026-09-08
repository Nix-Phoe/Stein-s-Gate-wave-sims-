# wavesim_core : le moteur physique, indépendant de Qt.
#
# Bibliothèque STATIC (et non plus INTERFACE) depuis qu'on a une vraie
# logique compilée (PropagationChannel.cpp). Les classes encore header-only
# (Vec3, Trajectory...) sont listées ici uniquement pour qu'elles apparaissent
# dans les IDE (CMake ne les compile pas séparément, elles sont incluses là
# où elles sont utilisées).
add_library(wavesim_core STATIC
    Vec3.hpp
    PhysicalConstants.hpp
    Trajectory.hpp
    LinearTrajectory.hpp
    CircularTrajectory.hpp
    Transmitter.hpp
    Receiver.hpp
    PropagationChannel.hpp
    PropagationChannel.cpp
    SignalGenerator.hpp
    SignalGenerator.cpp
    Simulation.hpp
    Simulation.cpp
)

# Les fichiers incluent via "core/Vec3.hpp" -> le dossier racine à exposer
# est src/, pas src/core/.
target_include_directories(wavesim_core PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_compile_features(wavesim_core PUBLIC cxx_std_17)
