# wavesim_gui : l'exécutable graphique Qt Widgets.
#
# CMAKE_AUTOMOC ON (voir CMakeLists.txt racine) génère automatiquement le
# code moc nécessaire pour les classes qui héritent de QObject et utilisent
# Q_OBJECT (signaux/slots) : ControlPanel, PlotWidget, TrajectoryView,
# MainWindow.
add_executable(wavesim_gui
    main.cpp
    MainWindow.hpp
    MainWindow.cpp
    ControlPanel.hpp
    ControlPanel.cpp
    TrajectoryView.hpp
    TrajectoryView.cpp
    PlotWidget.hpp
    PlotWidget.cpp
)

# Lien avec le moteur physique (wavesim_core) ET avec Qt Widgets. C'est le
# SEUL exécutable du projet qui dépend de Qt — wavesim_core et wavesim_tests
# n'en ont jamais besoin.
target_link_libraries(wavesim_gui PRIVATE wavesim_core Qt::Widgets)
