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
//#include <QDebug>
#include "audioanalyser.h"



//==================================================================================================
// Public:

WaveGraphicsView::WaveGraphicsView( QWidget* parent ) :
    QGraphicsView( parent ),
    mLoopMarkerSnapMode( SNAP_OFF ),
    mInteractionMode( AUDITION_ITEMS ),
    mIsViewZoomedIn( false )
{
    // Set up view and scene
    setViewport( new QGLWidget( QGLFormat(QGL::SampleBuffers) ) );
    setRenderHint( QPainter::HighQualityAntialiasing, false );
    setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    setOptimizationFlags( DontSavePainterState | DontAdjustForAntialiasing );
    setBackgroundBrush( Qt::gray );
    setCacheMode( CacheBackground );

    setScene( new QGraphicsScene( 0.0, 0.0, 1024.0, 768.0 ) );

    // Set up playhead
    mPlayhead = new QGraphicsLineItem( 0.0, 0.0, 0.0, scene()->height() - 1 );
    mPlayhead->setPen( QColor( Qt::red ) );
    mPlayhead->setZValue( ZValues::PLAYHEAD );

    mTimer = new QTimeLine();
    mTimer->setFrameRange( 0, 100 );
    mTimer->setCurveShape( QTimeLine::LinearCurve );
    mTimer->setUpdateInterval( 17 );

    mAnimation = new QGraphicsItemAnimation;
    mAnimation->setItem( mPlayhead );
    mAnimation->setTimeLine( mTimer );

    QObject::connect( mTimer, SIGNAL( finished() ),
                      this, SLOT( removePlayhead() ) );

    QObject::connect( mTimer, SIGNAL( finished() ),
                      this, SIGNAL( playheadFinishedScrolling() ) );
}



SharedWaveformItem WaveGraphicsView::createWaveform( const SharedSampleBuffer sampleBuffer,
                                                     const SharedSampleHeader sampleHeader,
                                                     const qreal scenePosX,
                                                     const int orderPos,
                                                     qreal width )
{
    Q_ASSERT( sampleBuffer->getNumFrames() > 0 );

    mSampleHeader = sampleHeader;

    if ( width <= 0.0 )
    {
        width = scene()->width();
    }

    SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer, orderPos, width, scene()->height() ) );
    waveformItem->setPos( scenePosX, 0.0 );

    mWaveformItemList.insert( orderPos, waveformItem );

    connectWaveformToGraphicsView( waveformItem );

    scene()->addItem( waveformItem.data() );
    scene()->update();

    setInteractionMode( mInteractionMode );

    return waveformItem;
}



QList<SharedWaveformItem> WaveGraphicsView::createWaveforms( const QList<SharedSampleBuffer> sampleBufferList,
                                                             const SharedSampleHeader sampleHeader,
                                                             const qreal startScenePosX,
                                                             const int startOrderPos,
                                                             qreal totalWidth )
{
    mSampleHeader = sampleHeader;

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
        waveformItem->setPos( scenePosX, 0.0 );

        mWaveformItemList.insert( orderPos, waveformItem );
        newWaveformItems << waveformItem;

        connectWaveformToGraphicsView( waveformItem );

        scene()->addItem( waveformItem.data() );
        scene()->update();

        scenePosX += sliceWidth;
        orderPos++;
    }

    setInteractionMode( mInteractionMode );

    return newWaveformItems;
}



