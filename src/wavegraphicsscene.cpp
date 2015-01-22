/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#include "wavegraphicsscene.h"
#include "audioanalyser.h"
#include "globals.h"


//==================================================================================================
// Public:

WaveGraphicsScene::WaveGraphicsScene( const qreal x, const qreal y, const qreal width, const qreal height, QObject* parent ) :
    QGraphicsScene( x, y, width, height, parent ),
    m_loopMarkerSnapMode( SNAP_OFF )
{
    createRuler();

    // Set up playhead
    m_playhead = new QGraphicsLineItem( 0.0, 0.0, 0.0, height - 1 );
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



WaveGraphicsView* WaveGraphicsScene::getView() const
{
    return static_cast<WaveGraphicsView*>( views().first() );
}



SharedWaveformItem WaveGraphicsScene::createWaveform( const SharedSampleBuffer sampleBuffer,
                                                      const SharedSampleHeader sampleHeader,
                                                      const qreal scenePosX,
                                                      const int orderPos,
                                                      qreal width )
{
    Q_ASSERT( sampleBuffer->getNumFrames() > 0 );

    m_sampleHeader = sampleHeader;

    if ( width <= 0.0 )
    {
        width = this->width();
    }

    SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer, orderPos, width, height() ) );
    waveformItem->setPos( scenePosX, Ruler::HEIGHT );

    m_waveformItemList.insert( orderPos, waveformItem );

    connectWaveform( waveformItem );

    addItem( waveformItem.data() );
    update();

    WaveGraphicsView* view = getView();

    view->setInteractionMode( view->getInteractionMode() );

    return waveformItem;
}



QList<SharedWaveformItem> WaveGraphicsScene::createWaveforms( const QList<SharedSampleBuffer> sampleBufferList,
                                                              const SharedSampleHeader sampleHeader,
                                                              const qreal startScenePosX,
                                                              const int startOrderPos,
                                                              qreal totalWidth )
{
    m_sampleHeader = sampleHeader;

    const int totalNumFrames = SampleUtils::getTotalNumFrames( sampleBufferList );

    if ( totalWidth <= 0.0 )
    {
        totalWidth = width();
    }

    qreal scenePosX = startScenePosX;
    int orderPos = startOrderPos;

    QList<SharedWaveformItem> newWaveformItems;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        const qreal sliceWidth = sampleBuffer->getNumFrames() * ( totalWidth / totalNumFrames );

        SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer,
                                                           orderPos,
                                                           sliceWidth,
                                                           height() ) );
        waveformItem->setPos( scenePosX, Ruler::HEIGHT );

        m_waveformItemList.insert( orderPos, waveformItem );
        newWaveformItems << waveformItem;

        connectWaveform( waveformItem );

        addItem( waveformItem.data() );
        update();

        scenePosX += sliceWidth;
        orderPos++;
    }

    WaveGraphicsView* view = getView();

    view->setInteractionMode( view->getInteractionMode() );

    return newWaveformItems;
}



void WaveGraphicsScene::moveWaveforms( const QList<int> oldOrderPositions, const int numPlacesMoved )
{
    Q_ASSERT( ! m_waveformItemList.isEmpty() );

    reorderWaveformItems( oldOrderPositions, numPlacesMoved );

    foreach ( int orderPos, oldOrderPositions )
    {
        const int newOrderPos = orderPos + numPlacesMoved;
        slideWaveformItemIntoPlace( newOrderPos );
    }
}



void WaveGraphicsScene::insertWaveforms( const QList<SharedWaveformItem> waveformItems )
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
    const int totalNumFrames = getTotalNumFrames( m_waveformItemList );
    qreal scenePosX = 0.0;

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        const qreal itemWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames );

        item->setRect( 0.0, 0.0, itemWidth, height() );
        item->setPos( scenePosX, Ruler::HEIGHT );

        scenePosX += itemWidth;
    }

    // Add waveform items to scene
    foreach ( SharedWaveformItem item, waveformItems )
    {
        addItem( item.data() );
    }
    update();

    WaveGraphicsView* view = getView();

    view->setInteractionMode( view->getInteractionMode() );

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



