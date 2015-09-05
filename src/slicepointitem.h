/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef SLICEPOINTITEM_H
#define SLICEPOINTITEM_H

#include <QObject>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "JuceHeader.h"
#include "globals.h"


class SlicePointItem;
typedef QSharedPointer<SlicePointItem> SharedSlicePointItem;


class SlicePointItem : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT

public:
    enum { Type = UserTypes::SLICE_POINT };

    SlicePointItem( qreal height,
                    bool canBeMovedPastOtherSlicePoints,
                    qreal minDistFromOtherSlicePoints = 1.0,
                    QGraphicsItem* parent = NULL );

    int type() const                                    { return Type; }

    void setHeight( qreal height );
    void setPos( qreal x, qreal y );

    int getFrameNum() const                             { return m_frameNum; }
    void setFrameNum( int frameNum )                    { m_frameNum = frameNum; }

    bool isSnapEnabled() const                          { return m_isSnapEnabled; }
    void setSnap( bool enable )                         { m_isSnapEnabled = enable; }

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );

public:
    // For use with qSort() - sorts slice point items by frame number
    static bool isLessThanFrameNum( const SharedSlicePointItem item1, const SharedSlicePointItem item2 );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value );

    void mousePressEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );

    void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

private:
    // Calculate how far this slice point can be moved to the left and the right
    void calcMinMaxScenePosX();

    const QBrush m_selectedBrush;
    const bool m_canBeMovedPastOtherSlicePoints;

    bool m_isSnapEnabled;
    bool m_isLeftMousePressed;

    int m_frameNum;

    qreal m_scenePosX_beforeMove;
    qreal m_minDistFromOtherItems;
    qreal m_minScenePosX;
    qreal m_maxScenePosX;

private:
    static void setRulerMarkColour( QGraphicsItem* item, QColor colour );

signals:
    void scenePosChanged( SlicePointItem* item );

private:
    JUCE_LEAK_DETECTOR( SlicePointItem );
};


#endif // SLICEPOINTITEM_H
