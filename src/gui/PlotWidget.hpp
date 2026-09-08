#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QString>

// ============================================================================
// PlotWidget — widget de courbe X/Y générique, dessiné à la main avec
// QPainter.
//
// -- Pourquoi QPainter et pas QtCharts ? --
// QtCharts est un module Qt séparé (pas garanti présent avec toute
// installation de Qt de base), ce qui aurait ajouté une dépendance externe
// à justifier sans l'avoir écrite soi-même. Avec QPainter, on sait
// EXACTEMENT comment chaque pixel est placé (mise à l'échelle des axes,
// tracé de la courbe) — ce qui est le but pédagogique de tout ce projet.
// La contrepartie assumée : pas de zoom/tooltip interactif comme
// QtCharts offrirait — acceptable ici.
//
// -- Pourquoi une classe GÉNÉRIQUE (pas "DopplerPlotWidget" etc.) ? --
// Les trois graphiques demandés (Doppler, puissance, signal temporel) sont
// tous des courbes "valeur en fonction du temps" : même logique de tracé,
// seuls le titre, les labels et les données changent. Une seule classe
// réutilisée trois fois évite de dupliquer tout le code de mise à
// l'échelle des axes.
//
// -- Pourquoi ce widget ne connaît RIEN du moteur physique (wavesim) ? --
// Il prend des QVector<QPointF> génériques, pas des SimulationSample. C'est
// MainWindow qui fait la conversion (extraire le champ voulu de l'historique
// de la simulation). Ça garde PlotWidget réutilisable pour n'importe quelle
// courbe, et respecte la même séparation moteur/affichage que core/ vs gui/
// (ici appliquée à l'intérieur même de l'UI).
// ============================================================================
class PlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlotWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setAxisLabels(const QString& xLabel, const QString& yLabel);

    // Si true, trace une ligne horizontale pointillée à y=0 — utile pour
    // les courbes qui changent de signe (Doppler, signal), pour repérer
    // visuellement le passage par zéro.
    void setShowZeroLine(bool show);

    // Remplace entièrement les données affichées. Les points doivent être
    // triés par x croissant (ce qui est garanti par un historique de
    // simulation, rempli dans l'ordre du temps).
    void setData(const QVector<QPointF>& points);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize minimumSizeHint() const override;

private:
    QString title_;
    QString xLabel_;
    QString yLabel_;
    QVector<QPointF> points_;
    bool showZeroLine_ = false;

    // Rectangle utilisable pour tracer la courbe (widget moins les marges
    // réservées au titre et aux labels d'axes).
    QRectF plotArea() const;
};
