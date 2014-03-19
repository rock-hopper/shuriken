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
    void moveSlicePoint( const int currentFrameNum, const int newFrameNum );
    SharedSlicePointItem getSlicePointAt( const int frameNum );
    void hideSlicePoints();
    void showSlicePoints();
    QList<int> getSlicePointFrameNumList() const    { return mSlicePointFrameNumList; }

    void clearAll();
    void clearWaveform();

protected:
    void resizeEvent( QResizeEvent* event );

private:
    qreal getScenePosX( const int frameNum );
    int getFrameNum( const qreal scenePosX );
    void sortSlicePointLists();

    QList<SharedWaveformItem> mWaveformItemList;
    QList<SharedSlicePointItem> mSlicePointItemList;
    QList<int> mSlicePointFrameNumList;
    int mNumFrames;

private:
    static bool isLessThan( const SharedSlicePointItem& lhs, const SharedSlicePointItem& rhs );

signals:
    void waveformSliceOrderChanged( const int oldOrderPos, const int newOrderPos );
    void slicePointOrderChanged( const int oldFrameNum, const int newFrameNum );

private slots:
    void setZoom( const int zoomFactor );
    void reorderWaveformSlices( const int oldOrderPos, const int newOrderPos );
    void slideWaveformSliceIntoPlace( const int orderPos );
    void reorderSlicePoints( SlicePointItem* const item );
};

#endif // WAVEGRAPHICSVIEW_H
