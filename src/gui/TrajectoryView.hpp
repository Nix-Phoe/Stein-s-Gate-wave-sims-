#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>

// ============================================================================
// TrajectoryView — vue 2D (plan horizontal X/Y, vu du dessus) montrant la
// position du récepteur (fixe), la position courante de l'émetteur, et sa
// trace (positions passées).
//
// -- Pourquoi préserver le RAPPORT D'ASPECT (aspect ratio) ? --
// C'est le piège classique d'une vue 2D "maison" : si on mappe X et Y du
// monde vers l'écran avec des échelles DIFFÉRENTES (ex: pour remplir tout
// le widget quel que soit sa forme), un cercle physique apparaît comme une
// ellipse à l'écran. Pour une trajectoire circulaire, ce serait trompeur —
// on doit voir un vrai cercle. La solution : calculer une seule échelle,
// le minimum entre l'échelle X et l'échelle Y nécessaires pour tout faire
// tenir, et l'appliquer aux deux axes. La contrepartie : le dessin ne
// remplit pas forcément tout le widget (bandes vides sur les côtés ou en
// haut/bas) — c'est un compromis correct et assumé.
// ============================================================================
class TrajectoryView : public QWidget {
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget* parent = nullptr);

    // Position du récepteur dans le plan (x,y) du monde, en mètres.
    void setReceiverPosition(const QPointF& worldPosition);

    // Position courante de l'émetteur + trace complète de ses positions
    // passées (dans le même repère que le récepteur).
    void setTransmitterState(const QPointF& currentWorldPosition,
                              const QVector<QPointF>& worldTrace);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize minimumSizeHint() const override;

private:
    QPointF receiverPosition_{0.0, 0.0};
    QPointF currentTransmitterPosition_{0.0, 0.0};
    QVector<QPointF> transmitterTrace_;
    bool hasTransmitterData_ = false;
};
