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

#ifndef WAVEGRAPHICSVIEW_H
#define WAVEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QResizeEvent>
#include "waveformitem.h"
#include "slicepointitem.h"
#include "samplebuffer.h"


typedef QSharedPointer<WaveformItem> SharedWaveformItem;


class WaveGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    WaveGraphicsView( QWidget* parent = NULL );

    void createWaveform( const SharedSampleBuffer sampleBuffer );
    QList<SharedWaveformItem> createWaveformSlices( const QList<SharedSampleBuffer> sampleBufferList );
    void moveWaveformSlice( const int oldOrderPos, const int newOrderPos );

    SharedSlicePointItem createSlicePoint( const int frameNum );
    void addSlicePoint( const SharedSlicePointItem slicePoint );
    void deleteSlicePoint( const SharedSlicePointItem slicePointItem );
    void moveSlicePoint( const SharedSlicePointItem slicePointItem, const int newFrameNum );
    SharedSlicePointItem getSelectedSlicePoint();
    void hideSlicePoints();
    void showSlicePoints();
    QList<int> getSlicePointFrameNumList();

    void clearAll();
    void clearWaveform();

    qreal getScenePosX( const int frameNum ) const;
    int getFrameNum( qreal scenePosX ) const;

    void zoomIn();
    void zoomOut();
    void zoomOriginal();

protected:
    void resizeEvent( QResizeEvent* event );

private:
    QList<SharedWaveformItem> mWaveformItemList;
    QList<SharedSlicePointItem> mSlicePointItemList;
    int mNumFrames;

signals:
    void waveformSliceOrderChanged( const int oldOrderPos, const int newOrderPos );
    void slicePointOrderChanged( const SharedSlicePointItem slicePoint, const int oldFrameNum, const int newFrameNum );
    void rightMousePressed( const int waveformItemOrderPos, const int startFrame, const int endFrame );

private slots:
    void reorderWaveformSlices( const int oldOrderPos, const int newOrderPos );
    void slideWaveformSliceIntoPlace( const int orderPos );
    void reorderSlicePoints( SlicePointItem* const movedItem );
    void determinePlayPos( const int waveformItemOrderPos, const QPointF mouseScenePos );
};

#endif // WAVEGRAPHICSVIEW_H
