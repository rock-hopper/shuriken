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

#include "wavegraphicsview.h"
#include <QGLWidget>
#include <QDebug>
#include "audioanalyser.h"
#include "globals.h"


//==================================================================================================
// Public:

WaveGraphicsView::WaveGraphicsView( QWidget* parent ) :
    QGraphicsView( parent ),
    m_loopMarkerSnapMode( SNAP_OFF ),
    m_interactionMode( AUDITION_ITEMS ),
    m_isViewZoomedIn( false )
{
    // Set up view and scene
    setViewport( new QGLWidget( QGLFormat(QGL::SampleBuffers) ) );
    setRenderHint( QPainter::HighQualityAntialiasing, false );
    setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    setOptimizationFlags( DontSavePainterState | DontAdjustForAntialiasing );
    setBackgroundBrush( Qt::gray );
    setCacheMode( CacheBackground );

    setScene( new QGraphicsScene( 0.0, 0.0, 1024.0, 768.0 ) );

    createRuler();

    // Set up playhead
    m_playhead = new QGraphicsLineItem( 0.0, 0.0, 0.0, scene()->height() - 1 );
    m_playhead->setPen( QColor( Qt::red ) );
    m_playhead->setZValue( ZValues::PLAYHEAD );

    m_timer = new QTimeLine();
    m_timer->setFrameRange( 0, 100 );
    m_timer->setCurveShape( QTimeLine::LinearCurve );
    m_timer->setUpdateInterval( 17 );

    m_animation = new QGraphicsItemAnimation;
    m_animation->setItem( m_playhead );
    m_animation->setTimeLine( m_timer );

    QObject::connect( m_timer, SIGNAL( finished() ),
                      this, SLOT( removePlayhead() ) );

    QObject::connect( m_timer, SIGNAL( finished() ),
                      this, SIGNAL( playheadFinishedScrolling() ) );
}



SharedWaveformItem WaveGraphicsView::createWaveform( const SharedSampleBuffer sampleBuffer,
                                                     const SharedSampleHeader sampleHeader,
                                                     const qreal scenePosX,
                                                     const int orderPos,
                                                     qreal width )
{
    Q_ASSERT( sampleBuffer->getNumFrames() > 0 );

    m_sampleHeader = sampleHeader;

    if ( width <= 0.0 )
    {
        width = scene()->width();
    }

    SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer, orderPos, width, scene()->height() ) );
    waveformItem->setPos( scenePosX, Ruler::HEIGHT );

    m_waveformItemList.insert( orderPos, waveformItem );

    connectWaveformToGraphicsView( waveformItem );

    scene()->addItem( waveformItem.data() );
    scene()->update();

    setInteractionMode( m_interactionMode );

    return waveformItem;
}



QList<SharedWaveformItem> WaveGraphicsView::createWaveforms( const QList<SharedSampleBuffer> sampleBufferList,
                                                             const SharedSampleHeader sampleHeader,
                                                             const qreal startScenePosX,
                                                             const int startOrderPos,
                                                             qreal totalWidth )
{
    m_sampleHeader = sampleHeader;

    const int numFrames = SampleUtils::getTotalNumFrames( sampleBufferList );

    if ( totalWidth <= 0.0 )
    {
        totalWidth = scene()->width();
    }

    qreal scenePosX = startScenePosX;
    int orderPos = startOrderPos;

    QList<SharedWaveformItem> newWaveformItems;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        const qreal sliceWidth = sampleBuffer->getNumFrames() * ( totalWidth / numFrames );

        SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer,
                                                           orderPos,
                                                           sliceWidth,
                                                           scene()->height() ) );
        waveformItem->setPos( scenePosX, Ruler::HEIGHT );

        m_waveformItemList.insert( orderPos, waveformItem );
        newWaveformItems << waveformItem;

        connectWaveformToGraphicsView( waveformItem );

        scene()->addItem( waveformItem.data() );
        scene()->update();

        scenePosX += sliceWidth;
        orderPos++;
    }

    setInteractionMode( m_interactionMode );

    return newWaveformItems;
}



void WaveGraphicsView::moveWaveforms( const QList<int> oldOrderPositions, const int numPlacesMoved )
{
    Q_ASSERT( ! m_waveformItemList.isEmpty() );

    reorderWaveformItems( oldOrderPositions, numPlacesMoved );

    foreach ( int orderPos, oldOrderPositions )
    {
        const int newOrderPos = orderPos + numPlacesMoved;
        slideWaveformItemIntoPlace( newOrderPos );
    }
}



