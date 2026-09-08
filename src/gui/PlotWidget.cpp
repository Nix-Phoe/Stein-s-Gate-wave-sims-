#include "PlotWidget.hpp"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace {
// Marges réservées autour de la zone de tracé, en pixels, pour le titre et
// les labels des axes.
constexpr int kMarginLeft = 60;
constexpr int kMarginRight = 20;
constexpr int kMarginTop = 30;
constexpr int kMarginBottom = 40;
constexpr int kTickCount = 5; // nombre de graduations par axe
} // namespace

PlotWidget::PlotWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(150);
}

void PlotWidget::setTitle(const QString& title) {
    title_ = title;
    update(); // planifie un repaint (Qt appellera paintEvent plus tard)
}

void PlotWidget::setAxisLabels(const QString& xLabel, const QString& yLabel) {
    xLabel_ = xLabel;
    yLabel_ = yLabel;
    update();
}

void PlotWidget::setShowZeroLine(bool show) {
    showZeroLine_ = show;
    update();
}

void PlotWidget::setData(const QVector<QPointF>& points) {
    points_ = points;
    update();
}

QSize PlotWidget::minimumSizeHint() const {
    return QSize(300, 150);
}

QRectF PlotWidget::plotArea() const {
    return QRectF(kMarginLeft, kMarginTop, width() - kMarginLeft - kMarginRight,
                  height() - kMarginTop - kMarginBottom);
}

void PlotWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fond du widget (couleur de la fenêtre, pour rester cohérent avec le
    // thème clair/sombre du système plutôt que de forcer du blanc).
    painter.fillRect(rect(), palette().base());

    const QRectF area = plotArea();

    // Titre, centré en haut.
    painter.setPen(palette().text().color());
    if (!title_.isEmpty()) {
        painter.drawText(QRectF(0, 4, width(), kMarginTop - 4), Qt::AlignHCenter, title_);
    }

    if (points_.isEmpty()) {
        painter.drawText(area, Qt::AlignCenter, QStringLiteral("(pas de données)"));
        return;
    }

    // --- Détermination des bornes des axes à partir des données ---
    double xMin = points_.first().x();
    double xMax = xMin;
    double yMin = points_.first().y();
    double yMax = yMin;
    for (const QPointF& p : points_) {
        xMin = std::min(xMin, p.x());
        xMax = std::max(xMax, p.x());
        yMin = std::min(yMin, p.y());
        yMax = std::max(yMax, p.y());
    }

    // Si toutes les valeurs sont identiques (ex: un seul point, ou un
    // signal constant), xMax-xMin ou yMax-yMin vaudrait 0 -> on élargirait
    // par zéro dans la mise à l'échelle. On force alors un intervalle
    // arbitraire autour de la valeur pour éviter la division par zéro.
    if (xMax - xMin < 1e-12) {
        xMax = xMin + 1.0;
    }
    if (yMax - yMin < 1e-12) {
        yMin -= 1.0;
        yMax += 1.0;
    }
    // Si la ligne du zéro doit être visible (ex: Doppler qui change de
    // signe), on s'assure que 0 fait partie de la plage affichée même si
    // toutes les données sont, par exemple, strictement positives.
    if (showZeroLine_) {
        yMin = std::min(yMin, 0.0);
        yMax = std::max(yMax, 0.0);
    }
    // Petite marge de 5% en haut/bas pour que la courbe ne touche pas
    // exactement les bords du cadre.
    const double yPadding = (yMax - yMin) * 0.05;
    yMin -= yPadding;
    yMax += yPadding;

    // Conversion (x,y) en unités "données" -> position en pixels dans area.
    // L'axe Y écran grandit vers le bas, donc on inverse (area.bottom() -
    // ... ) pour que les valeurs élevées apparaissent en haut du graphique.
    auto toPixel = [&](const QPointF& p) -> QPointF {
        const double px = area.left() + (p.x() - xMin) / (xMax - xMin) * area.width();
        const double py = area.bottom() - (p.y() - yMin) / (yMax - yMin) * area.height();
        return QPointF(px, py);
    };

    // --- Cadre et graduations ---
    QPen axisPen(palette().mid().color());
    painter.setPen(axisPen);
    painter.drawRect(area);

    painter.setPen(palette().text().color());
    for (int i = 0; i <= kTickCount; ++i) {
        const double fraction = static_cast<double>(i) / kTickCount;

        // Graduation verticale (axe X) : valeur de temps sous le cadre.
        const double xValue = xMin + fraction * (xMax - xMin);
        const double xPixel = area.left() + fraction * area.width();
        painter.drawLine(QPointF(xPixel, area.bottom()), QPointF(xPixel, area.bottom() + 4));
        painter.drawText(QRectF(xPixel - 30, area.bottom() + 6, 60, 16), Qt::AlignHCenter,
                          QString::number(xValue, 'g', 3));

        // Graduation horizontale (axe Y) : valeur à gauche du cadre.
        const double yValue = yMax - fraction * (yMax - yMin); // en haut = valeur max
        const double yPixel = area.top() + fraction * area.height();
        painter.drawLine(QPointF(area.left() - 4, yPixel), QPointF(area.left(), yPixel));
        painter.drawText(QRectF(0, yPixel - 8, kMarginLeft - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                          QString::number(yValue, 'g', 3));
    }

    // Labels d'axes.
    if (!xLabel_.isEmpty()) {
        painter.drawText(QRectF(area.left(), height() - 18, area.width(), 16), Qt::AlignHCenter,
                          xLabel_);
    }
    if (!yLabel_.isEmpty()) {
        painter.save();
        painter.translate(12, area.center().y());
        painter.rotate(-90);
        painter.drawText(QRectF(-area.height() / 2, -10, area.height(), 20), Qt::AlignHCenter,
                          yLabel_);
        painter.restore();
    }

    // --- Ligne du zéro, si demandée et visible dans la plage affichée ---
    if (showZeroLine_ && yMin <= 0.0 && yMax >= 0.0) {
        QPen zeroPen(palette().mid().color(), 1, Qt::DashLine);
        painter.setPen(zeroPen);
        const double yPixelZero = toPixel(QPointF(xMin, 0.0)).y();
        painter.drawLine(QPointF(area.left(), yPixelZero), QPointF(area.right(), yPixelZero));
    }

    // --- Tracé de la courbe elle-même ---
    QPainterPath path;
    path.moveTo(toPixel(points_.first()));
    for (int i = 1; i < points_.size(); ++i) {
        path.lineTo(toPixel(points_[i]));
    }
    QPen curvePen(palette().highlight().color(), 2);
    painter.setPen(curvePen);
    painter.drawPath(path);
}