SharedWaveformItem WaveGraphicsView::joinWaveforms( const QList<int> orderPositions )
{
    QList<SharedWaveformItem> itemsToJoin;
    QList<SharedSampleBuffer> sampleBufferList;

    foreach( int orderPos, orderPositions )
    {
        itemsToJoin << mWaveformItemList.at( orderPos );
    }

    qreal width = 0.0;

    foreach ( SharedWaveformItem item, itemsToJoin )
    {
        scene()->removeItem( item.data() );
        mWaveformItemList.removeOne( item );

        sampleBufferList << item->getSampleBuffer();
        width += item->rect().width();
    }

    const SharedSampleBuffer sampleBuffer = SampleUtils::joinSampleBuffers( sampleBufferList );
    const int joinedItemOrderPos = itemsToJoin.first()->getOrderPos();

    SharedWaveformItem joinedItem = createWaveform( sampleBuffer,
                                                    mSampleHeader,
                                                    itemsToJoin.first()->scenePos().x(),
                                                    joinedItemOrderPos,
                                                    width );

    for ( int orderPos = joinedItemOrderPos + 1; orderPos < mWaveformItemList.size(); orderPos++ )
    {
        mWaveformItemList.at( orderPos )->setOrderPos( orderPos );
    }

    return joinedItem;
}



QList<SharedWaveformItem> WaveGraphicsView::splitWaveform( const int orderPos, const QList<int> slicePointFrameNums )
{
    SharedWaveformItem itemToSplit = mWaveformItemList.at( orderPos );

    scene()->removeItem( itemToSplit.data() );

    mWaveformItemList.removeOne( itemToSplit );

    QList<SharedSampleBuffer> sampleBufferList = SampleUtils::splitSampleBuffer( itemToSplit->getSampleBuffer(),
                                                                                 slicePointFrameNums );

    QList<SharedWaveformItem> waveformItemList = createWaveforms( sampleBufferList,
                                                                  mSampleHeader,
                                                                  itemToSplit->scenePos().x(),
                                                                  itemToSplit->getOrderPos(),
                                                                  itemToSplit->rect().width() );

    const int lastSplitItemOrderPos = waveformItemList.last()->getOrderPos();

    for ( int orderPos = lastSplitItemOrderPos + 1; orderPos < mWaveformItemList.size(); orderPos++ )
    {
        mWaveformItemList.at( orderPos )->setOrderPos( orderPos );
    }

    return waveformItemList;
}



void WaveGraphicsView::moveWaveforms( const QList<int> oldOrderPositions, const int numPlacesMoved )
{
    Q_ASSERT( ! mWaveformItemList.isEmpty() );

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
    for ( int i = firstOrderPos; i < mWaveformItemList.size(); i++ )
    {
        mWaveformItemList.at( i )->setOrderPos( i + numItemsToAdd );
    }

    // Add new waveform items to list
    foreach ( SharedWaveformItem item, waveformItems )
    {
        mWaveformItemList.insert( item->getOrderPos(), item );
    }

    // Resize and reposition all waveform items
    const int numFrames = getTotalNumFrames( mWaveformItemList );
    qreal scenePosX = 0.0;

    foreach ( SharedWaveformItem item, mWaveformItemList )
    {
        const qreal itemWidth = item->getSampleBuffer()->getNumFrames() * ( scene()->width() / numFrames );

        item->setRect( 0.0, 0.0, itemWidth, scene()->height() );
        item->setPos( scenePosX, 0.0 );

        scenePosX += itemWidth;
    }

    // Add waveform items to scene
    foreach ( SharedWaveformItem item, waveformItems )
    {
        scene()->addItem( item.data() );
    }
    scene()->update();

    setInteractionMode( mInteractionMode );

    // Reset loop markers
    if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL )
    {
        const int startFrame = 0;
        const int endFrame = getTotalNumFrames( mWaveformItemList ) - 1;

        mLoopMarkerLeft->setFrameNum( startFrame );
        mLoopMarkerRight->setFrameNum( endFrame );

        mLoopMarkerLeft->setPos( 0.0, 0.0 );
        mLoopMarkerRight->setPos( getScenePosX( endFrame ), 0.0 );
    }
}