void WaveGraphicsView::insertWaveforms( const QList<SharedWaveformItem> waveformItems )
{
    const int numItemsToAdd = waveformItems.size();
    const int firstOrderPos = waveformItems.first()->getOrderPos();

    // If necessary, set new order positions
    for ( int i = firstOrderPos; i < m_waveformItemList.size(); i++ )
    {
        m_waveformItemList.at( i )->setOrderPos( i + numItemsToAdd );
    }

    // Add new waveform items to list
    foreach ( SharedWaveformItem item, waveformItems )
    {
        m_waveformItemList.insert( item->getOrderPos(), item );
    }

    // Resize and reposition all waveform items
    const int numFrames = getTotalNumFrames( m_waveformItemList );
    qreal scenePosX = 0.0;

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        const qreal itemWidth = item->getSampleBuffer()->getNumFrames() * ( scene()->width() / numFrames );

        item->setRect( 0.0, 0.0, itemWidth, scene()->height() );
        item->setPos( scenePosX, Ruler::HEIGHT );

        scenePosX += itemWidth;
    }

    // Add waveform items to scene
    foreach ( SharedWaveformItem item, waveformItems )
    {
        scene()->addItem( item.data() );
    }
    scene()->update();

    setInteractionMode( m_interactionMode );

    // Reset loop markers
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        const int startFrame = 0;
        const int endFrame = getTotalNumFrames( m_waveformItemList ) - 1;

        m_loopMarkerLeft->setFrameNum( startFrame );
        m_loopMarkerRight->setFrameNum( endFrame );

        m_loopMarkerLeft->setPos( 0.0, Ruler::HEIGHT );
        m_loopMarkerRight->setPos( getScenePosX( endFrame ), Ruler::HEIGHT );
    }
}



QList<SharedWaveformItem> WaveGraphicsView::removeWaveforms( const QList<int> waveformOrderPositions )
{
    const int startOrderPos = waveformOrderPositions.first();

    QList<SharedWaveformItem> removedWaveforms;

    // Remove waveform items from scene
    for ( int i = 0; i < waveformOrderPositions.size(); i++ )
    {
        SharedWaveformItem item = m_waveformItemList.at( startOrderPos );

        m_waveformItemList.removeAt( startOrderPos );
        scene()->removeItem( item.data() );

        removedWaveforms << item;
    }
    scene()->update();

    // If necessary, set new order positions and remove gap between remaining items
    if ( startOrderPos < m_waveformItemList.size() )
    {
        qreal distanceToMove = 0.0;

        foreach ( SharedWaveformItem item, removedWaveforms )
        {
            distanceToMove += item->rect().width();
        }

        for ( int i = startOrderPos; i < m_waveformItemList.size(); i++ )
        {
            m_waveformItemList.at( i )->setOrderPos( i );

            const qreal oldScenePosX = m_waveformItemList.at( i )->scenePos().x();
            m_waveformItemList.at( i )->setPos( oldScenePosX - distanceToMove, Ruler::HEIGHT );
        }
    }

    // Resize remaining waveform items
    qreal totalWidth = 0.0;

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        totalWidth += item->rect().width();
    }

    resizeWaveformItems( scene()->width() / totalWidth );

    // Reset loop markers
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        const int startFrame = 0;
        const int endFrame = getTotalNumFrames( m_waveformItemList ) - 1;

        m_loopMarkerLeft->setFrameNum( startFrame );
        m_loopMarkerRight->setFrameNum( endFrame );

        m_loopMarkerLeft->setPos( 0.0, Ruler::HEIGHT );
        m_loopMarkerRight->setPos( getScenePosX( endFrame ), Ruler::HEIGHT );
    }

    return removedWaveforms;
}



QList<int> WaveGraphicsView::getSelectedWaveformsOrderPositions() const
{
    const QList<WaveformItem*> selectedItems = WaveformItem::getSelectedWaveformItems( scene() );
    QList<int> orderPositions;

    foreach ( WaveformItem* item, selectedItems )
    {
        orderPositions << item->getOrderPos();
    }

    return orderPositions;
}



SharedWaveformItem WaveGraphicsView::getWaveformAt( const int orderPos ) const
{
    return m_waveformItemList.at( orderPos );
}



void WaveGraphicsView::redrawWaveforms()
{
    resizeWaveformItems( 1.0 );
    viewport()->update();
}



SharedSlicePointItem WaveGraphicsView::createSlicePoint( const int frameNum, const bool canBeMovedPastOtherSlicePoints )
{
    const qreal scenePosX = getScenePosX( frameNum );

    SlicePointItem* item = new SlicePointItem( scene()->height() - Ruler::HEIGHT - 1, canBeMovedPastOtherSlicePoints );
    item->setPos( scenePosX, Ruler::HEIGHT );
    item->setFrameNum( frameNum );

    QTransform matrix;
    const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point handle is set to correct width even if view is scaled
    item->setTransform( matrix );

    QObject::connect( item, SIGNAL( scenePosChanged(FrameMarkerItem*) ),
                      this, SLOT( updateSlicePointFrameNum(FrameMarkerItem*) ) );

    scene()->addItem( item );
    scene()->update();

    SharedSlicePointItem sharedSlicePoint = SharedSlicePointItem( item );
    m_slicePointItemList.append( sharedSlicePoint );

    return sharedSlicePoint;
}



void WaveGraphicsView::addSlicePoint( const SharedSlicePointItem slicePoint )
{
    const int slicePointFrameNum = slicePoint.data()->getFrameNum();
    const qreal scenePosX = getScenePosX( slicePointFrameNum );

    QTransform matrix;
    const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point remains same width when view is scaled
    slicePoint.data()->setTransform( matrix );

    slicePoint.data()->setHeight( scene()->height() - Ruler::HEIGHT - 1 );
    slicePoint.data()->setPos( scenePosX, Ruler::HEIGHT );

    m_slicePointItemList.append( slicePoint );

    scene()->addItem( slicePoint.data() );
    scene()->update();
}



