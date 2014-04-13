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

#ifndef SLICEPOINTITEM_H
#define SLICEPOINTITEM_H

#include <QObject>
#include <QGraphicsPolygonItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>


class SlicePointItem;
typedef QSharedPointer<SlicePointItem> SharedSlicePointItem;


class SlicePointItem : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 2 };

    SlicePointItem( const qreal height, QGraphicsItem* parent = NULL );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );
    void setHeight( const qreal height );
    int getFrameNum() const                     { return mFrameNum; }
    void setFrameNum( const int frameNum )      { mFrameNum = frameNum; }
    int type() const                            { return Type; }

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void mousePressEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );

private:
    int mFrameNum;
    qreal mScenePosBeforeMove;

signals:
    void scenePosChanged( SlicePointItem* const item );
};


#endif // SLICEPOINTITEM_H