QList<SharedWaveformItem> WaveGraphicsScene::removeWaveforms( const QList<int> waveformOrderPositions )
{
    const int startOrderPos = waveformOrderPositions.first();

    QList<SharedWaveformItem> removedWaveforms;

    // Remove waveform items from scene
    for ( int i = 0; i < waveformOrderPositions.size(); i++ )
    {
        SharedWaveformItem item = m_waveformItemList.at( startOrderPos );

        m_waveformItemList.removeAt( startOrderPos );
        removeItem( item.data() );

        removedWaveforms << item;
    }
    update();

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

    resizeWaveformItems( width() / totalWidth );

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



QList<WaveformItem*> WaveGraphicsScene::getSelectedWaveforms() const
{
    QList<WaveformItem*> selectedItems;

    foreach ( QGraphicsItem* item, this->selectedItems() )
    {
        if ( item->type() == WaveformItem::Type )
        {
            selectedItems << qgraphicsitem_cast<WaveformItem*>( item );
        }
    }
    qSort( selectedItems.begin(), selectedItems.end(), WaveformItem::isLessThanOrderPos );

    return selectedItems;
}



QList<int> WaveGraphicsScene::getSelectedWaveformsOrderPositions() const
{
    const QList<WaveformItem*> selectedItems = getSelectedWaveforms();
    QList<int> orderPositions;

    foreach ( WaveformItem* item, selectedItems )
    {
        orderPositions << item->getOrderPos();
    }

    return orderPositions;
}



void WaveGraphicsScene::resizeWaveforms( const QList<int> orderPositions, const QList<qreal> scaleFactorX )
{
    if ( ! m_waveformItemList.isEmpty() )
    {
        const int totalNumFrames = getTotalNumFrames( m_waveformItemList );

        for ( int i = 0; i < orderPositions.size(); i++ )
        {
            SharedWaveformItem item = m_waveformItemList.at( orderPositions.at( i ) );

            const qreal origWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames );
            const qreal newWidth = origWidth * scaleFactorX.at( i );

            item->setRect( 0.0, 0.0, newWidth, height() );

            slideWaveformItemIntoPlace( orderPositions.at( i ) );
        }
    }
}



QList<qreal> WaveGraphicsScene::getWaveformScaleFactors( const QList<int> orderPositions ) const
{
    QList<qreal> scaleFactorList;

    if ( ! m_waveformItemList.isEmpty() )
    {
        const int totalNumFrames = getTotalNumFrames( m_waveformItemList );

        foreach ( int orderPos, orderPositions )
        {
            SharedWaveformItem item = m_waveformItemList.at( orderPos );

            const qreal origWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames );

            scaleFactorList << item->rect().width() / origWidth;
        }
    }

    return scaleFactorList;
}



SharedSlicePointItem WaveGraphicsScene::createSlicePoint( const int frameNum, const bool canBeMovedPastOtherSlicePoints )
{
    const qreal scenePosX = getScenePosX( frameNum );

    SlicePointItem* item = new SlicePointItem( height() - Ruler::HEIGHT - 1, canBeMovedPastOtherSlicePoints );
    item->setPos( scenePosX, Ruler::HEIGHT );
    item->setFrameNum( frameNum );

    QTransform matrix;
    const qreal currentScaleFactor = views().first()->transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point handle is set to correct width even if view is scaled
    item->setTransform( matrix );

    QObject::connect( item, SIGNAL( scenePosChanged(FrameMarkerItem*) ),
                      this, SLOT( updateSlicePointFrameNum(FrameMarkerItem*) ) );

    addItem( item );
    update();

    SharedSlicePointItem sharedSlicePoint = SharedSlicePointItem( item );
    m_slicePointItemList.append( sharedSlicePoint );

    return sharedSlicePoint;
}