void WaveGraphicsView::removeSlicePoint( const SharedSlicePointItem slicePointItem )
{
    scene()->removeItem( slicePointItem.data() );
    scene()->update();

    m_slicePointItemList.removeOne( slicePointItem );
}



void WaveGraphicsView::moveSlicePoint( const SharedSlicePointItem slicePointItem, const int newFrameNum )
{
    const qreal newScenePosX = getScenePosX( newFrameNum );

    slicePointItem->setFrameNum( newFrameNum );
    slicePointItem->setPos( newScenePosX, Ruler::HEIGHT );
}



void WaveGraphicsView::hideSlicePoints()
{
    foreach ( SharedSlicePointItem item, m_slicePointItemList )
    {
        item->setVisible( false );
    }
}



void WaveGraphicsView::showSlicePoints()
{
    foreach ( SharedSlicePointItem item, m_slicePointItemList )
    {
        item->setVisible( true );
    }
}



SharedSlicePointItem WaveGraphicsView::getSelectedSlicePoint()
{
    const QList<QGraphicsItem*> selectedItems = scene()->selectedItems();
    SharedSlicePointItem selectedSlicePointItem;

    if ( ! selectedItems.isEmpty() )
    {
        QGraphicsItem* const item = selectedItems.first();

        if ( item->type() == SlicePointItem::Type )
        {
            SlicePointItem* const slicePointItem = qgraphicsitem_cast<SlicePointItem*>( item );

            foreach ( SharedSlicePointItem sharedSlicePointItem, m_slicePointItemList )
            {
                if ( sharedSlicePointItem == slicePointItem )
                {
                    selectedSlicePointItem = sharedSlicePointItem;
                    break;
                }
            }
        }
    }

    return selectedSlicePointItem;
}



QList<int> WaveGraphicsView::getSlicePointFrameNums() const
{
    QList<int> slicePointFrameNums;

    foreach ( SharedSlicePointItem slicePointItem, m_slicePointItemList )
    {
        slicePointFrameNums.append( slicePointItem->getFrameNum() );
    }

    qSort( slicePointFrameNums );

    const int totalNumFrames = getTotalNumFrames( m_waveformItemList );
    const int minFramesBetweenSlicePoints = roundToInt( m_sampleHeader->sampleRate * AudioAnalyser::MIN_INTER_ONSET_SECS );

    QList<int> amendedFrameNumList;
    int prevFrameNum = 0;

    foreach ( int frameNum, slicePointFrameNums )
    {
        if ( frameNum > minFramesBetweenSlicePoints &&
             frameNum < totalNumFrames - minFramesBetweenSlicePoints )
        {
            if ( frameNum > prevFrameNum + minFramesBetweenSlicePoints )
            {
                amendedFrameNumList << frameNum;
                prevFrameNum = frameNum;
            }
        }
    }

    return amendedFrameNumList;
}



void WaveGraphicsView::showLoopMarkers()
{
    if ( m_loopMarkerLeft == NULL && m_loopMarkerRight == NULL )
    {
        createLoopMarkers();
    }

    m_loopMarkerLeft->setVisible( true );
    m_loopMarkerRight->setVisible( true );
}



void WaveGraphicsView::hideLoopMarkers()
{
    m_loopMarkerLeft->setVisible( false );
    m_loopMarkerRight->setVisible( false );
}



void WaveGraphicsView::getSampleRangesBetweenLoopMarkers( int& firstOrderPos, QList<SharedSampleRange>& sampleRanges ) const
{
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL)
    {
        const int leftMarkerFrameNum = getRelativeLoopMarkerFrameNum( m_loopMarkerLeft );
        const int rightMarkerFrameNum = getRelativeLoopMarkerFrameNum( m_loopMarkerRight );

        const int leftWaveformOrderPos = getWaveformUnderLoopMarker( m_loopMarkerLeft )->getOrderPos();
        const int rightWaveformOrderPos = getWaveformUnderLoopMarker( m_loopMarkerRight )->getOrderPos();

        const int minNumFrames = roundToInt( m_sampleHeader->sampleRate * AudioAnalyser::MIN_INTER_ONSET_SECS );

        bool isFirstOrderPos = true;

        for ( int orderPos = leftWaveformOrderPos; orderPos <= rightWaveformOrderPos; orderPos++ )
        {
            int startFrame = 0;
            int numFrames = m_waveformItemList.at( orderPos )->getSampleBuffer()->getNumFrames();

            if ( orderPos == leftWaveformOrderPos )
            {
                startFrame = leftMarkerFrameNum;
            }

            if ( orderPos == rightWaveformOrderPos )
            {
                numFrames = rightMarkerFrameNum - startFrame;
            }
            else
            {
                numFrames = numFrames - startFrame;
            }

            if ( numFrames > minNumFrames )
            {
                if ( isFirstOrderPos )
                {
                    firstOrderPos = orderPos;
                    isFirstOrderPos = false;
                }

                SharedSampleRange range( new SampleRange );

                range->startFrame = startFrame;
                range->numFrames = numFrames;

                sampleRanges << range;
            }
        }
    }
}