QList<SharedWaveformItem> WaveGraphicsView::removeWaveforms( const QList<int> waveformOrderPositions )
{
    const int firstOrderPos = waveformOrderPositions.first();

    QList<SharedWaveformItem> removedWaveforms;

    // Remove waveform items from scene
    for ( int i = 0; i < waveformOrderPositions.size(); i++ )
    {
        SharedWaveformItem item = mWaveformItemList.at( firstOrderPos );
        mWaveformItemList.removeAt( firstOrderPos );

        scene()->removeItem( item.data() );
        removedWaveforms << item;
    }
    scene()->update();

    // If necessary, set new order positions and remove gap between remaining items
    if ( firstOrderPos < mWaveformItemList.size() )
    {
        qreal distanceToMove = 0.0;

        foreach ( SharedWaveformItem item, removedWaveforms )
        {
            distanceToMove += item->rect().width();
        }

        for ( int i = firstOrderPos; i < mWaveformItemList.size(); i++ )
        {
            mWaveformItemList.at( i )->setOrderPos( i );

            const qreal oldScenePosX = mWaveformItemList.at( i )->scenePos().x();
            mWaveformItemList.at( i )->setPos( oldScenePosX - distanceToMove, 0.0 );
        }
    }

    // Resize remaining waveform items
    qreal totalWidth = 0.0;

    foreach ( SharedWaveformItem item, mWaveformItemList )
    {
        totalWidth += item->rect().width();
    }

    resizeWaveformItems( scene()->width() / totalWidth );

    // Reset loop markers
    if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL )
    {
        const int startFrame = 0;
        const int endFrame = getTotalNumFrames( mWaveformItemList ) - 1;

        mLoopMarkerLeft->setFrameNum( startFrame );
        mLoopMarkerRight->setFrameNum( endFrame );

        mLoopMarkerLeft->setPos( 0.0, 0.0 );
        mLoopMarkerRight->setPos( getScenePosX( endFrame ), 0.0 );
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
    return mWaveformItemList.at( orderPos );
}



void WaveGraphicsView::redrawWaveforms()
{
    resizeWaveformItems( 1.0 );
    viewport()->update();
}



SharedSlicePointItem WaveGraphicsView::createSlicePoint( const int frameNum )
{
    const qreal scenePosX = getScenePosX( frameNum );

    SharedSlicePointItem sharedSlicePoint;

    SlicePointItem* slicePointItem = new SlicePointItem( scene()->height() - 1 );
    slicePointItem->setPos( scenePosX, 0.0 );
    slicePointItem->setFrameNum( frameNum );

    QTransform matrix;
    const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point remains correct width if view is scaled
    slicePointItem->setTransform( matrix );

    sharedSlicePoint = SharedSlicePointItem( slicePointItem );
    mSlicePointItemList.append( sharedSlicePoint );

    QObject::connect( slicePointItem, SIGNAL( scenePosChanged(SlicePointItem*const) ),
                      this, SLOT( updateSlicePointFrameNum(SlicePointItem*const) ) );

    scene()->addItem( slicePointItem );
    scene()->update();

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

    slicePoint.data()->setHeight( scene()->height() - 1 );
    slicePoint.data()->setPos( scenePosX, 0.0 );

    mSlicePointItemList.append( slicePoint );

    scene()->addItem( slicePoint.data() );
    scene()->update();
}



void WaveGraphicsView::removeSlicePoint( const SharedSlicePointItem slicePointItem )
{
    scene()->removeItem( slicePointItem.data() );
    scene()->update();

    mSlicePointItemList.removeOne( slicePointItem );
}



void WaveGraphicsView::moveSlicePoint( const SharedSlicePointItem slicePointItem, const int newFrameNum )
{
    const qreal newScenePosX = getScenePosX( newFrameNum );

    slicePointItem->setFrameNum( newFrameNum );
    slicePointItem->setPos( newScenePosX, 0.0 );
}



void WaveGraphicsView::hideSlicePoints()
{
    foreach ( SharedSlicePointItem item, mSlicePointItemList )
    {
        item->setVisible( false );
    }
}