void WaveGraphicsScene::addSlicePoint( const SharedSlicePointItem slicePoint )
{
    const int slicePointFrameNum = slicePoint.data()->getFrameNum();
    const qreal scenePosX = getScenePosX( slicePointFrameNum );

    QTransform matrix;
    const qreal currentScaleFactor = views().first()->transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point remains same width when view is scaled
    slicePoint.data()->setTransform( matrix );

    slicePoint.data()->setHeight( height() - Ruler::HEIGHT - 1 );
    slicePoint.data()->setPos( scenePosX, Ruler::HEIGHT );

    m_slicePointItemList.append( slicePoint );

    addItem( slicePoint.data() );
    update();
}



void WaveGraphicsScene::removeSlicePoint( const SharedSlicePointItem slicePointItem )
{
    removeItem( slicePointItem.data() );
    update();

    m_slicePointItemList.removeOne( slicePointItem );
}



void WaveGraphicsScene::moveSlicePoint( const SharedSlicePointItem slicePointItem, const int newFrameNum )
{
    const qreal newScenePosX = getScenePosX( newFrameNum );

    slicePointItem->setFrameNum( newFrameNum );
    slicePointItem->setPos( newScenePosX, Ruler::HEIGHT );
}



SharedSlicePointItem WaveGraphicsScene::getSelectedSlicePoint()
{
    const QList<QGraphicsItem*> selectedItems = this->selectedItems();
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



QList<int> WaveGraphicsScene::getSlicePointFrameNums() const
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



void WaveGraphicsScene::showLoopMarkers()
{
    if ( m_loopMarkerLeft == NULL && m_loopMarkerRight == NULL )
    {
        createLoopMarkers();
    }

    m_loopMarkerLeft->setVisible( true );
    m_loopMarkerRight->setVisible( true );
}



void WaveGraphicsScene::hideLoopMarkers()
{
    m_loopMarkerLeft->setVisible( false );
    m_loopMarkerRight->setVisible( false );
}



void WaveGraphicsScene::getSampleRangesBetweenLoopMarkers( int& firstOrderPos, QList<SharedSampleRange>& sampleRanges ) const
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



int WaveGraphicsScene::getNumFramesBetweenLoopMarkers() const
{
    int numFrames = 0;

    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        numFrames = m_loopMarkerRight->getFrameNum() - m_loopMarkerLeft->getFrameNum();
    }

    return numFrames;
}



void WaveGraphicsScene::selectNone()
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



void WaveGraphicsScene::selectAll()
{
    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        item->setSelected( true );
    }
}



void WaveGraphicsScene::startPlayhead( const bool isLoopingDesired, const qreal stretchRatio )
{
    const qreal sampleRate = m_sampleHeader->sampleRate;

    if ( sampleRate > 0.0 )
    {
        int numFrames = getTotalNumFrames( m_waveformItemList );

        qreal startPosX = 0.0;
        qreal endPosX   = width() - 1;

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

        m_playhead->setLine( 0.0, 0.0, 0.0, height() - 1 );
        m_playhead->setVisible( true );
        addItem( m_playhead );

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



void WaveGraphicsScene::startPlayhead( const qreal startPosX, const qreal endPosX, const int numFrames, const qreal stretchRatio )
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

        m_playhead->setLine( 0.0, 0.0, 0.0, height() - 1 );
        m_playhead->setVisible( true );
        addItem( m_playhead );

        m_timer->setLoopCount( 1 );
        m_timer->setDuration( millis );
        m_timer->start();
    }
}



void WaveGraphicsScene::stopPlayhead()
{
    if ( isPlayheadScrolling() )
    {
        m_timer->stop();
        removePlayhead();
    }
}



