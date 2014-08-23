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

    FrameMarkerItem( const QBrush brush, const QBrush selectedBrush, const qreal height, const Handle handle, QGraphicsItem* parent = NULL );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );
    void setHeight( const qreal height );
    int getFrameNum() const                     { return mFrameNum; }
    void setFrameNum( const int frameNum )      { mFrameNum = frameNum; }
    int type() const                            { return Type; }

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );

    // Subclasses which reimplement this method MUST call FrameMarkerItem::mousePressEvent( event )
    // at the start of the reimplementation
    void mousePressEvent( QGraphicsSceneMouseEvent* event );

private:
    const QBrush mSelectedBrush;
    const Handle mHandle;

    int mFrameNum;

private:
    JUCE_LEAK_DETECTOR( FrameMarkerItem );
};


#endif // FRAMEMARKERITEM_H