void WaveGraphicsView::showSlicePoints()
{
    foreach ( SharedSlicePointItem item, mSlicePointItemList )
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

            foreach ( SharedSlicePointItem sharedSlicePointItem, mSlicePointItemList )
            {
                if ( sharedSlicePointItem == slicePointItem )
                {
                    selectedSlicePointItem = sharedSlicePointItem;
                }
            }
        }
    }

    return selectedSlicePointItem;
}



QList<int> WaveGraphicsView::getSlicePointFrameNums() const
{
    QList<int> slicePointFrameNums;

    foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
    {
        slicePointFrameNums.append( slicePointItem->getFrameNum() );
    }

    qSort( slicePointFrameNums );

    const int totalNumFrames = getTotalNumFrames( mWaveformItemList );
    const int minFramesBetweenSlicePoints = roundToInt( mSampleHeader->sampleRate * AudioAnalyser::MIN_INTER_ONSET_SECS );

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
    if ( mLoopMarkerLeft == NULL && mLoopMarkerRight == NULL )
    {
        createLoopMarkers();
    }

    mLoopMarkerLeft->setVisible( true );
    mLoopMarkerRight->setVisible( true );
}



void WaveGraphicsView::hideLoopMarkers()
{
    mLoopMarkerLeft->setVisible( false );
    mLoopMarkerRight->setVisible( false );
}



void WaveGraphicsView::getSampleRangesBetweenLoopMarkers( int& firstOrderPos, QList<SharedSampleRange>& sampleRanges ) const
{
    if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL)
    {
        const int leftMarkerFrameNum = getRelativeLoopMarkerFrameNum( mLoopMarkerLeft );
        const int rightMarkerFrameNum = getRelativeLoopMarkerFrameNum( mLoopMarkerRight );

        const int leftWaveformOrderPos = getWaveformUnderLoopMarker( mLoopMarkerLeft )->getOrderPos();
        const int rightWaveformOrderPos = getWaveformUnderLoopMarker( mLoopMarkerRight )->getOrderPos();

        const int minNumFrames = roundToInt( mSampleHeader->sampleRate * AudioAnalyser::MIN_INTER_ONSET_SECS );

        bool isFirstOrderPos = true;

        for ( int orderPos = leftWaveformOrderPos; orderPos <= rightWaveformOrderPos; orderPos++ )
        {
            int startFrame = 0;
            int numFrames = mWaveformItemList.at( orderPos )->getSampleBuffer()->getNumFrames();

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

    if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL )
    {
        numFrames = mLoopMarkerRight->getFrameNum() - mLoopMarkerLeft->getFrameNum();
    }

    return numFrames;
}



void WaveGraphicsView::selectNone()
{
    foreach ( SharedSlicePointItem item, mSlicePointItemList )
    {
        item->setSelected( false );
    }

    foreach ( SharedWaveformItem item, mWaveformItemList )
    {
        item->setSelected( false );
    }
}



void WaveGraphicsView::selectAll()
{
    foreach ( SharedWaveformItem item, mWaveformItemList )
    {
        item->setSelected( true );
    }
}



void WaveGraphicsView::startPlayhead( const bool isLoopingDesired, const qreal stretchRatio )
{
    const qreal sampleRate = mSampleHeader->sampleRate;

    if ( sampleRate > 0.0 )
    {
        int numFrames = getTotalNumFrames( mWaveformItemList );

        qreal startPosX = 0.0;
        qreal endPosX   = scene()->width() - 1;

        if ( mLoopMarkerLeft != NULL && mLoopMarkerLeft->isVisible() )
        {
            numFrames = getFrameNum( mLoopMarkerRight->scenePos().x() - mLoopMarkerLeft->scenePos().x() );
            startPosX = mLoopMarkerLeft->scenePos().x();
            endPosX   = mLoopMarkerRight->scenePos().x();
        }

        const int millis = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );

        if ( isPlayheadScrolling() )
        {
            stopPlayhead();
        }

        mAnimation->setPosAt( 0.0, QPointF( startPosX, 0.0 ) );
        mAnimation->setPosAt( 1.0, QPointF( endPosX,   0.0 ) );

        mPlayhead->setLine( 0.0, 0.0, 0.0, scene()->height() - 1 );
        mPlayhead->setVisible( true );
        scene()->addItem( mPlayhead );

        if ( isLoopingDesired )
        {
            mTimer->setLoopCount( 0 );
        }
        else
        {
            mTimer->setLoopCount( 1 );
        }
        mTimer->setDuration( millis );
        mTimer->start();
    }
}



