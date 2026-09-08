#include "TrajectoryView.hpp"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace {
constexpr int kMargin = 30; // marge en pixels autour de la zone de dessin
constexpr double kMarkerRadius = 6.0;
} // namespace

TrajectoryView::TrajectoryView(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 200);
}

void TrajectoryView::setReceiverPosition(const QPointF& worldPosition) {
    receiverPosition_ = worldPosition;
    update();
}

void TrajectoryView::setTransmitterState(const QPointF& currentWorldPosition,
                                          const QVector<QPointF>& worldTrace) {
    currentTransmitterPosition_ = currentWorldPosition;
    transmitterTrace_ = worldTrace;
    hasTransmitterData_ = true;
    update();
}

QSize TrajectoryView::minimumSizeHint() const {
    return QSize(250, 250);
}

void TrajectoryView::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().base());

    // --- Calcul de la boîte englobante en coordonnées MONDE (mètres) ---
    // On inclut toujours le récepteur, pour qu'il reste visible même si la
    // trace de l'émetteur en est très éloignée.
    double worldMinX = receiverPosition_.x();
    double worldMaxX = receiverPosition_.x();
    double worldMinY = receiverPosition_.y();
    double worldMaxY = receiverPosition_.y();

    auto includePoint = [&](const QPointF& p) {
        worldMinX = std::min(worldMinX, p.x());
        worldMaxX = std::max(worldMaxX, p.x());
        worldMinY = std::min(worldMinY, p.y());
        worldMaxY = std::max(worldMaxY, p.y());
    };
    if (hasTransmitterData_) {
        includePoint(currentTransmitterPosition_);
        for (const QPointF& p : transmitterTrace_) {
            includePoint(p);
        }
    }

    // Évite une boîte englobante de largeur/hauteur nulle (ex: tout au
    // même endroit au tout début de la simulation).
    if (worldMaxX - worldMinX < 1.0) {
        worldMinX -= 1.0;
        worldMaxX += 1.0;
    }
    if (worldMaxY - worldMinY < 1.0) {
        worldMinY -= 1.0;
        worldMaxY += 1.0;
    }
    // Marge de 10% autour des points, pour ne pas coller les marqueurs au bord.
    const double paddingX = (worldMaxX - worldMinX) * 0.1;
    const double paddingY = (worldMaxY - worldMinY) * 0.1;
    worldMinX -= paddingX;
    worldMaxX += paddingX;
    worldMinY -= paddingY;
    worldMaxY += paddingY;

    const QRectF pixelArea(kMargin, kMargin, width() - 2.0 * kMargin, height() - 2.0 * kMargin);

    // --- Échelle UNIQUE pour X et Y (voir commentaire de classe : pourquoi
    // préserver le rapport d'aspect) ---
    const double scaleX = pixelArea.width() / (worldMaxX - worldMinX);
    const double scaleY = pixelArea.height() / (worldMaxY - worldMinY);
    const double scale = std::min(scaleX, scaleY);

    // Centre du monde affiché, pour centrer le dessin dans pixelArea même
    // quand scale != scaleX ou != scaleY (bandes vides sur un des axes).
    const double worldCenterX = (worldMinX + worldMaxX) / 2.0;
    const double worldCenterY = (worldMinY + worldMaxY) / 2.0;
    const QPointF pixelCenter = pixelArea.center();

    // Conversion monde -> écran. L'axe Y écran grandit vers le bas alors que
    // l'axe Y "monde" (nord/altitude latérale) grandit vers le haut par
    // convention mathématique -> on inverse le signe du terme en Y.
    auto toPixel = [&](const QPointF& worldPoint) -> QPointF {
        const double px = pixelCenter.x() + (worldPoint.x() - worldCenterX) * scale;
        const double py = pixelCenter.y() - (worldPoint.y() - worldCenterY) * scale;
        return QPointF(px, py);
    };

    // --- Trace de l'émetteur (positions passées) ---
    if (hasTransmitterData_ && transmitterTrace_.size() >= 2) {
        QPainterPath tracePath;
        tracePath.moveTo(toPixel(transmitterTrace_.first()));
        for (int i = 1; i < transmitterTrace_.size(); ++i) {
            tracePath.lineTo(toPixel(transmitterTrace_[i]));
        }
        QPen tracePen(palette().mid().color(), 1.5, Qt::DashLine);
        painter.setPen(tracePen);
        painter.drawPath(tracePath);
    }

    // --- Récepteur : marqueur carré fixe, avec étiquette ---
    const QPointF rxPixel = toPixel(receiverPosition_);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(200, 60, 60));
    painter.drawRect(QRectF(rxPixel.x() - kMarkerRadius, rxPixel.y() - kMarkerRadius,
                             2 * kMarkerRadius, 2 * kMarkerRadius));
    painter.setPen(palette().text().color());
    painter.drawText(rxPixel + QPointF(kMarkerRadius + 4, 4), QStringLiteral("Rx"));

    // --- Émetteur : marqueur rond à sa position courante, avec étiquette ---
    if (hasTransmitterData_) {
        const QPointF txPixel = toPixel(currentTransmitterPosition_);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(60, 120, 200));
        painter.drawEllipse(txPixel, kMarkerRadius, kMarkerRadius);
        painter.setPen(palette().text().color());
        painter.drawText(txPixel + QPointF(kMarkerRadius + 4, 4), QStringLiteral("Tx"));
    }
}