int WaveGraphicsView::getNumFramesBetweenLoopMarkers() const
{
    int numFrames = 0;

    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        numFrames = m_loopMarkerRight->getFrameNum() - m_loopMarkerLeft->getFrameNum();
    }

    return numFrames;
}



void WaveGraphicsView::selectNone()
{
    foreach ( SharedSlicePointItem item, m_slicePointItemList )
    {
        item->setSelected( false );
    }

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        item->setSelected( false );
    }
}



void WaveGraphicsView::selectAll()
{
    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        item->setSelected( true );
    }
}



void WaveGraphicsView::startPlayhead( const bool isLoopingDesired, const qreal stretchRatio )
{
    const qreal sampleRate = m_sampleHeader->sampleRate;

    if ( sampleRate > 0.0 )
    {
        int numFrames = getTotalNumFrames( m_waveformItemList );

        qreal startPosX = 0.0;
        qreal endPosX   = scene()->width() - 1;

        if ( m_loopMarkerLeft != NULL && m_loopMarkerLeft->isVisible() )
        {
            numFrames = getFrameNum( m_loopMarkerRight->scenePos().x() - m_loopMarkerLeft->scenePos().x() );
            startPosX = m_loopMarkerLeft->scenePos().x();
            endPosX   = m_loopMarkerRight->scenePos().x();
        }

        const int millis = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );

        if ( isPlayheadScrolling() )
        {
            stopPlayhead();
        }

        m_animation->setPosAt( 0.0, QPointF( startPosX, Ruler::HEIGHT ) );
        m_animation->setPosAt( 1.0, QPointF( endPosX,   Ruler::HEIGHT ) );

        m_playhead->setLine( 0.0, 0.0, 0.0, scene()->height() - 1 );
        m_playhead->setVisible( true );
        scene()->addItem( m_playhead );

        if ( isLoopingDesired )
        {
            m_timer->setLoopCount( 0 );
        }
        else
        {
            m_timer->setLoopCount( 1 );
        }
        m_timer->setDuration( millis );
        m_timer->start();
    }
}



void WaveGraphicsView::startPlayhead( const qreal startPosX, const qreal endPosX, const int numFrames, const qreal stretchRatio )
{
    const qreal sampleRate = m_sampleHeader->sampleRate;

    if ( sampleRate > 0.0 )
    {
        const int millis = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );

        if ( isPlayheadScrolling() )
        {
            stopPlayhead();
        }

        m_animation->setPosAt( 0.0, QPointF( startPosX, Ruler::HEIGHT ) );
        m_animation->setPosAt( 1.0, QPointF( endPosX,   Ruler::HEIGHT ) );

        m_playhead->setLine( 0.0, 0.0, 0.0, scene()->height() - 1 );
        m_playhead->setVisible( true );
        scene()->addItem( m_playhead );

        m_timer->setLoopCount( 1 );
        m_timer->setDuration( millis );
        m_timer->start();
    }
}



void WaveGraphicsView::stopPlayhead()
{
    if ( isPlayheadScrolling() )
    {
        m_timer->stop();
        removePlayhead();
    }
}



void WaveGraphicsView::setPlayheadLooping( const bool isLoopingDesired )
{
    if ( isLoopingDesired )
    {
        m_timer->setLoopCount( 0 );
    }
    else
    {
        m_timer->setLoopCount( 1 );
    }
}



void WaveGraphicsView::updatePlayheadSpeed( const qreal stretchRatio )
{
    if ( isPlayheadScrolling() )
    {
        m_playhead->setVisible( false );

        m_timer->stop();

        const qreal sampleRate = m_sampleHeader->sampleRate;
        int numFrames = 0;

        if ( m_loopMarkerLeft != NULL && m_loopMarkerLeft->isVisible() )
        {
            numFrames = getFrameNum( m_loopMarkerRight->scenePos().x() - m_loopMarkerLeft->scenePos().x() );
        }
        else
        {
            numFrames = getTotalNumFrames( m_waveformItemList );
        }

        const int newDuration = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );
//        const int newTime = roundToInt( mTimer->currentTime() * stretchRatio );

        m_timer->setDuration( newDuration );
//        mTimer->setCurrentTime( newTime );

        m_timer->resume();
    }
}



void WaveGraphicsView::clearAll()
{
    foreach ( QGraphicsItem* item, items() )
    {
        scene()->removeItem( item );
    }
    scene()->update();

    m_waveformItemList.clear();
    m_slicePointItemList.clear();
    m_rulerMarksList.clear();
    m_loopMarkerLeft = NULL;
    m_loopMarkerRight = NULL;

    createRuler();
}



void WaveGraphicsView::clearWaveform()
{
    foreach ( QGraphicsItem* item, items() )
    {
        if ( item->type() == WaveformItem::Type )
        {
            scene()->removeItem( item );
        }
    }
    scene()->update();

    m_waveformItemList.clear();
}