void WaveGraphicsView::startPlayhead( const qreal startPosX, const qreal endPosX, const int numFrames, const qreal stretchRatio )
{
    const qreal sampleRate = mSampleHeader->sampleRate;

    if ( sampleRate > 0.0 )
    {
        const int millis = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );

        if ( isPlayheadScrolling() )
        {
            stopPlayhead();
        }

        mAnimation->setPosAt( 0.0, QPointF( startPosX, 0.0 ) );
        mAnimation->setPosAt( 1.0, QPointF( endPosX,   0.0 ) );

        mPlayhead->setLine( 0.0, 0.0, 0.0, scene()->height() - 1 );
        mPlayhead->setVisible( true );
        scene()->addItem( mPlayhead );

        mTimer->setLoopCount( 1 );
        mTimer->setDuration( millis );
        mTimer->start();
    }
}



void WaveGraphicsView::stopPlayhead()
{
    if ( isPlayheadScrolling() )
    {
        mTimer->stop();
        removePlayhead();
    }
}



void WaveGraphicsView::setPlayheadLooping( const bool isLoopingDesired )
{
    if ( isLoopingDesired )
    {
        mTimer->setLoopCount( 0 );
    }
    else
    {
        mTimer->setLoopCount( 1 );
    }
}



void WaveGraphicsView::updatePlayheadSpeed( const qreal stretchRatio )
{
    if ( isPlayheadScrolling() )
    {
        mPlayhead->setVisible( false );

        mTimer->stop();

        const qreal sampleRate = mSampleHeader->sampleRate;
        int numFrames = 0;

        if ( mLoopMarkerLeft != NULL && mLoopMarkerLeft->isVisible() )
        {
            numFrames = getFrameNum( mLoopMarkerRight->scenePos().x() - mLoopMarkerLeft->scenePos().x() );
        }
        else
        {
            numFrames = getTotalNumFrames( mWaveformItemList );
        }

        const int newDuration = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );
//        const int newTime = roundToInt( mTimer->currentTime() * stretchRatio );

        mTimer->setDuration( newDuration );
//        mTimer->setCurrentTime( newTime );

        mTimer->resume();
    }
}



void WaveGraphicsView::clearAll()
{
    foreach ( QGraphicsItem* item, items() )
    {
        scene()->removeItem( item );
    }
    scene()->update();

    mWaveformItemList.clear();
    mSlicePointItemList.clear();
    mLoopMarkerLeft = NULL;
    mLoopMarkerRight = NULL;
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

    mWaveformItemList.clear();
}



qreal WaveGraphicsView::getScenePosX( const int frameNum ) const
{
    const int numFrames = getTotalNumFrames( mWaveformItemList );

    qreal scenePosX = frameNum * ( scene()->width() / numFrames );

    if ( scenePosX < 0.0)
        scenePosX = 0.0;

    if ( scenePosX >= scene()->width() )
        scenePosX = scene()->width() - 1;

    return scenePosX;
}



int WaveGraphicsView::getFrameNum( qreal scenePosX ) const
{
    const int numFrames = getTotalNumFrames( mWaveformItemList );

    int frameNum = roundToInt( scenePosX / ( scene()->width() / numFrames ) );

    if ( frameNum < 0 )
        frameNum = 0;

    if ( frameNum >= numFrames )
        frameNum = numFrames - 1;

    return frameNum;
}



void WaveGraphicsView::zoomIn()
{
    mIsViewZoomedIn = true;

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
        mIsViewZoomedIn = false;
    }
}



