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

#ifndef WAVEGRAPHICSVIEW_H
#define WAVEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>
#include "JuceHeader.h"
#include "wavegraphicsscene.h"

class WaveGraphicsScene;


class WaveGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    WaveGraphicsView( QWidget* parent = NULL );

    WaveGraphicsScene* getScene() const;

    void redrawWaveforms();

    void zoomIn();
    void zoomOut();
    void zoomOriginal();

    enum InteractionMode { SELECT_MOVE_ITEMS, MULTI_SELECT_ITEMS, AUDITION_ITEMS };

    InteractionMode getInteractionMode() const              { return m_interactionMode; }
    void setInteractionMode( InteractionMode mode );

protected:
    void resizeEvent( QResizeEvent* event );

private:
    ScopedPointer<WaveGraphicsScene> m_scene;

    InteractionMode m_interactionMode;

    bool m_isViewZoomedIn;

signals:
    void minDetailLevelReached();
    void maxDetailLevelReached();

private slots:
    void relayMaxDetailLevelReached();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( WaveGraphicsView );
};


#endif // WAVEGRAPHICSVIEW_H
