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
#include <QDebug>


//==================================================================================================
// Public:

WaveGraphicsScene::WaveGraphicsScene( const qreal x, const qreal y, const qreal width, const qreal height, QObject* parent ) :
    QGraphicsScene( x, y, width, height, parent ),
    m_interactionMode( AUDITION_ITEMS )
{
    createBpmRuler();

    // Set up playhead
    m_playhead = new QGraphicsLineItem( 0.0, 0.0, 0.0, height - BpmRuler::HEIGHT );
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



void WaveGraphicsScene::setInteractionMode( const InteractionMode mode )
{
    WaveGraphicsView* const view = getView();

    switch ( mode )
    {
    case SELECT_MOVE_ITEMS:
        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, true );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        view->setDragMode( QGraphicsView::NoDrag );
        break;
    case MULTI_SELECT_ITEMS:
        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        view->setDragMode( QGraphicsView::RubberBandDrag );
        break;
    case AUDITION_ITEMS:
        foreach ( SharedWaveformItem item, m_waveformItemList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, false );
        }
        view->setDragMode( QGraphicsView::NoDrag );
        break;
    default:
        break;
    }

    m_interactionMode = mode;
}



SharedWaveformItem WaveGraphicsScene::createWaveform( const SharedSampleBuffer sampleBuffer,
                                                      const SharedSampleHeader sampleHeader,
                                                      const bool shouldBeAddedToScene )
{
    Q_ASSERT( sampleBuffer->getNumFrames() > 0 );

    m_sampleHeader = sampleHeader;

    const int orderPos = 0;

    SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer, orderPos, width(), height() - BpmRuler::HEIGHT ) );

    connectWaveform( waveformItem );

    if ( shouldBeAddedToScene )
    {
        QList<SharedWaveformItem> waveformList;

        waveformList.append( waveformItem );

        insertWaveforms( waveformList );
    }

    return waveformItem;
}



QList<SharedWaveformItem> WaveGraphicsScene::createWaveforms( const QList<SharedSampleBuffer> sampleBufferList,
                                                              const SharedSampleHeader sampleHeader,
                                                              const bool shouldBeAddedToScene )
{
    m_sampleHeader = sampleHeader;

    const qreal sliceWidth = 1.0;   // This will get overridden in insertWaveforms()

    int orderPos = 0;

    QList<SharedWaveformItem> waveformList;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        SharedWaveformItem waveformItem( new WaveformItem( sampleBuffer, orderPos, sliceWidth, height() - BpmRuler::HEIGHT ) );

        waveformList << waveformItem;

        connectWaveform( waveformItem );

        orderPos++;
    }

    if ( shouldBeAddedToScene )
    {
        insertWaveforms( waveformList );
    }

    return waveformList;
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
        const qreal itemWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames ) * item->getStretchRatio();

        item->setRect( 0.0, 0.0, itemWidth, height() - BpmRuler::HEIGHT );
        item->setPos( scenePosX, BpmRuler::HEIGHT );

        scenePosX += itemWidth;
    }

    // Add waveform items to scene
    foreach ( SharedWaveformItem item, waveformItems )
    {
        addItem( item.data() );
    }
    update();

    setInteractionMode( m_interactionMode );
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

    // If necessary, set new order positions
    if ( startOrderPos < m_waveformItemList.size() )
    {
        for ( int i = startOrderPos; i < m_waveformItemList.size(); i++ )
        {
            m_waveformItemList.at( i )->setOrderPos( i );
        }
    }

    // Resize and reposition all waveform items
    const int totalNumFrames = getTotalNumFrames( m_waveformItemList );
    qreal scenePosX = 0.0;

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        const qreal itemWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames ) * item->getStretchRatio();

        item->setRect( 0.0, 0.0, itemWidth, height() - BpmRuler::HEIGHT );
        item->setPos( scenePosX, BpmRuler::HEIGHT );

        scenePosX += itemWidth;
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



void WaveGraphicsScene::selectNextWaveform()
{
    const QList<int> orderPositions = getSelectedWaveformsOrderPositions();

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        item->setSelected( false );
    }

    if ( ! orderPositions.isEmpty() )
    {
        int orderPos = orderPositions.last() + 1;

        if ( orderPos >= m_waveformItemList.size() )
        {
            orderPos = 0;
        }

        m_waveformItemList.at( orderPos )->setSelected( true );
    }
    else
    {
        m_waveformItemList.first()->setSelected( true );
    }
}