qreal WaveGraphicsView::getScenePosX( const int frameNum ) const
{
    const int numFrames = getTotalNumFrames( m_waveformItemList );

    qreal scenePosX = frameNum * ( scene()->width() / numFrames );

    if ( scenePosX < 0.0)
        scenePosX = 0.0;

    if ( scenePosX >= scene()->width() )
        scenePosX = scene()->width() - 1;

    return scenePosX;
}



int WaveGraphicsView::getFrameNum( qreal scenePosX ) const
{
    const int numFrames = getTotalNumFrames( m_waveformItemList );

    int frameNum = roundToInt( scenePosX / ( scene()->width() / numFrames ) );

    if ( frameNum < 0 )
        frameNum = 0;

    if ( frameNum >= numFrames )
        frameNum = numFrames - 1;

    return frameNum;
}



void WaveGraphicsView::zoomIn()
{
    m_isViewZoomedIn = true;

    const qreal newXScaleFactor = transform().m11() * 2; // m11() returns the current horizontal scale factor

    QTransform matrix;
    matrix.scale( newXScaleFactor, 1.0 );
    setTransform( matrix );

    scaleItems( newXScaleFactor );
}



void WaveGraphicsView::zoomOut()
{
    const qreal newXScaleFactor = transform().m11() * 0.5; // m11() returns the current horizontal scale factor

    QTransform matrix;
    matrix.scale( newXScaleFactor, 1.0 );
    setTransform( matrix );

    scaleItems( newXScaleFactor );

    if ( newXScaleFactor == 1.0 )
    {
        emit minDetailLevelReached();
        m_isViewZoomedIn = false;
    }
}



void WaveGraphicsView::zoomOriginal()
{
    m_isViewZoomedIn = false;

    resetTransform();
    scaleItems( 1.0 );
}



void WaveGraphicsView::setInteractionMode( const InteractionMode mode )
{
    switch ( mode )
    {
    case SELECT_MOVE_ITEMS:
        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, true );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        setDragMode( NoDrag );
        break;
    case MULTI_SELECT_ITEMS:
        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        setDragMode( RubberBandDrag );
        break;
    case AUDITION_ITEMS:
        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, false );
        }
        setDragMode( NoDrag );
        break;
    default:
        break;
    }

    m_interactionMode = mode;
}



void WaveGraphicsView::setBpmRulerMarks( const qreal bpm, const int timeSigNumerator )
{
    if ( bpm > 0.0 && timeSigNumerator > 0 )
    {
        foreach ( SharedGraphicsItem item, m_rulerMarksList )
        {
            scene()->removeItem( item.data() );
        }

        m_rulerMarksList.clear();

        const int totalNumFrames = getTotalNumFrames( m_waveformItemList );
        const qreal framesPerBeat = ( m_sampleHeader->sampleRate * 60 ) / bpm;

        int beat = 0;
        int bar = 1;

        for ( int frameNum = 0; frameNum < totalNumFrames; frameNum += framesPerBeat, beat++ )
        {
            if ( beat % timeSigNumerator == 0 )
            {
                QGraphicsSimpleTextItem* textItem = scene()->addSimpleText( QString::number( bar ) );
                textItem->setPos( getScenePosX( frameNum ), 1.0 );
                textItem->setBrush( Qt::white );
                textItem->setZValue( 1 );
                m_rulerMarksList.append( SharedGraphicsItem( textItem ) );
                bar++;
            }
            else
            {
                QGraphicsLineItem* lineItem = scene()->addLine( 0.0, 0.0, 0.0, Ruler::HEIGHT - 5.0 );
                lineItem->setPos( getScenePosX( frameNum ), 2.0 );
                lineItem->setPen( QPen( Qt::white ) );
                lineItem->setZValue( 1 );
                m_rulerMarksList.append( SharedGraphicsItem( lineItem ) );
            }
        }
    }
}



//==================================================================================================
// Protected:

void WaveGraphicsView::resizeEvent ( QResizeEvent* event )
{
    scene()->setSceneRect( 0.0, 0.0, event->size().width(), event->size().height() );

    if ( event->oldSize().width() > 0 )
    {
        const qreal scaleFactorX = scene()->width() / event->oldSize().width();

        resizeWaveformItems( scaleFactorX );
        resizeSlicePointItems( scaleFactorX );
        resizePlayhead();
        resizeLoopMarkers( scaleFactorX );
        resizeRuler( scaleFactorX );
    }

    QGraphicsView::resizeEvent( event );
}



//==================================================================================================
// Private:

void WaveGraphicsView::resizeWaveformItems( const qreal scaleFactorX )
{
    if ( ! m_waveformItemList.isEmpty() )
    {
        foreach ( SharedWaveformItem waveformItem, m_waveformItemList )
        {
            const qreal newWidth = waveformItem->rect().width() * scaleFactorX;
            waveformItem->setRect( 0.0, 0.0, newWidth, scene()->height() );

            const qreal newX = waveformItem->scenePos().x() * scaleFactorX;
            waveformItem->setPos( newX, Ruler::HEIGHT );
        }
    }
}



