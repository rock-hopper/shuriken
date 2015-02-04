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

#include "JuceHeader.h"
#include "framemarkeritem.h"


class SlicePointItem;
typedef QSharedPointer<SlicePointItem> SharedSlicePointItem;


class SlicePointItem : public FrameMarkerItem
{
public:
    enum { Type = UserTypes::SLICE_POINT };

    SlicePointItem( qreal height,
                    bool canBeMovedPastOtherSlicePoints,
                    qreal minDistFromOtherSlicePoints = 1.0,
                    QGraphicsItem* parent = NULL );

    int type() const    { return Type; }

    bool canBeMovedPastOtherSlicePoints() const         { return m_canBeMovedPastOtherSlicePoints; }
    void setMovePastOtherSlicePoints( bool canBeMoved ) { m_canBeMovedPastOtherSlicePoints = canBeMoved; }

    bool isSnapEnabled() const                          { return m_isSnapEnabled; }
    void setSnap( bool enable )                      { m_isSnapEnabled = enable; }

public:
    // For use with qSort(); sorts by frame number
    static bool isLessThanFrameNum( const SharedSlicePointItem item1, const SharedSlicePointItem item2 );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value );

    void mousePressEvent( QGraphicsSceneMouseEvent* event );

private:
    bool m_isSnapEnabled;
    bool m_canBeMovedPastOtherSlicePoints;
    qreal m_minDistFromOtherItems;
    qreal m_minScenePosX;
    qreal m_maxScenePosX;

    JUCE_LEAK_DETECTOR( SlicePointItem );
};


#endif // SLICEPOINTITEM_H
