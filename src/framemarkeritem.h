/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#ifndef FRAMEMARKERITEM_H
#define FRAMEMARKERITEM_H

#include <QObject>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "JuceHeader.h"
#include "globals.h"


class FrameMarkerItem : public QObject, public QGraphicsPolygonItem
{
public:
    enum { Type = UserTypes::FRAME_MARKER };

    enum Handle { HANDLE_TOP_BOTTOM, HANDLE_CENTRE_RIGHT, HANDLE_CENTRE_LEFT };

    FrameMarkerItem( QBrush brush, QBrush selectedBrush, qreal height, Handle handle, QGraphicsItem* parent = NULL );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );

    void setHeight( qreal height );

    qreal getHandleWidth() const                { return m_handleWidth; }
    qreal getHandleHeight() const               { return m_handleHeight; }

    int getFrameNum() const                     { return m_frameNum; }
    void setFrameNum( int frameNum )            { m_frameNum = frameNum; }

    int type() const                            { return Type; }

protected:
    // Subclasses that reimplement this method MUST ALWAYS call "return FrameMarkerItem::itemChange"
    // in their reimplementation
    QVariant itemChange( GraphicsItemChange change, const QVariant& value );

    // Subclasses that reimplement this method MUST call FrameMarkerItem::mousePressEvent
    // at the start of the reimplementation
    void mousePressEvent( QGraphicsSceneMouseEvent* event );

    const qreal m_handleWidth;
    const qreal m_handleHeight;

private:
    const Handle m_handle;
    const QBrush m_selectedBrush;

    int m_frameNum;

private:
    JUCE_LEAK_DETECTOR( FrameMarkerItem );
};


#endif // FRAMEMARKERITEM_H
