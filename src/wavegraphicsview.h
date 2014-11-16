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

    // Creates a new waveform item and adds it to the scene. If 'width' is not specified by the caller
    // then the waveform's width will be the same as the scene width
    SharedWaveformItem createWaveform( SharedSampleBuffer sampleBuffer,
                                       SharedSampleHeader sampleHeader,
                                       qreal scenePosX = 0.0,
                                       int orderPos = 0,
                                       qreal width = 0.0 );

    // Creates new waveform items and adds them to the scene. If 'width' is not specified by the caller
    // then the combined widths of all waveforms will be the same as the scene width
    QList<SharedWaveformItem> createWaveforms( QList<SharedSampleBuffer> sampleBufferList,
                                               SharedSampleHeader sampleHeader,
                                               qreal startScenePosX = 0.0,
                                               int startOrderPos = 0,
                                               qreal totalWidth = 0.0 );

    // Creates a new waveform item by joining several waveform items together. The new item's order
    // position and scene position will be the same as the first item in the list. The items'
    // sample buffers are also joined
    SharedWaveformItem joinWaveforms( QList<int> orderPositions );

    // Splits a waveform item into several new waveform items. The item's sample buffer is also split
    QList<SharedWaveformItem> splitWaveform( int orderPos, QList<int> slicePointFrameNums );

    void moveWaveforms( QList<int> oldOrderPositions, int numPlacesMoved );

    // Insert waveform items into the scene
    void insertWaveforms( QList<SharedWaveformItem> waveformItems );

    // Remove waveform items from the scene
    QList<SharedWaveformItem> removeWaveforms( QList<int> waveformOrderPositions );

    QList<int> getSelectedWaveformsOrderPositions() const;

    SharedWaveformItem getWaveformAt( int orderPos ) const;

    int getNumWaveformItems() const                         { return m_waveformItemList.size(); }

    // Redraw all waveform items
    void redrawWaveforms();

    // Create a new slice point item and add it to the scene
    SharedSlicePointItem createSlicePoint( int frameNum );

    // Add a slice point item to the scene
    void addSlicePoint( SharedSlicePointItem slicePoint );

    // Remove a slice point item from the scene
    void removeSlicePoint( SharedSlicePointItem slicePointItem );

    void moveSlicePoint( SharedSlicePointItem slicePointItem, int newFrameNum );

    void hideSlicePoints();
    void showSlicePoints();

    // Returns the currently selected slice point item
    SharedSlicePointItem getSelectedSlicePoint();

    // Returns a sorted list containing the frame no. of every slice point item
    QList<int> getSlicePointFrameNums() const;

    // Returns a list of all slice point items
    QList<SharedSlicePointItem> getSlicePointList() const   { return m_slicePointItemList; }

    void showLoopMarkers();
    void hideLoopMarkers();

    LoopMarkerItem* getLeftLoopMarker() const               { return m_loopMarkerLeft; }
    LoopMarkerItem* getRightLoopMarker() const              { return m_loopMarkerRight; }

    enum LoopMarkerSnapMode { SNAP_OFF, SNAP_MARKERS_TO_SLICES, SNAP_SLICES_TO_MARKERS };
    void setLoopMarkerSnapMode( LoopMarkerSnapMode mode )   { m_loopMarkerSnapMode = mode; }

    void getSampleRangesBetweenLoopMarkers( int& firstOrderPos, QList<SharedSampleRange>& sampleRanges ) const;
    int getNumFramesBetweenLoopMarkers() const;

    void selectNone();
    void selectAll();

    void startPlayhead( bool isLoopingDesired, qreal stretchRatio = 1.0 );
    void startPlayhead( qreal startPosX,
                        qreal endPosX,
                        int numFrames,
                        qreal stretchRatio = 1.0 );
    void stopPlayhead();
    bool isPlayheadScrolling() const                        { return m_timer->state() == QTimeLine::Running; }
    void setPlayheadLooping( bool isLoopingDesired );
    void updatePlayheadSpeed( qreal stretchRatio );

    void clearAll();
    void clearWaveform();

    qreal getScenePosX( int frameNum ) const;
    int getFrameNum( qreal scenePosX ) const;

    void zoomIn();
    void zoomOut();
    void zoomOriginal();

    enum InteractionMode { MOVE_ITEMS, SELECT_ITEMS, AUDITION_ITEMS };
    void setInteractionMode( InteractionMode mode );

protected:
    void resizeEvent( QResizeEvent* event );

private:
    void resizeWaveformItems( qreal scaleFactorX );
    void resizeSlicePointItems( qreal scaleFactorX );
    void resizePlayhead();
    void resizeLoopMarkers( qreal scaleFactorX );

    void scaleItems( qreal scaleFactorX );

    void createLoopMarkers();
    void setLoopMarkerFrameNum( LoopMarkerItem* loopMarker );
    int getRelativeLoopMarkerFrameNum( const LoopMarkerItem* loopMarker ) const;
    SharedWaveformItem getWaveformUnderLoopMarker( const LoopMarkerItem* loopMarker ) const;
    void updateLoopMarkerFrameNums();
    void snapLoopMarkerToSlicePoint( LoopMarkerItem* loopMarker );
    void snapLoopMarkerToWaveform( LoopMarkerItem* loopMarker );
    void snapSlicePointToLoopMarker( SlicePointItem* slicePoint );

    void connectWaveformToGraphicsView( SharedWaveformItem item );

    QList<SharedWaveformItem> m_waveformItemList;
    QList<SharedSlicePointItem> m_slicePointItemList;
    SharedSampleHeader m_sampleHeader;

    ScopedPointer<QGraphicsLineItem> m_playhead;
    ScopedPointer<QTimeLine> m_timer;
    ScopedPointer<QGraphicsItemAnimation> m_animation;

    ScopedPointer<LoopMarkerItem> m_loopMarkerLeft;
    ScopedPointer<LoopMarkerItem> m_loopMarkerRight;

    LoopMarkerSnapMode m_loopMarkerSnapMode;

    InteractionMode m_interactionMode;

    bool m_isViewZoomedIn;

private:
    static int getTotalNumFrames( QList<SharedWaveformItem> waveformItemList );

signals:
    void slicePointOrderChanged( SharedSlicePointItem slicePoint, int oldFrameNum, int newFrameNum );
    void loopMarkerPosChanged();
    void minDetailLevelReached();
    void maxDetailLevelReached();
    void playheadFinishedScrolling();

private slots:
    // 'oldOrderPositions' is assumed to be sorted
    // 'numPlacesMoved' should be negative if the items have moved left, or positive if the items have moved right
    void reorderWaveformItems( QList<int> oldOrderPositions, int numPlacesMoved );

    void slideWaveformItemIntoPlace( int orderPos );
    void updateSlicePointFrameNum( SlicePointItem* movedItem );
    void updateLoopMarkerFrameNum( LoopMarkerItem* movedItem );
    void removePlayhead();
    void relayMaxDetailLevelReached();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( WaveGraphicsView );
};


#endif // WAVEGRAPHICSVIEW_H