void WaveGraphicsScene::selectPreviousWaveform()
{
    const QList<int> orderPositions = getSelectedWaveformsOrderPositions();

    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        item->setSelected( false );
    }

    if ( ! orderPositions.isEmpty() )
    {
        int orderPos = orderPositions.first() - 1;

        if ( orderPos < 0 )
        {
            orderPos = m_waveformItemList.size() - 1;
        }

        m_waveformItemList.at( orderPos )->setSelected( true );
    }
    else
    {
        m_waveformItemList.last()->setSelected( true );
    }
}



void WaveGraphicsScene::stretchWaveforms( const QList<int> orderPosList, const QList<qreal> ratioList )
{
    if ( ! m_waveformItemList.isEmpty() )
    {
        const int totalNumFrames = getTotalNumFrames( m_waveformItemList );

        for ( int i = 0; i < orderPosList.size() && i < ratioList.size(); i++ )
        {
            SharedWaveformItem item = m_waveformItemList.at( orderPosList.at( i ) );

            const qreal origWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames );
            const qreal newWidth = origWidth * ratioList.at( i );

            item->setRect( 0.0, 0.0, newWidth, height() - BpmRuler::HEIGHT );
            item->setStretchRatio( ratioList.at( i ) );

            slideWaveformItemIntoPlace( orderPosList.at( i ) );
        }
    }
}



QList<qreal> WaveGraphicsScene::getWaveformStretchRatios( const QList<int> orderPositions ) const
{
    QList<qreal> stretchRatioList;

    if ( ! m_waveformItemList.isEmpty() )
    {
        const int totalNumFrames = getTotalNumFrames( m_waveformItemList );

        foreach ( int orderPos, orderPositions )
        {
            SharedWaveformItem item = m_waveformItemList.at( orderPos );

            const qreal origWidth = item->getSampleBuffer()->getNumFrames() * ( width() / totalNumFrames );

            stretchRatioList << item->rect().width() / origWidth;
        }
    }

    return stretchRatioList;
}



void WaveGraphicsScene::redrawWaveforms()
{
    resizeWaveformItems( 1.0 );
    getView()->viewport()->update();
}



SharedSlicePointItem WaveGraphicsScene::createSlicePoint( const int frameNum, const bool canBeMovedPastOtherSlicePoints )
{
    const qreal scenePosX = getScenePosX( frameNum );

    SlicePointItem* item = new SlicePointItem( height() - BpmRuler::HEIGHT, canBeMovedPastOtherSlicePoints );
    item->setPos( scenePosX, BpmRuler::HEIGHT );
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

    slicePoint.data()->setHeight( height() - BpmRuler::HEIGHT );
    slicePoint.data()->setPos( scenePosX, BpmRuler::HEIGHT );

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
    slicePointItem->setPos( newScenePosX, BpmRuler::HEIGHT );
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
    const int minFramesBetweenSlicePoints = roundToIntAccurate( m_sampleHeader->sampleRate * AudioAnalyser::MIN_INTER_ONSET_SECS );

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
    const int numFrames = getTotalNumFrames( m_waveformItemList );

    const qreal startPosX = 0.0;
    const qreal endPosX   = width() - 1;

    startPlayhead( startPosX, endPosX, numFrames, isLoopingDesired, stretchRatio );
}



void WaveGraphicsScene::startPlayhead( const qreal startPosX,
                                       const qreal endPosX,
                                       const int numFrames,
                                       const bool isLoopingDesired,
                                       const qreal stretchRatio )
{
    const qreal sampleRate = m_sampleHeader->sampleRate;

    if ( sampleRate > 0.0 )
    {
        const int millis = roundToIntAccurate( (numFrames / sampleRate) * 1000 * stretchRatio );

        if ( isPlayheadScrolling() )
        {
            stopPlayhead();
        }

        m_animation->setPosAt( 0.0, QPointF( startPosX, BpmRuler::HEIGHT ) );
        m_animation->setPosAt( 1.0, QPointF( endPosX,   BpmRuler::HEIGHT ) );

        m_playhead->setLine( 0.0, 0.0, 0.0, height() - BpmRuler::HEIGHT );
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
        const int numFrames = getTotalNumFrames( m_waveformItemList );

        const int newDuration = roundToInt( (numFrames / sampleRate) * 1000 * stretchRatio );
//        const int newTime = roundToInt( mTimer->currentTime() * stretchRatio );

        m_timer->setDuration( newDuration );
//        mTimer->setCurrentTime( newTime );

        m_timer->resume();
    }
}



