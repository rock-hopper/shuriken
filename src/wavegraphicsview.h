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
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include "JuceHeader.h"
#include "waveformitem.h"
#include "slicepointitem.h"
#include "loopmarkeritem.h"
#include "samplebuffer.h"


class WaveGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    WaveGraphicsView( QWidget* parent = NULL );

    // Creates a new waveform item and returns a shared pointer to it
    SharedWaveformItem createWaveform( const SharedSampleBuffer sampleBuffer,
                                       const SharedSampleHeader sampleHeader,
                                       const SharedSampleRange sampleRange );

    // Creates new waveform items and returns a list of shared pointers to them
    QList<SharedWaveformItem> createWaveforms( const SharedSampleBuffer sampleBuffer,
                                               const SharedSampleHeader sampleHeader,
                                               const QList<SharedSampleRange> sampleRangeList );

    // Creates a new waveform item by joining several waveform items together. The new item's start frame,
    // order position and scene position will be the same as the first item in the list. The new item's
    // width and no. of frames will be the sum of the widths and no. of frames of all items in the list
    SharedWaveformItem joinWaveforms( const QList<int> orderPositions );

    // Splits a waveform item that has been created with 'joinWaveforms()'
    QList<SharedWaveformItem> splitWaveform( const int orderPos );

    void moveWaveforms( const QList<int> oldOrderPositions, const int numPlacesMoved );

    // Add waveform items to the scene
    void addWaveforms( const QList<SharedWaveformItem> waveformItems );

    // Remove waveform items from the scene
    QList<SharedWaveformItem> removeWaveforms( const QList<int> waveformOrderPositions );

    QList<int> getSelectedWaveformsOrderPositions() const;

    SharedWaveformItem getWaveformAt( const int orderPos ) const;

    int getNumWaveformItems() const                         { return mWaveformItemList.size(); }

    // Redraw all waveform items
    void redrawWaveforms();

    // Create a new slice point item and add it to the scene
    SharedSlicePointItem createSlicePoint( const int frameNum );

    // Add a slice point item to the scene
    void addSlicePoint( const SharedSlicePointItem slicePoint );

    // Remove a slice point item from the scene
    void removeSlicePoint( const SharedSlicePointItem slicePointItem );

    void moveSlicePoint( const SharedSlicePointItem slicePointItem, const int newFrameNum );

    void hideSlicePoints();
    void showSlicePoints();

    // Returns the currently selected slice point item
    SharedSlicePointItem getSelectedSlicePoint();

    // Returns a sorted list containing the frame no. of every slice point item
    QList<int> getSlicePointFrameNumList() const;

    // Returns a list of all slice point items
    QList<SharedSlicePointItem> getSlicePointList() const   { return mSlicePointItemList; }

    void showLoopMarkers();
    void hideLoopMarkers();

    // The frame no. of each loop marker is set according to the ordering of waveform items
    LoopMarkerItem* getLeftLoopMarker() const               { return mLoopMarkerLeft; }     // Frame no. is inclusive
    LoopMarkerItem* getRightLoopMarker() const              { return mLoopMarkerRight; }    // Frame no. is exclusive

    enum LoopMarkerSnapMode { SNAP_OFF, SNAP_MARKERS_TO_SLICES, SNAP_SLICES_TO_MARKERS };
    void setLoopMarkerSnapMode( const LoopMarkerSnapMode mode )                 { mLoopMarkerSnapMode = mode; }

    QList<SharedSampleRange> getSampleRangesBetweenLoopMarkers( const QList<SharedSampleRange> currentSampleRangeList ) const;
    int getNumFramesBetweenLoopMarkers() const;

    void selectNone();
    void selectAll();

    void startPlayhead( const bool isLoopingEnabled );
    void startPlayhead( const qreal startPosX, const qreal endPosX, const int numFrames );
    void stopPlayhead();
    bool isPlayheadScrolling() const                        { return mTimer->state() == QTimeLine::Running; }

    void clearAll();
    void clearWaveform();

    // These methods do not take the ordering of waveform items into account
    qreal getScenePosX( const int frameNum ) const;
    int getFrameNum( qreal scenePosX ) const;

    void zoomIn();
    void zoomOut();
    void zoomOriginal();

    enum InteractionMode { MOVE_ITEMS, SELECT_ITEMS, AUDITION_ITEMS };
    void setInteractionMode( const InteractionMode mode );

protected:
    void resizeEvent( QResizeEvent* event );

private:
    void resizeWaveformItems( const qreal scaleFactorX );
    void resizeSlicePointItems( const qreal scaleFactorX );
    void resizePlayhead();
    void resizeLoopMarkers( const qreal scaleFactorX );

    void scaleItems( const qreal scaleFactorX );

    void createLoopMarkers();
    void setLoopMarkerFrameNum( LoopMarkerItem* const loopMarker );
    int getWaveformOrderPosUnderLoopMarker( LoopMarkerItem* const loopMarker ) const;
    void updateLoopMarkerFrameNums();
    void snapLoopMarkerToSlicePoint( LoopMarkerItem* const loopMarker );
    void snapLoopMarkerToWaveform( LoopMarkerItem* const loopMarker );
    void snapSlicePointToLoopMarker( SlicePointItem* const slicePoint );

    QList<SharedWaveformItem> mWaveformItemList;
    QList<SharedSlicePointItem> mSlicePointItemList;
    SharedSampleHeader mSampleHeader;

    ScopedPointer<QGraphicsLineItem> mPlayhead;
    ScopedPointer<QTimeLine> mTimer;
    ScopedPointer<QGraphicsItemAnimation> mAnimation;

    ScopedPointer<LoopMarkerItem> mLoopMarkerLeft;
    ScopedPointer<LoopMarkerItem> mLoopMarkerRight;

    LoopMarkerSnapMode mLoopMarkerSnapMode;

    bool mIsViewZoomedIn;

private:
    static int getTotalNumFrames( QList<SharedWaveformItem> items );
    static int getTotalNumFrames( QList<SharedSampleRange> sampleRanges );

signals:
    void slicePointOrderChanged( const SharedSlicePointItem slicePoint, const int oldFrameNum, const int newFrameNum );
    void loopMarkerPosChanged();
    void minDetailLevelReached();
    void maxDetailLevelReached();
    void playheadFinishedScrolling();

private slots:
    // 'oldOrderPositions' is assumed to be sorted
    // 'numPlacesMoved' should be negative if the items have moved left, or positive if the items have moved right
    void reorderWaveformItems( QList<int> oldOrderPositions, const int numPlacesMoved );

    void slideWaveformItemIntoPlace( const int orderPos );
    void updateSlicePointFrameNum( SlicePointItem* const movedItem );
    void updateLoopMarkerFrameNum( LoopMarkerItem* const movedItem );
    void removePlayhead();
    void relayMaxDetailLevelReached();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( WaveGraphicsView );
};


#endif // WAVEGRAPHICSVIEW_H