void WaveGraphicsView::resizeSlicePointItems( const qreal scaleFactorX )
{
    foreach ( SharedSlicePointItem slicePointItem, m_slicePointItemList )
    {
        slicePointItem->setHeight( scene()->height() - Ruler::HEIGHT - 1 );

        const bool canBeMoved = slicePointItem->canBeMovedPastOtherSlicePoints();
        const qreal newX = slicePointItem->scenePos().x() * scaleFactorX;

        slicePointItem->setMovePastOtherSlicePoints( true );
        slicePointItem->setPos( newX, Ruler::HEIGHT );
        slicePointItem->setMovePastOtherSlicePoints( canBeMoved );
    }
}



void WaveGraphicsView::resizePlayhead()
{
    if ( m_timer->state() == QTimeLine::Running )
    {
        m_timer->stop();

        m_animation->clear();
        m_animation->setPosAt( 0.0, QPointF( 0.0, Ruler::HEIGHT ) );
        m_animation->setPosAt( 1.0, QPointF( scene()->width() - 1, Ruler::HEIGHT ) );

        m_playhead->setLine( 0.0, 0.0, 0.0, scene()->height() - Ruler::HEIGHT - 1 );

        m_timer->resume();
    }
}



void WaveGraphicsView::resizeLoopMarkers( const qreal scaleFactorX )
{
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        m_loopMarkerLeft->setHeight( scene()->height() - Ruler::HEIGHT - 1 );
        m_loopMarkerRight->setHeight( scene()->height() - Ruler::HEIGHT - 1 );
        {
            const qreal newX = m_loopMarkerLeft->scenePos().x() * scaleFactorX;
            m_loopMarkerLeft->setPos( newX, Ruler::HEIGHT );
        }
        {
            const qreal newX = m_loopMarkerRight->scenePos().x() * scaleFactorX;
            m_loopMarkerRight->setPos( newX, Ruler::HEIGHT );
        }
    }
}



void WaveGraphicsView::resizeRuler( const qreal scaleFactorX )
{
    m_rulerBackground->setRect( 0.0, 0.0, scene()->width(), Ruler::HEIGHT );

    foreach ( SharedGraphicsItem item, m_rulerMarksList )
    {
        const qreal newX = item->scenePos().x() * scaleFactorX;
        item->setPos( newX, 1.0 );
    }
}



void WaveGraphicsView::scaleItems( const qreal scaleFactorX )
{
    if ( scaleFactorX > 0.0 )
    {
        QTransform matrix;
        matrix.scale( 1.0 / scaleFactorX, 1.0 ); // Items remain same width when view is scaled

        foreach ( SharedSlicePointItem slicePointItem, m_slicePointItemList )
        {
            slicePointItem->setTransform( matrix );
        }

        foreach ( SharedGraphicsItem item, m_rulerMarksList )
        {
            item->setTransform( matrix );
        }

        if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
        {
            m_loopMarkerLeft->setTransform( matrix );
            m_loopMarkerRight->setTransform( matrix );
        }
    }
}



void WaveGraphicsView::createLoopMarkers()
{
    m_loopMarkerLeft = new LoopMarkerItem( LoopMarkerItem::LEFT_MARKER, scene()->height() - Ruler::HEIGHT - 1 );

    m_loopMarkerRight = new LoopMarkerItem( LoopMarkerItem::RIGHT_MARKER, scene()->height() - Ruler::HEIGHT - 1 );

    const int startFrame = 0;
    const int endFrame = getTotalNumFrames( m_waveformItemList ) - 1;

    m_loopMarkerLeft->setFrameNum( startFrame );
    m_loopMarkerRight->setFrameNum( endFrame );

    m_loopMarkerLeft->setPos( 0.0, Ruler::HEIGHT );
    m_loopMarkerRight->setPos( getScenePosX( endFrame ), Ruler::HEIGHT );

    QTransform matrix;
    const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // loop marker remains correct width if view is scaled
    m_loopMarkerLeft->setTransform( matrix );
    m_loopMarkerRight->setTransform( matrix );

    QObject::connect( m_loopMarkerLeft, SIGNAL( scenePosChanged(FrameMarkerItem*) ),
                      this, SLOT( updateLoopMarkerFrameNum(FrameMarkerItem*) ) );

    QObject::connect( m_loopMarkerRight, SIGNAL( scenePosChanged(FrameMarkerItem*) ),
                      this, SLOT( updateLoopMarkerFrameNum(FrameMarkerItem*) ) );

    scene()->addItem( m_loopMarkerLeft );
    scene()->addItem( m_loopMarkerRight );
    scene()->update();

    updateLoopMarkerFrameNum( m_loopMarkerLeft );
    updateLoopMarkerFrameNum( m_loopMarkerRight );
}



void WaveGraphicsView::setLoopMarkerFrameNum( LoopMarkerItem* const loopMarker )
{
    if ( loopMarker != NULL )
    {
        int newFrameNum = getFrameNum( loopMarker->scenePos().x() );

        loopMarker->setFrameNum( newFrameNum );
    }
}



int WaveGraphicsView::getRelativeLoopMarkerFrameNum( const LoopMarkerItem* const loopMarker ) const
{
    int frameNum = 0;

    if ( loopMarker != NULL )
    {
        frameNum = loopMarker->getFrameNum();

        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            const int numFrames = item->getSampleBuffer()->getNumFrames();

            if ( frameNum < numFrames )
            {
                break;
            }

            frameNum -= numFrames;
        }
    }

    return frameNum;
}