void WaveGraphicsScene::setBpmRulerMarks( const qreal bpm, const int timeSigNumerator, int divisionsPerBeat )
{
    if ( bpm > 0.0 && timeSigNumerator > 0 )
    {
        if ( divisionsPerBeat < 1 )
        {
            divisionsPerBeat = 1;
        }

        const int divsPerBeat = divisionsPerBeat;

        foreach ( SharedGraphicsItem item, m_rulerMarksList )
        {
            removeItem( item.data() );
        }

        m_rulerMarksList.clear();

        QTransform matrix;
        const qreal currentScaleFactor = views().first()->transform().m11(); // m11() returns horizontal scale factor
        matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // ruler mark remains same width when view is scaled

#if QT_VERSION >= 0x040700  // Qt 4.7
        const qreal barScenePosY = 2.0;
        const qreal beatScenePosY = 3.0;
        const qreal divScenePosY = 7.0;
#else
        const qreal barScenePosY = 1.0;
        const qreal beatScenePosY = 2.0;
        const qreal divScenePosY = 6.0;
#endif

        const qreal beatLineHeight = BpmRuler::HEIGHT - 5.0;
        const qreal divLineHeight = BpmRuler::HEIGHT - 13.0;

        const int totalNumFrames = getTotalNumFrames( m_waveformItemList );
        const qreal framesPerDivision = ( ( m_sampleHeader->sampleRate * 60 ) / bpm ) / divsPerBeat;

        int frameNum = 0;
        int bar = 1;
        int beat = 0;
        int div = 0;

        while ( frameNum < totalNumFrames )
        {
            if ( div % (divsPerBeat * timeSigNumerator) == 0 ) // Bar
            {
                QGraphicsSimpleTextItem* textItem = addSimpleText( QString::number( bar ) );
                textItem->setPos( getScenePosX( frameNum ), barScenePosY );
                textItem->setBrush( Qt::white );
                textItem->setZValue( ZValues::BPM_RULER_TEXT );
                textItem->setTransform( matrix );
                m_rulerMarksList.append( SharedGraphicsItem( textItem ) );
                bar++;
                beat++;
            }
            else if ( div % divsPerBeat == 0 ) // Beat
            {
                QGraphicsLineItem* lineItem = addLine( 0.0, 0.0, 0.0, beatLineHeight );
                lineItem->setPos( getScenePosX( frameNum ), beatScenePosY );
                lineItem->setPen( QPen( Qt::white ) );
                lineItem->setZValue( ZValues::BPM_RULER_TEXT );
                lineItem->setTransform( matrix );
                m_rulerMarksList.append( SharedGraphicsItem( lineItem ) );
                beat++;
            }
            else // Division
            {
                QGraphicsLineItem* lineItem = addLine( 0.0, 0.0, 0.0, divLineHeight );
                lineItem->setPos( getScenePosX( frameNum ), divScenePosY );
                lineItem->setPen( QPen( Qt::white ) );
                lineItem->setZValue( ZValues::BPM_RULER_TEXT );
                lineItem->setTransform( matrix );
                m_rulerMarksList.append( SharedGraphicsItem( lineItem ) );
            }

            frameNum += framesPerDivision;
            div++;
        }
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

    createBpmRuler();
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

    int frameNum = roundToIntAccurate( scenePosX / ( width() / numFrames ) );

    if ( frameNum < 0 )
        frameNum = 0;

    if ( frameNum >= numFrames )
        frameNum = numFrames - 1;

    return frameNum;
}



void WaveGraphicsScene::resizeWaveformItems( const qreal scaleFactorX )
{
    foreach ( SharedWaveformItem item, m_waveformItemList )
    {
        const qreal newWidth = item->rect().width() * scaleFactorX;
        item->setRect( 0.0, 0.0, newWidth, height() - BpmRuler::HEIGHT );

        const qreal newX = item->scenePos().x() * scaleFactorX;
        item->setPos( newX, BpmRuler::HEIGHT );
    }
}



void WaveGraphicsScene::resizeSlicePointItems( const qreal scaleFactorX )
{
    const qreal height = this->height() - BpmRuler::HEIGHT;

    foreach ( SharedSlicePointItem slicePoint, m_slicePointItemList )
    {
        slicePoint->setHeight( height );

        const bool isSnapEnabled = slicePoint->isSnapEnabled();

        slicePoint->setSnap( false );

        const qreal scenePosX = slicePoint->scenePos().x() * scaleFactorX;

        slicePoint->setPos( scenePosX, BpmRuler::HEIGHT );

        slicePoint->setSnap( isSnapEnabled );
    }
}



void WaveGraphicsScene::resizePlayhead()
{
    if ( m_timer->state() == QTimeLine::Running )
    {
        m_timer->stop();

        m_animation->clear();
        m_animation->setPosAt( 0.0, QPointF( 0.0, BpmRuler::HEIGHT ) );
        m_animation->setPosAt( 1.0, QPointF( width() - 1, BpmRuler::HEIGHT ) );

        m_playhead->setLine( 0.0, 0.0, 0.0, height() - BpmRuler::HEIGHT );

        m_timer->resume();
    }
}



void WaveGraphicsScene::resizeRuler( const qreal scaleFactorX )
{
    m_rulerBackground->setRect( 0.0, 0.0, width(), BpmRuler::HEIGHT );

    foreach ( SharedGraphicsItem item, m_rulerMarksList )
    {
        const qreal newX = item->scenePos().x() * scaleFactorX;
        item->setPos( newX, item->scenePos().y() );
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
    }
}



//==================================================================================================
// Private:

WaveGraphicsView* WaveGraphicsScene::getView() const
{
    return static_cast<WaveGraphicsView*>( views().first() );
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



void WaveGraphicsScene::createBpmRuler()
{
    m_rulerBackground = addRect( 0.0, 0.0, width(), BpmRuler::HEIGHT, QPen( QColor(0,0,0,0) ), QBrush( Qt::darkGray ) );
    m_rulerBackground->setZValue( ZValues::BPM_RULER );

    QGraphicsSimpleTextItem* textItem = addSimpleText( "0 BPM" );
    textItem->setPos( 1.0, 1.0 );
    textItem->setBrush( Qt::white );
    textItem->setZValue( ZValues::BPM_RULER_TEXT );
    m_rulerMarksList.append( SharedGraphicsItem( textItem ) );
}



//==================================================================================================
// Private Static:

int WaveGraphicsScene::getTotalNumFrames( const QList<SharedWaveformItem> waveformItemList )
{
    int numFrames = 0;

    foreach ( SharedWaveformItem item, waveformItemList )
    {
        numFrames += static_cast<int>( item->getSampleBuffer()->getNumFrames() * item->getStretchRatio() );
    }

    return numFrames;
}



//==================================================================================================
// Private Slots:

void WaveGraphicsScene::reorderWaveformItems( const QList<int> oldOrderPositions, const int numPlacesMoved )
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

            m_waveformItemList.at( orderPos )->setPos( currentScenePosX + distanceToMove, BpmRuler::HEIGHT );
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

            m_waveformItemList.at( orderPos )->setPos( currentScenePosX - distanceToMove, BpmRuler::HEIGHT );
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

    m_waveformItemList.at( orderPos )->setPos( newScenePosX, BpmRuler::HEIGHT );
}



void WaveGraphicsScene::updateSlicePointFrameNum( FrameMarkerItem* const movedItem )
{
    const int oldFrameNum = movedItem->getFrameNum();
    const int newFrameNum = getFrameNum( movedItem->pos().x() );

    movedItem->setFrameNum( newFrameNum );

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



void WaveGraphicsScene::removePlayhead()
{
    removeItem( m_playhead );
    update();
}
