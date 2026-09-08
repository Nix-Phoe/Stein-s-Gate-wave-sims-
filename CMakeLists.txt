cmake_minimum_required(VERSION 3.16)
project(steins_gate_wave_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Le moteur physique (jamais de dépendance Qt).
add_subdirectory(src/core)

option(WAVESIM_BUILD_TESTS "Compiler les tests unitaires (doctest)" ON)
if(WAVESIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- Interface graphique Qt ---
# CMAKE_AUTOMOC : génère automatiquement le code moc (nécessaire pour tout
# QObject utilisant Q_OBJECT/signaux/slots) sans avoir à l'invoquer à la
# main. C'est LE mécanisme CMake standard pour Qt.
set(CMAKE_AUTOMOC ON)

option(WAVESIM_BUILD_GUI "Compiler l'interface graphique Qt" ON)
if(WAVESIM_BUILD_GUI)
    # On cherche Qt6 d'abord (version actuelle), puis Qt5 en repli, pour
    # rester utilisable sur une installation Qt5 existante sans dupliquer
    # tout le CMakeLists. QUIET : si aucun n'est trouvé, on désactive
    # simplement la cible GUI plutôt que de faire échouer toute la
    # configuration (le moteur physique + les tests restent utilisables
    # sans Qt).
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if(Qt6_FOUND)
        set(WAVESIM_QT_VERSION 6)
    else()
        find_package(Qt5 QUIET COMPONENTS Widgets)
        if(Qt5_FOUND)
            set(WAVESIM_QT_VERSION 5)
        endif()
    endif()

    if(Qt6_FOUND OR Qt5_FOUND)
        add_subdirectory(src/gui)
    else()
        message(WARNING "Qt introuvable : la cible wavesim_gui ne sera pas construite. "
                         "Le moteur physique (wavesim_core) et les tests restent disponibles.")
    endif()
endif()
