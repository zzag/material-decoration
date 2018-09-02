/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// own
#include "MaximizeButton.h"
#include "Decoration.h"

// KDecoration
#include <KDecoration2/DecoratedClient>

// KF
#include <KColorUtils>

// Qt
#include <QPainter>

namespace Material
{

MaximizeButton::MaximizeButton(Decoration *decoration, QObject *parent)
    : DecorationButton(KDecoration2::DecorationButtonType::Maximize, decoration, parent)
{
    auto *decoratedClient = decoration->client().data();
    connect(decoratedClient, &KDecoration2::DecoratedClient::maximizeableChanged,
            this, &MaximizeButton::setVisible);

    connect(this, &MaximizeButton::hoveredChanged, this,
        [this] {
            update();
        });

    const int titleBarHeight = decoration->titleBarHeight();
    const QSize size(qRound(titleBarHeight * 1.33), titleBarHeight);
    setGeometry(QRect(QPoint(0, 0), size));
    setVisible(decoratedClient->isMaximizeable());
}

MaximizeButton::~MaximizeButton()
{
}

void MaximizeButton::paint(QPainter *painter, const QRect &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    const QRectF buttonRect = geometry();
    QRectF maximizeRect = QRectF(0, 0, 10, 10);
    maximizeRect.moveCenter(buttonRect.center().toPoint());

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing, false);

    // Background.
    painter->setPen(Qt::NoPen);
    painter->setBrush(backgroundColor());
    painter->drawRect(buttonRect);

    // Foreground.
    painter->setPen(foregroundColor());
    painter->setBrush(Qt::NoBrush);

    if (isChecked()) {
        painter->drawPolygon(QVector<QPointF> {
            maximizeRect.bottomLeft(),
            maximizeRect.topLeft() + QPoint(0, 2),
            maximizeRect.topRight() + QPointF(-2, 2),
            maximizeRect.bottomRight() + QPointF(-2, 0)
        });

        painter->drawPolyline(QVector<QPointF> {
            maximizeRect.topLeft() + QPointF(2, 2),
            maximizeRect.topLeft() + QPointF(2, 0),
            maximizeRect.topRight(),
            maximizeRect.bottomRight() + QPointF(0, -2),
            maximizeRect.bottomRight() + QPointF(-2, -2)
        });
    } else {
        painter->drawRect(maximizeRect);
    }

    painter->restore();
}

QColor MaximizeButton::backgroundColor() const
{
    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return {};
    }

    if (isPressed()) {
        return KColorUtils::mix(
            deco->titleBarBackgroundColor(),
            deco->titleBarForegroundColor(),
            0.3);
    }

    if (isHovered()) {
        return KColorUtils::mix(
            deco->titleBarBackgroundColor(),
            deco->titleBarForegroundColor(),
            0.2);
    }

    return deco->titleBarBackgroundColor();
}

QColor MaximizeButton::foregroundColor() const
{
    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return {};
    }

    return deco->titleBarForegroundColor();
}

} // namespace Material