SharedWaveformItem WaveGraphicsView::getWaveformUnderLoopMarker( const LoopMarkerItem* const loopMarker ) const
{
    SharedWaveformItem waveformItem;

    if ( loopMarker != NULL )
    {
        int frameNum = loopMarker->getFrameNum();

        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            const int numFrames = item->getSampleBuffer()->getNumFrames();

            if ( frameNum < numFrames )
            {
                waveformItem = item;
                break;
            }

            frameNum -= numFrames;
        }
    }

    return waveformItem;
}



void WaveGraphicsView::updateLoopMarkerFrameNums()
{
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        setLoopMarkerFrameNum( m_loopMarkerLeft );
        setLoopMarkerFrameNum( m_loopMarkerRight );

        emit loopMarkerPosChanged();
    }
}



void WaveGraphicsView::snapLoopMarkerToSlicePoint( LoopMarkerItem* const loopMarker )
{
    if ( loopMarker != NULL )
    {
        const int oldFrameNum = loopMarker->getFrameNum();
        const int minFrameNum = 0;
        const int maxFrameNum = getTotalNumFrames( m_waveformItemList ) - 1;

        QList<int> frameNumList;

        frameNumList << minFrameNum;

        foreach ( SharedSlicePointItem slicePoint, m_slicePointItemList )
        {
            const int frameNum = slicePoint->getFrameNum();

            if ( frameNum > minFrameNum && frameNum < maxFrameNum )
                frameNumList << frameNum;
        }

        frameNumList << maxFrameNum;

        int newFrameNum = 0;
        int smallestNumFrames = maxFrameNum;

        foreach ( int frameNum, frameNumList )
        {
            const int numFrames = qAbs( oldFrameNum - frameNum );

            if ( numFrames < smallestNumFrames )
            {
                smallestNumFrames = numFrames;
                newFrameNum = frameNum;
            }
        }

        loopMarker->setFrameNum( newFrameNum );
        loopMarker->setPos( getScenePosX( newFrameNum ), Ruler::HEIGHT );
    }
}



void WaveGraphicsView::snapLoopMarkerToWaveform( LoopMarkerItem* const loopMarker )
{
    if ( loopMarker != NULL )
    {
        const int oldFrameNum = loopMarker->getFrameNum();

        QList<int> frameNumList;

        int frameNum = 0;

        for ( int i = 0; i < m_waveformItemList.size(); i++ )
        {
            frameNumList << frameNum;
            frameNum += m_waveformItemList.at( i )->getSampleBuffer()->getNumFrames();
        }

        frameNumList << frameNum - 1;

        int newFrameNum = 0;
        int smallestNumFrames = frameNumList.last();

        foreach ( int frameNum, frameNumList )
        {
            const int numFrames = qAbs( oldFrameNum - frameNum );

            if ( numFrames < smallestNumFrames )
            {
                smallestNumFrames = numFrames;
                newFrameNum = frameNum;
            }
        }

        loopMarker->setFrameNum( newFrameNum );
        loopMarker->setPos( getScenePosX( newFrameNum ), Ruler::HEIGHT );
    }
}



void WaveGraphicsView::snapSlicePointToLoopMarker( SlicePointItem* const slicePoint )
{
    if ( slicePoint != NULL && m_loopMarkerLeft != NULL && m_loopMarkerLeft->isVisible() )
    {
        const qreal snapThreshold = 30.0;

        qreal scenePosX = slicePoint->scenePos().x();
        int frameNum = slicePoint->getFrameNum();

        if ( qAbs( scenePosX - m_loopMarkerLeft->scenePos().x() ) <= snapThreshold )
        {
            scenePosX = m_loopMarkerLeft->scenePos().x();
            frameNum = m_loopMarkerLeft->getFrameNum();
        }
        else if ( qAbs( scenePosX - m_loopMarkerRight->scenePos().x() ) <= snapThreshold )
        {
            scenePosX = m_loopMarkerRight->scenePos().x();
            frameNum = m_loopMarkerRight->getFrameNum();
        }

        slicePoint->setPos( scenePosX, Ruler::HEIGHT );
        slicePoint->setFrameNum( frameNum );
    }
}



