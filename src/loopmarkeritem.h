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

#ifndef LOOPMARKERITEM_H
#define LOOPMARKERITEM_H

#include "JuceHeader.h"
#include "framemarkeritem.h"


class LoopMarkerItem : public FrameMarkerItem
{
public:
    enum { Type = UserTypes::LOOP_MARKER };

    enum LoopMarkerType { LEFT_MARKER, RIGHT_MARKER };

    LoopMarkerItem( LoopMarkerType markerType, qreal height, QGraphicsItem* parent = NULL );

    int type() const                            { return Type; }
    LoopMarkerType getLoopMarkerType() const    { return m_loopMarkerType; }

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value );

private:
    const LoopMarkerType m_loopMarkerType;

private:
    JUCE_LEAK_DETECTOR( FrameMarkerItem );
};


#endif // LOOPMARKERITEM_H