void WaveGraphicsView::zoomOriginal()
{
    mIsViewZoomedIn = false;

    resetTransform();
    scaleItems( 1.0 );
}



void WaveGraphicsView::setInteractionMode( const InteractionMode mode )
{
    switch ( mode )
    {
    case MOVE_ITEMS:
        foreach ( SharedWaveformItem item, mWaveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, true );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        setDragMode( NoDrag );
        break;
    case SELECT_ITEMS:
        foreach ( SharedWaveformItem item, mWaveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        setDragMode( RubberBandDrag );
        break;
    case AUDITION_ITEMS:
        foreach ( SharedWaveformItem item, mWaveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, false );
        }
        setDragMode( NoDrag );
        break;
    default:
        break;
    }

    mInteractionMode = mode;
}



//==================================================================================================
// Protected:

void WaveGraphicsView::resizeEvent ( QResizeEvent* event )
{
    scene()->setSceneRect( 0.0, 0.0, event->size().width(), event->size().height() );

    const qreal scaleFactorX = scene()->width() / event->oldSize().width();
//    const qreal scaleFactorY = scene()->height() / event->oldSize().height();

    resizeWaveformItems( scaleFactorX );
    resizeSlicePointItems( scaleFactorX );
    resizePlayhead();
    resizeLoopMarkers( scaleFactorX );

    QGraphicsView::resizeEvent( event );
}



//==================================================================================================
// Private:

void WaveGraphicsView::resizeWaveformItems( const qreal scaleFactorX )
{
    if ( ! mWaveformItemList.isEmpty() )
    {
        foreach ( SharedWaveformItem waveformItem, mWaveformItemList )
        {
            const qreal newWidth = waveformItem->rect().width() * scaleFactorX;
            waveformItem->setRect( 0.0, 0.0, newWidth, scene()->height() );

            const qreal newX = waveformItem->scenePos().x() * scaleFactorX;
            waveformItem->setPos( newX, 0.0 );
        }
    }
}



void WaveGraphicsView::resizeSlicePointItems( const qreal scaleFactorX )
{
    if ( ! mSlicePointItemList.isEmpty() )
    {
        foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
        {
            slicePointItem->setHeight( scene()->height() - 1 );
            const qreal newX = slicePointItem->scenePos().x() * scaleFactorX;
            slicePointItem->setPos( newX, 0.0 );
        }
    }
}



void WaveGraphicsView::resizePlayhead()
{
    if ( mTimer->state() == QTimeLine::Running )
    {
        mTimer->stop();

        mAnimation->clear();
        mAnimation->setPosAt( 0.0, QPointF( 0.0, 0.0 ) );
        mAnimation->setPosAt( 1.0, QPointF( scene()->width() - 1, 0.0 ) );

        mPlayhead->setLine( 0.0, 0.0, 0.0, scene()->height() - 1 );

        mTimer->resume();
    }
}



void WaveGraphicsView::resizeLoopMarkers( const qreal scaleFactorX )
{
    if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL )
    {
        mLoopMarkerLeft->setHeight( scene()->height() - 1 );
        mLoopMarkerRight->setHeight( scene()->height() - 1 );
        {
            const qreal newX = mLoopMarkerLeft->scenePos().x() * scaleFactorX;
            mLoopMarkerLeft->setPos( newX, 0.0 );
        }
        {
            const qreal newX = mLoopMarkerRight->scenePos().x() * scaleFactorX;
            mLoopMarkerRight->setPos( newX, 0.0 );
        }
    }
}



void WaveGraphicsView::scaleItems( const qreal scaleFactorX )
{
    if ( scaleFactorX > 0.0 )
    {
        QTransform matrix;
        matrix.scale( 1.0 / scaleFactorX, 1.0 ); // Items remain same width when view is scaled

        if ( ! mSlicePointItemList.isEmpty() )
        {
            foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
            {
                slicePointItem->setTransform( matrix );
            }
        }

        if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL )
        {
            mLoopMarkerLeft->setTransform( matrix );
            mLoopMarkerRight->setTransform( matrix );
        }
    }
}



