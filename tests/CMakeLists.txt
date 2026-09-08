# Exécutable de tests unitaires. Ne dépend QUE de wavesim_core (jamais de
# Qt) : c'est ce qui garantit que la physique est testable indépendamment
# de l'interface graphique.
add_executable(wavesim_tests
    test_main.cpp
    test_vec3.cpp
    test_trajectory.cpp
    test_propagation_channel.cpp
    test_signal_generator.cpp
    test_simulation.cpp
)

target_link_libraries(wavesim_tests PRIVATE wavesim_core)

# doctest.h est vendorisé (pas installé sur le système) : on ajoute son
# dossier aux chemins d'inclusion de CET exécutable seulement.
target_include_directories(wavesim_tests PRIVATE ${CMAKE_SOURCE_DIR}/external/doctest)

add_test(NAME wavesim_tests COMMAND wavesim_tests)