void WaveGraphicsScene::setPlayheadLooping( const bool isLoopingDesired )
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



void WaveGraphicsScene::updatePlayheadSpeed( const qreal stretchRatio )
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



void WaveGraphicsScene::clearAll()
{
    foreach ( QGraphicsItem* item, items() )
    {
        removeItem( item );
    }
    update();

    m_waveformItemList.clear();
    m_slicePointItemList.clear();
    m_rulerMarksList.clear();
    m_loopMarkerLeft = NULL;
    m_loopMarkerRight = NULL;

    createRuler();
}



void WaveGraphicsScene::clearWaveform()
{
    foreach ( QGraphicsItem* item, items() )
    {
        if ( item->type() == WaveformItem::Type )
        {
            removeItem( item );
        }
    }
    update();

    m_waveformItemList.clear();
}



qreal WaveGraphicsScene::getScenePosX( const int frameNum ) const
{
    const int numFrames = getTotalNumFrames( m_waveformItemList );

    qreal scenePosX = frameNum * ( width() / numFrames );

    if ( scenePosX < 0.0)
        scenePosX = 0.0;

    if ( scenePosX >= width() )
        scenePosX = width() - 1;

    return scenePosX;
}



int WaveGraphicsScene::getFrameNum( qreal scenePosX ) const
{
    const int numFrames = getTotalNumFrames( m_waveformItemList );

    int frameNum = roundToInt( scenePosX / ( width() / numFrames ) );

    if ( frameNum < 0 )
        frameNum = 0;

    if ( frameNum >= numFrames )
        frameNum = numFrames - 1;

    return frameNum;
}



void WaveGraphicsScene::setBpmRulerMarks( const qreal bpm, const int timeSigNumerator )
{
    if ( bpm > 0.0 && timeSigNumerator > 0 )
    {
        foreach ( SharedGraphicsItem item, m_rulerMarksList )
        {
            removeItem( item.data() );
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
                QGraphicsSimpleTextItem* textItem = addSimpleText( QString::number( bar ) );
                textItem->setPos( getScenePosX( frameNum ), 1.0 );
                textItem->setBrush( Qt::white );
                textItem->setZValue( 1 );
                m_rulerMarksList.append( SharedGraphicsItem( textItem ) );
                bar++;
            }
            else
            {
                QGraphicsLineItem* lineItem = addLine( 0.0, 0.0, 0.0, Ruler::HEIGHT - 5.0 );
                lineItem->setPos( getScenePosX( frameNum ), 2.0 );
                lineItem->setPen( QPen( Qt::white ) );
                lineItem->setZValue( 1 );
                m_rulerMarksList.append( SharedGraphicsItem( lineItem ) );
            }
        }
    }
}



void WaveGraphicsScene::resizeWaveformItems( const qreal scaleFactorX )
{
    foreach ( SharedWaveformItem waveformItem, m_waveformItemList )
    {
        const qreal newWidth = waveformItem->rect().width() * scaleFactorX;
        waveformItem->setRect( 0.0, 0.0, newWidth, height() );

        const qreal newX = waveformItem->scenePos().x() * scaleFactorX;
        waveformItem->setPos( newX, Ruler::HEIGHT );
    }
}



void WaveGraphicsScene::resizeSlicePointItems( const qreal scaleFactorX )
{
    foreach ( SharedSlicePointItem slicePointItem, m_slicePointItemList )
    {
        slicePointItem->setHeight( height() - Ruler::HEIGHT - 1 );

        const bool canBeMoved = slicePointItem->canBeMovedPastOtherSlicePoints();
        const qreal newX = slicePointItem->scenePos().x() * scaleFactorX;

        slicePointItem->setMovePastOtherSlicePoints( true );
        slicePointItem->setPos( newX, Ruler::HEIGHT );
        slicePointItem->setMovePastOtherSlicePoints( canBeMoved );
    }
}