void WaveGraphicsView::createLoopMarkers()
{
    mLoopMarkerLeft = new LoopMarkerItem( LoopMarkerItem::LEFT_MARKER, scene()->height() - 1 );
    mLoopMarkerRight = new LoopMarkerItem( LoopMarkerItem::RIGHT_MARKER, scene()->height() - 1 );

    const int startFrame = 0;
    const int endFrame = getTotalNumFrames( mWaveformItemList ) - 1;

    mLoopMarkerLeft->setFrameNum( startFrame );
    mLoopMarkerRight->setFrameNum( endFrame );

    mLoopMarkerLeft->setPos( 0.0, 0.0 );
    mLoopMarkerRight->setPos( getScenePosX( endFrame ), 0.0 );

    QTransform matrix;
    const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // loop marker remains correct width if view is scaled
    mLoopMarkerLeft->setTransform( matrix );
    mLoopMarkerRight->setTransform( matrix );

    mLoopMarkerLeft->setZValue( ZValues::LOOP_MARKER );
    mLoopMarkerRight->setZValue( ZValues::LOOP_MARKER );

    QObject::connect( mLoopMarkerLeft, SIGNAL( scenePosChanged(LoopMarkerItem*const) ),
                      this, SLOT( updateLoopMarkerFrameNum(LoopMarkerItem*const) ) );

    QObject::connect( mLoopMarkerRight, SIGNAL( scenePosChanged(LoopMarkerItem*const) ),
                      this, SLOT( updateLoopMarkerFrameNum(LoopMarkerItem*const) ) );

    scene()->addItem( mLoopMarkerLeft );
    scene()->addItem( mLoopMarkerRight );
    scene()->update();

    updateLoopMarkerFrameNum( mLoopMarkerLeft );
    updateLoopMarkerFrameNum( mLoopMarkerRight );
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

        foreach ( SharedWaveformItem item, mWaveformItemList )
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

        foreach ( SharedWaveformItem item, mWaveformItemList )
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
    if ( mLoopMarkerLeft != NULL && mLoopMarkerRight != NULL )
    {
        setLoopMarkerFrameNum( mLoopMarkerLeft );
        setLoopMarkerFrameNum( mLoopMarkerRight );

        emit loopMarkerPosChanged();
    }
}



void WaveGraphicsView::snapLoopMarkerToSlicePoint( LoopMarkerItem* const loopMarker )
{
    if ( loopMarker != NULL )
    {
        const int oldFrameNum = loopMarker->getFrameNum();
        const int minFrameNum = 0;
        const int maxFrameNum = getTotalNumFrames( mWaveformItemList ) - 1;

        QList<int> frameNumList;

        frameNumList << minFrameNum;

        foreach ( SharedSlicePointItem slicePoint, mSlicePointItemList )
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
        loopMarker->setPos( getScenePosX( newFrameNum ), 0.0 );
    }
}



