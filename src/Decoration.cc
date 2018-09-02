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
#include "Decoration.h"
#include "CloseButton.h"
#include "MaximizeButton.h"
#include "MinimizeButton.h"
#include "metrics.h"

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>

// Qt
#include <QPainter>

namespace Material
{

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
{
}

Decoration::~Decoration()
{
}

void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
{
    auto *decoratedClient = client().data();

    if (!decoratedClient->isShaded()) {
        paintFrameBackground(painter, repaintRegion);
    }

    paintTitleBarBackground(painter, repaintRegion);
    paintCaption(painter, repaintRegion);
    paintButtons(painter, repaintRegion);
}

void Decoration::init()
{
    auto *decoratedClient = client().data();

    connect(decoratedClient, &KDecoration2::DecoratedClient::widthChanged,
            this, &Decoration::updateTitleBar);
    connect(decoratedClient, &KDecoration2::DecoratedClient::widthChanged,
            this, &Decoration::updateButtonsGeometry);
    connect(decoratedClient, &KDecoration2::DecoratedClient::maximizedChanged,
            this, &Decoration::updateButtonsGeometry);

    auto repaintTitleBar = [this] {
        update(titleBar());
    };

    connect(decoratedClient, &KDecoration2::DecoratedClient::captionChanged,
            this, repaintTitleBar);
    connect(decoratedClient, &KDecoration2::DecoratedClient::activeChanged,
            this, repaintTitleBar);

    updateBorders();
    updateResizeBorders();
    updateTitleBar();

    auto buttonCreator = [this] (KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
            -> KDecoration2::DecorationButton* {
        Q_UNUSED(decoration)

        switch (type) {
        case KDecoration2::DecorationButtonType::Close:
            return new CloseButton(this, parent);

        case KDecoration2::DecorationButtonType::Maximize:
            return new MaximizeButton(this, parent);

        case KDecoration2::DecorationButtonType::Minimize:
            return new MinimizeButton(this, parent);

        default:
            return nullptr;
        }
    };

    m_leftButtons = new KDecoration2::DecorationButtonGroup(
        KDecoration2::DecorationButtonGroup::Position::Left,
        this,
        buttonCreator);

    m_rightButtons = new KDecoration2::DecorationButtonGroup(
        KDecoration2::DecorationButtonGroup::Position::Right,
        this,
        buttonCreator);

    updateButtonsGeometry();
}

void Decoration::updateBorders()
{
    QMargins borders;
    borders.setTop(titleBarHeight());
    setBorders(borders);
}

void Decoration::updateResizeBorders()
{
    QMargins borders;

    const int extender = settings()->largeSpacing();
    borders.setLeft(extender);
    borders.setTop(extender);
    borders.setRight(extender);
    borders.setBottom(extender);

    setResizeOnlyBorders(borders);
}

void Decoration::updateTitleBar()
{
    auto *decoratedClient = client().data();
    setTitleBar(QRect(0, 0, decoratedClient->width(), titleBarHeight()));
}

void Decoration::updateButtonsGeometry()
{
    if (!m_leftButtons->buttons().isEmpty()) {
        m_leftButtons->setPos(QPointF(0, 0));
        m_leftButtons->setSpacing(0);
    }

    if (!m_rightButtons->buttons().isEmpty()) {
        m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width(), 0));
        m_rightButtons->setSpacing(0);
    }

    update();
}

int Decoration::titleBarHeight() const
{
    const QFontMetrics fontMetrics(settings()->font());
    const int baseUnit = settings()->gridUnit();

    int height = 0;

    height += baseUnit * metrics::TitleBar_TopMargin;
    height += fontMetrics.height();
    height += baseUnit * metrics::TitleBar_BottomMargin;

    return height;
}

void Decoration::paintFrameBackground(QPainter *painter, const QRect &repaintRegion) const
{
    Q_UNUSED(repaintRegion)

    const auto *decoratedClient = client().data();

    painter->save();

    painter->fillRect(rect(), Qt::transparent);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(decoratedClient->color(
        decoratedClient->isActive()
            ? KDecoration2::ColorGroup::Active
            : KDecoration2::ColorGroup::Inactive,
        KDecoration2::ColorRole::Frame));
    painter->setClipRect(0, borderTop(), size().width(), size().height() - borderTop(), Qt::IntersectClip);
    painter->drawRect(rect());

    painter->restore();
}

QColor Decoration::titleBarBackgroundColor() const
{
    const auto *decoratedClient = client().data();
    const auto group = decoratedClient->isActive()
        ? KDecoration2::ColorGroup::Active
        : KDecoration2::ColorGroup::Inactive;
    return decoratedClient->color(group, KDecoration2::ColorRole::TitleBar);
}

QColor Decoration::titleBarForegroundColor() const
{
    const auto *decoratedClient = client().data();
    const auto group = decoratedClient->isActive()
        ? KDecoration2::ColorGroup::Active
        : KDecoration2::ColorGroup::Inactive;
    return decoratedClient->color(group, KDecoration2::ColorRole::Foreground);
}

void Decoration::paintTitleBarBackground(QPainter *painter, const QRect &repaintRegion) const
{
    Q_UNUSED(repaintRegion)

    const auto *decoratedClient = client().data();

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(titleBarBackgroundColor());
    painter->drawRect(QRect(0, 0, decoratedClient->width(), titleBarHeight()));
    painter->restore();
}

void Decoration::paintCaption(QPainter *painter, const QRect &repaintRegion) const
{
    Q_UNUSED(repaintRegion)

    const auto *decoratedClient = client().data();

    const int textWidth = settings()->fontMetrics().boundingRect(decoratedClient->caption()).width();
    const QRect textRect((size().width() - textWidth) / 2, 0, textWidth, titleBarHeight());

    const QRect titleBarRect(0, 0, size().width(), titleBarHeight());

    const QRect availableRect = titleBarRect.adjusted(
        m_leftButtons->geometry().width() + settings()->smallSpacing(), 0,
        -(m_rightButtons->geometry().width() + settings()->smallSpacing()), 0
    );

    QRect captionRect;
    Qt::Alignment alignment;

    if (textRect.left() < availableRect.left()) {
        captionRect = availableRect;
        alignment = Qt::AlignLeft | Qt::AlignVCenter;
    } else if (availableRect.right() < textRect.right()) {
        captionRect = availableRect;
        alignment = Qt::AlignRight | Qt::AlignVCenter;
    } else {
        captionRect = titleBarRect;
        alignment = Qt::AlignCenter;
    }

    const QString caption = painter->fontMetrics().elidedText(
        decoratedClient->caption(), Qt::ElideMiddle, captionRect.width());

    painter->save();
    painter->setFont(settings()->font());
    painter->setPen(titleBarForegroundColor());
    painter->drawText(captionRect, alignment, caption);
    painter->restore();
}

void Decoration::paintButtons(QPainter *painter, const QRect &repaintRegion) const
{
    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

} // namespace Material