void WaveGraphicsScene::resizePlayhead()
{
    if ( m_timer->state() == QTimeLine::Running )
    {
        m_timer->stop();

        m_animation->clear();
        m_animation->setPosAt( 0.0, QPointF( 0.0, Ruler::HEIGHT ) );
        m_animation->setPosAt( 1.0, QPointF( width() - 1, Ruler::HEIGHT ) );

        m_playhead->setLine( 0.0, 0.0, 0.0, height() - Ruler::HEIGHT - 1 );

        m_timer->resume();
    }
}



void WaveGraphicsScene::resizeLoopMarkers( const qreal scaleFactorX )
{
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        m_loopMarkerLeft->setHeight( height() - Ruler::HEIGHT - 1 );
        m_loopMarkerRight->setHeight( height() - Ruler::HEIGHT - 1 );
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



void WaveGraphicsScene::resizeRuler( const qreal scaleFactorX )
{
    m_rulerBackground->setRect( 0.0, 0.0, width(), Ruler::HEIGHT );

    foreach ( SharedGraphicsItem item, m_rulerMarksList )
    {
        const qreal newX = item->scenePos().x() * scaleFactorX;
        item->setPos( newX, 1.0 );
    }
}



void WaveGraphicsScene::scaleItems( const qreal scaleFactorX )
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



//==================================================================================================
// Private:

void WaveGraphicsScene::createLoopMarkers()
{
    m_loopMarkerLeft = new LoopMarkerItem( LoopMarkerItem::LEFT_MARKER, height() - Ruler::HEIGHT - 1 );

    m_loopMarkerRight = new LoopMarkerItem( LoopMarkerItem::RIGHT_MARKER, height() - Ruler::HEIGHT - 1 );

    const int startFrame = 0;
    const int endFrame = getTotalNumFrames( m_waveformItemList ) - 1;

    m_loopMarkerLeft->setFrameNum( startFrame );
    m_loopMarkerRight->setFrameNum( endFrame );

    m_loopMarkerLeft->setPos( 0.0, Ruler::HEIGHT );
    m_loopMarkerRight->setPos( getScenePosX( endFrame ), Ruler::HEIGHT );

    QTransform matrix;
    const qreal currentScaleFactor = views().first()->transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // loop marker remains correct width if view is scaled
    m_loopMarkerLeft->setTransform( matrix );
    m_loopMarkerRight->setTransform( matrix );

    QObject::connect( m_loopMarkerLeft, SIGNAL( scenePosChanged(FrameMarkerItem*) ),
                      this, SLOT( updateLoopMarkerFrameNum(FrameMarkerItem*) ) );

    QObject::connect( m_loopMarkerRight, SIGNAL( scenePosChanged(FrameMarkerItem*) ),
                      this, SLOT( updateLoopMarkerFrameNum(FrameMarkerItem*) ) );

    addItem( m_loopMarkerLeft );
    addItem( m_loopMarkerRight );
    update();

    updateLoopMarkerFrameNum( m_loopMarkerLeft );
    updateLoopMarkerFrameNum( m_loopMarkerRight );
}



void WaveGraphicsScene::setLoopMarkerFrameNum( LoopMarkerItem* const loopMarker )
{
    if ( loopMarker != NULL )
    {
        int newFrameNum = getFrameNum( loopMarker->scenePos().x() );

        loopMarker->setFrameNum( newFrameNum );
    }
}



int WaveGraphicsScene::getRelativeLoopMarkerFrameNum( const LoopMarkerItem* const loopMarker ) const
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



SharedWaveformItem WaveGraphicsScene::getWaveformUnderLoopMarker( const LoopMarkerItem* const loopMarker ) const
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



void WaveGraphicsScene::updateLoopMarkerFrameNums()
{
    if ( m_loopMarkerLeft != NULL && m_loopMarkerRight != NULL )
    {
        setLoopMarkerFrameNum( m_loopMarkerLeft );
        setLoopMarkerFrameNum( m_loopMarkerRight );

        emit loopMarkerPosChanged();
    }
}



void WaveGraphicsScene::snapLoopMarkerToSlicePoint( LoopMarkerItem* const loopMarker )
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



void WaveGraphicsScene::snapLoopMarkerToWaveform( LoopMarkerItem* const loopMarker )
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



void WaveGraphicsScene::snapSlicePointToLoopMarker( SlicePointItem* const slicePoint )
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



void WaveGraphicsScene::connectWaveform( const SharedWaveformItem item )
{
    QObject::connect( item.data(), SIGNAL( orderPosIsChanging(QList<int>,int) ),
                      this, SLOT( reorderWaveformItems(QList<int>,int) ) );

    QObject::connect( item.data(), SIGNAL( finishedMoving(int) ),
                      this, SLOT( slideWaveformItemIntoPlace(int) ) );

    QObject::connect( item.data(), SIGNAL( maxDetailLevelReached() ),
                      getView(), SLOT( relayMaxDetailLevelReached() ) );
}



void WaveGraphicsScene::createRuler()
{
    m_rulerBackground = addRect( 0.0, 0.0, width(), Ruler::HEIGHT, QPen(), QBrush( Qt::black ) );

    QGraphicsSimpleTextItem* textItem = addSimpleText( "0 BPM" );
    textItem->setPos( 1.0, 1.0 );
    textItem->setBrush( Qt::white );
    textItem->setZValue( 1 );
    m_rulerMarksList.append( SharedGraphicsItem( textItem ) );
}



//==================================================================================================
// Private Static:

int WaveGraphicsScene::getTotalNumFrames( QList<SharedWaveformItem> waveformItemList )
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

void WaveGraphicsScene::reorderWaveformItems( QList<int> oldOrderPositions, const int numPlacesMoved )
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



void WaveGraphicsScene::slideWaveformItemIntoPlace( const int orderPos )
{
    qreal newScenePosX = 0.0;

    for ( int i = 0; i < orderPos; i++ )
    {
        newScenePosX += m_waveformItemList.at( i )->rect().width();
    }

    m_waveformItemList.at( orderPos )->setPos( newScenePosX, Ruler::HEIGHT );

    updateLoopMarkerFrameNums();
}



void WaveGraphicsScene::updateSlicePointFrameNum( FrameMarkerItem* const movedItem )
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

    qSort( m_slicePointItemList.begin(), m_slicePointItemList.end(), SlicePointItem::isLessThanFrameNum );

    SharedSlicePointItem sharedSlicePoint;

    int numFramesFromPrevSlicePoint = movedItem->getFrameNum();
    int numFramesToNextSlicePoint = getTotalNumFrames( m_waveformItemList ) - movedItem->getFrameNum();
    int orderPos = 0;

    for ( int i = 0; i < m_slicePointItemList.size(); i++ )
    {
        SharedSlicePointItem item = m_slicePointItemList.at( i );

        if ( item == movedItem )
        {
            sharedSlicePoint = item;

            if ( i > 0 )
            {
                numFramesFromPrevSlicePoint = movedItem->getFrameNum() - m_slicePointItemList.at( i - 1 )->getFrameNum();
            }

            if ( i < m_slicePointItemList.size() - 1 )
            {
                numFramesToNextSlicePoint = m_slicePointItemList.at( i + 1 )->getFrameNum() - movedItem->getFrameNum();
            }

            orderPos = i;
            break;
        }
    }

    emit slicePointPosChanged( sharedSlicePoint, orderPos, numFramesFromPrevSlicePoint, numFramesToNextSlicePoint, oldFrameNum );
}



void WaveGraphicsScene::updateLoopMarkerFrameNum( FrameMarkerItem* const movedItem )
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



void WaveGraphicsScene::removePlayhead()
{
    removeItem( m_playhead );
    update();
}