void WaveGraphicsView::snapLoopMarkerToWaveform( LoopMarkerItem* const loopMarker )
{
    if ( loopMarker != NULL )
    {
        const int oldFrameNum = loopMarker->getFrameNum();

        QList<int> frameNumList;

        int frameNum = 0;

        for ( int i = 0; i < mWaveformItemList.size(); i++ )
        {
            frameNumList << frameNum;
            frameNum += mWaveformItemList.at( i )->getSampleBuffer()->getNumFrames();
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
        loopMarker->setPos( getScenePosX( newFrameNum ), 0.0 );
    }
}



void WaveGraphicsView::snapSlicePointToLoopMarker( SlicePointItem* const slicePoint )
{
    if ( slicePoint != NULL && mLoopMarkerLeft != NULL && mLoopMarkerLeft->isVisible() )
    {
        const qreal snapThreshold = 30.0;

        qreal scenePosX = slicePoint->scenePos().x();

        if ( qAbs( scenePosX - mLoopMarkerLeft->scenePos().x() ) <= snapThreshold )
        {
            scenePosX = mLoopMarkerLeft->scenePos().x();
        }
        else if ( qAbs( scenePosX - mLoopMarkerRight->scenePos().x() ) <= snapThreshold )
        {
            scenePosX = mLoopMarkerRight->scenePos().x();
        }

        slicePoint->setPos( scenePosX, 0.0 );
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
        distanceToMove += mWaveformItemList.at( orderPos )->rect().width();
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
            const qreal currentScenePosX = mWaveformItemList.at( orderPos )->scenePos().x();

            mWaveformItemList.at( orderPos )->setPos( currentScenePosX + distanceToMove, 0.0 );
            mWaveformItemList.at( orderPos )->setOrderPos( orderPos + numSelectedItems );
        }

        for ( int i = 0; i < numSelectedItems; i++ )
        {
            const int orderPos = oldOrderPositions.at( i );

            mWaveformItemList.at( orderPos )->setOrderPos( orderPos + numPlacesMoved );
            mWaveformItemList.move( orderPos, orderPos + numPlacesMoved );
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
            const qreal currentScenePosX = mWaveformItemList.at( orderPos )->scenePos().x();

            mWaveformItemList.at( orderPos )->setPos( currentScenePosX - distanceToMove, 0.0 );
            mWaveformItemList.at( orderPos )->setOrderPos( orderPos - numSelectedItems );
        }

        const int lastIndex = numSelectedItems - 1;

        for ( int i = lastIndex; i >= 0; i-- )
        {
            const int orderPos = oldOrderPositions.at( i );

            mWaveformItemList.at( orderPos )->setOrderPos( orderPos + numPlacesMoved );
            mWaveformItemList.move( orderPos, orderPos + numPlacesMoved );
        }
    }
}



void WaveGraphicsView::slideWaveformItemIntoPlace( const int orderPos )
{
    qreal newScenePosX = 0.0;

    for ( int i = 0; i < orderPos; i++ )
    {
        newScenePosX += mWaveformItemList.at( i )->rect().width();
    }

    mWaveformItemList.at( orderPos )->setPos( newScenePosX, 0.0 );

    updateLoopMarkerFrameNums();
}



void WaveGraphicsView::updateSlicePointFrameNum( SlicePointItem* const movedItem )
{
    if ( mLoopMarkerSnapMode == SNAP_SLICES_TO_MARKERS )
    {
        snapSlicePointToLoopMarker( movedItem );
    }

    SharedSlicePointItem sharedSlicePoint;

    foreach ( SharedSlicePointItem item, mSlicePointItemList )
    {
        if ( item == movedItem )
        {
            sharedSlicePoint = item;
            break;
        }
    }

    const int oldFrameNum = sharedSlicePoint->getFrameNum();
    const int newFrameNum = getFrameNum( sharedSlicePoint->pos().x() );
    sharedSlicePoint->setFrameNum( newFrameNum );

    emit slicePointOrderChanged( sharedSlicePoint, oldFrameNum, newFrameNum );
}



void WaveGraphicsView::updateLoopMarkerFrameNum( LoopMarkerItem* const movedItem )
{
    setLoopMarkerFrameNum( movedItem );

    if ( mLoopMarkerSnapMode == SNAP_MARKERS_TO_SLICES )
    {
        if ( mWaveformItemList.size() > 1 )
        {
            snapLoopMarkerToWaveform( movedItem );
        }
        else
        {
            snapLoopMarkerToSlicePoint( movedItem );
        }
    }

    emit loopMarkerPosChanged();
}



void WaveGraphicsView::removePlayhead()
{
    scene()->removeItem( mPlayhead );
    scene()->update();
}



void WaveGraphicsView::relayMaxDetailLevelReached()
{
    if ( mIsViewZoomedIn )
    {
        emit maxDetailLevelReached();
    }
}