void WaveGraphicsView::connectWaveformToGraphicsView( const SharedWaveformItem item )
{
    QObject::connect( item.data(), SIGNAL( orderPosIsChanging(QList<int>,int) ),
                      this, SLOT( reorderWaveformItems(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( finishedMoving(int) ),
                      this, SLOT( slideWaveformItemIntoPlace(int) ) );

    QObject::connect( item.data(), SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( relayMaxDetailLevelReached() ) );
}



void WaveGraphicsView::createRuler()
{
    m_rulerBackground = scene()->addRect( 0.0, 0.0, scene()->width(), Ruler::HEIGHT, QPen(), QBrush( Qt::black ) );

    QGraphicsSimpleTextItem* textItem = scene()->addSimpleText( "0 BPM" );
    textItem->setPos( 1.0, 1.0 );
    textItem->setBrush( Qt::white );
    textItem->setZValue( 1 );
    m_rulerMarksList.append( SharedGraphicsItem( textItem ) );
}



//==================================================================================================
// Private Static:

int WaveGraphicsView::getTotalNumFrames( QList<SharedWaveformItem> waveformItemList )
{
    int numFrames = 0;

    foreach ( SharedWaveformItem item, waveformItemList )
    {
        numFrames += item->getSampleBuffer()->getNumFrames();
    }

    return numFrames;
}



//==================================================================================================
// Private Slots:

void WaveGraphicsView::reorderWaveformItems( QList<int> oldOrderPositions, const int numPlacesMoved )
{
    const int numSelectedItems = oldOrderPositions.size();

    qreal distanceToMove = 0.0;

    foreach ( int orderPos, oldOrderPositions )
    {
        distanceToMove += m_waveformItemList.at( orderPos )->rect().width();
    }

    QList<int> itemsToMoveCurrentOrderPositions;

    // If waveform items have been dragged to the left...
    if ( numPlacesMoved < 0 )
    {
        int pos = oldOrderPositions.first() + numPlacesMoved;

        for ( int num = 0; num < abs( numPlacesMoved ); num++ )
        {
            itemsToMoveCurrentOrderPositions << pos++;
        }

        foreach ( int orderPos, itemsToMoveCurrentOrderPositions )
        {
            const qreal currentScenePosX = m_waveformItemList.at( orderPos )->scenePos().x();

            m_waveformItemList.at( orderPos )->setPos( currentScenePosX + distanceToMove, Ruler::HEIGHT );
            m_waveformItemList.at( orderPos )->setOrderPos( orderPos + numSelectedItems );
        }

        for ( int i = 0; i < numSelectedItems; i++ )
        {
            const int orderPos = oldOrderPositions.at( i );

            m_waveformItemList.at( orderPos )->setOrderPos( orderPos + numPlacesMoved );
            m_waveformItemList.move( orderPos, orderPos + numPlacesMoved );
        }
    }
    else // If waveform items have been dragged to the right...
    {
        int pos = oldOrderPositions.last() + 1;

        for ( int num = 0; num < numPlacesMoved; num++ )
        {
            itemsToMoveCurrentOrderPositions << pos++;
        }

        foreach ( int orderPos, itemsToMoveCurrentOrderPositions )
        {
            const qreal currentScenePosX = m_waveformItemList.at( orderPos )->scenePos().x();

            m_waveformItemList.at( orderPos )->setPos( currentScenePosX - distanceToMove, Ruler::HEIGHT );
            m_waveformItemList.at( orderPos )->setOrderPos( orderPos - numSelectedItems );
        }

        const int lastIndex = numSelectedItems - 1;

        for ( int i = lastIndex; i >= 0; i-- )
        {
            const int orderPos = oldOrderPositions.at( i );

            m_waveformItemList.at( orderPos )->setOrderPos( orderPos + numPlacesMoved );
            m_waveformItemList.move( orderPos, orderPos + numPlacesMoved );
        }
    }
}



void WaveGraphicsView::slideWaveformItemIntoPlace( const int orderPos )
{
    qreal newScenePosX = 0.0;

    for ( int i = 0; i < orderPos; i++ )
    {
        newScenePosX += m_waveformItemList.at( i )->rect().width();
    }

    m_waveformItemList.at( orderPos )->setPos( newScenePosX, Ruler::HEIGHT );

    updateLoopMarkerFrameNums();
}



void WaveGraphicsView::updateSlicePointFrameNum( FrameMarkerItem* const movedItem )
{
    const int oldFrameNum = movedItem->getFrameNum();

    if ( m_loopMarkerSnapMode == SNAP_SLICES_TO_MARKERS )
    {
        snapSlicePointToLoopMarker( dynamic_cast<SlicePointItem*>( movedItem ) );
    }
    else
    {
        const int newFrameNum = getFrameNum( movedItem->pos().x() );
        movedItem->setFrameNum( newFrameNum );
    }

    SharedSlicePointItem sharedSlicePoint;

    foreach ( SharedSlicePointItem item, m_slicePointItemList )
    {
        if ( item == movedItem )
        {
            sharedSlicePoint = item;
            break;
        }
    }

    emit slicePointOrderChanged( sharedSlicePoint, oldFrameNum, sharedSlicePoint->getFrameNum() );
}



void WaveGraphicsView::updateLoopMarkerFrameNum( FrameMarkerItem* const movedItem )
{
    LoopMarkerItem* const item = dynamic_cast<LoopMarkerItem*>( movedItem );

    setLoopMarkerFrameNum( item );

    if ( m_loopMarkerSnapMode == SNAP_MARKERS_TO_SLICES )
    {
        if ( m_waveformItemList.size() > 1 )
        {
            snapLoopMarkerToWaveform( item );
        }
        else
        {
            snapLoopMarkerToSlicePoint( item );
        }
    }

    emit loopMarkerPosChanged();
}



void WaveGraphicsView::removePlayhead()
{
    scene()->removeItem( m_playhead );
    scene()->update();
}



void WaveGraphicsView::relayMaxDetailLevelReached()
{
    if ( m_isViewZoomedIn )
    {
        emit maxDetailLevelReached();
    }
}
