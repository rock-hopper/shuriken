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
#include <QDebug>
//#include <QGLWidget>


//==================================================================================================
// Public:

WaveGraphicsView::WaveGraphicsView( QWidget* parent ) :
    QGraphicsView( parent )
{
    // Set up view
//    setViewport( new QGLWidget( QGLFormat(QGL::SampleBuffers) ) );
//    setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
//    setRenderHint( QPainter::HighQualityAntialiasing, false );
    setViewport( new QWidget() );
    setRenderHint( QPainter::Antialiasing, false );

    setOptimizationFlags( DontSavePainterState | DontAdjustForAntialiasing );
    setBackgroundBrush( Qt::gray );
    setCacheMode( CacheBackground );

    // Set up the scene
    setScene( new QGraphicsScene( 0.0, 0.0, 1024.0, 768.0 ) );
}



SharedWaveformItem WaveGraphicsView::createWaveform( const SharedSampleBuffer sampleBuffer,
                                                     const SharedSampleRange sampleRange )
{
    Q_ASSERT( sampleBuffer->getNumFrames() > 0 );

    mSampleBuffer = sampleBuffer;

    WaveformItem* waveformItem = new WaveformItem( sampleBuffer, sampleRange, scene()->width(), scene()->height() );
    waveformItem->setPos( 0.0, 0.0 );

    mWaveformItemList.append( SharedWaveformItem( waveformItem ) );

    QObject::connect( waveformItem, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( relayMaxDetailLevelReached() ) );

    scene()->addItem( waveformItem );
    scene()->update();

    return mWaveformItemList.first();
}



QList<SharedWaveformItem> WaveGraphicsView::createWaveforms( const SharedSampleBuffer sampleBuffer,
                                                             const QList<SharedSampleRange> sampleRangeList )
{
    Q_ASSERT( sampleBuffer->getNumFrames() > 0 );

    mSampleBuffer = sampleBuffer;

    qreal scenePosX = 0.0;
    int orderPos = 0;

    foreach ( SharedSampleRange sampleRange, sampleRangeList )
    {
        const qreal sliceWidth = sampleRange->numFrames * ( scene()->width() / sampleBuffer->getNumFrames() );

        WaveformItem* waveformItem = new WaveformItem( sampleBuffer,
                                                       sampleRange,
                                                       orderPos,
                                                       sliceWidth,
                                                       scene()->height() );
        waveformItem->setPos( scenePosX, 0.0 );

        mWaveformItemList.append( SharedWaveformItem( waveformItem ) );

        QObject::connect( waveformItem, SIGNAL( orderPosIsChanging(QList<int>,int) ),
                          this, SLOT( reorderWaveformItems(QList<int>,int) ) );

        QObject::connect( waveformItem, SIGNAL( finishedMoving(int) ),
                          this, SLOT( slideWaveformItemIntoPlace(int) ) );

        QObject::connect( waveformItem, SIGNAL( maxDetailLevelReached() ),
                          this, SLOT( relayMaxDetailLevelReached() ) );

        scene()->addItem( waveformItem );
        scene()->update();

        scenePosX += sliceWidth;
        orderPos++;
    }

    return mWaveformItemList;
}



SharedWaveformItem WaveGraphicsView::joinWaveforms( const QList<int> orderPositions )
{
    QList<SharedWaveformItem> itemsToJoin;

    foreach( int orderPos, orderPositions )
    {
        itemsToJoin << mWaveformItemList.at( orderPos );
    }

    WaveformItem* waveformItem = new WaveformItem( itemsToJoin );

    foreach ( SharedWaveformItem item, itemsToJoin )
    {
        scene()->removeItem( item.data() );

        mWaveformItemList.removeOne( item );
    }

    const int newItemOrderPos = waveformItem->getOrderPos();

    mWaveformItemList.insert( newItemOrderPos, SharedWaveformItem( waveformItem ) );

    for ( int orderPos = newItemOrderPos + 1; orderPos < mWaveformItemList.size(); orderPos++ )
    {
        mWaveformItemList.at( orderPos )->setOrderPos( orderPos );
    }

    QObject::connect( waveformItem, SIGNAL( orderPosIsChanging(QList<int>,int) ),
                      this, SLOT( reorderWaveformItems(QList<int>,int) ) );

    QObject::connect( waveformItem, SIGNAL( finishedMoving(int) ),
                      this, SLOT( slideWaveformItemIntoPlace(int) ) );

    QObject::connect( waveformItem, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( relayMaxDetailLevelReached() ) );

    if ( dragMode() == RubberBandDrag )
    {
        waveformItem->setFlag( QGraphicsItem::ItemIsMovable, false );
    }

    scene()->addItem( waveformItem );
    scene()->update();

    return mWaveformItemList.at( newItemOrderPos );
}



QList<SharedWaveformItem> WaveGraphicsView::splitWaveform( const int orderPos )
{
    SharedWaveformItem itemToSplit = mWaveformItemList.at( orderPos );

    if( itemToSplit->isJoined() )
    {
        QList<SharedWaveformItem> joinedItems = itemToSplit->getJoinedItems();

        scene()->removeItem( itemToSplit.data() );

        mWaveformItemList.removeOne( itemToSplit );

        int orderPos = itemToSplit->getOrderPos();

        foreach ( SharedWaveformItem item, joinedItems )
        {
            mWaveformItemList.insert( orderPos, item );

            if ( dragMode() == RubberBandDrag )
            {
                item->setFlag( QGraphicsItem::ItemIsMovable, false );
            }
            else
            {
                item->setFlag( QGraphicsItem::ItemIsMovable, true );
            }

            scene()->addItem( item.data() );
            orderPos++;
        }

        while ( orderPos < mWaveformItemList.size() )
        {
            mWaveformItemList.at( orderPos )->setOrderPos( orderPos );
            orderPos++;
        }

        scene()->update();
    }

    return itemToSplit->getJoinedItems();
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



QList<int> WaveGraphicsView::getSelectedWaveformsOrderPositions() const
{
    const QList<WaveformItem*> selectedItems = WaveformItem::getSortedListSelectedItems( scene() );
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



SharedSlicePointItem WaveGraphicsView::createSlicePoint( const int frameNum )
{
    const qreal scenePosX = getScenePosX( frameNum );

    SharedSlicePointItem sharedSlicePoint;

    SlicePointItem* slicePointItem = new SlicePointItem( scene()->height() - 1 );
    slicePointItem->setPos( scenePosX, 0.0 );
    slicePointItem->setFrameNum( frameNum );

    QTransform matrix;
    const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
    matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point remains same width when view is scaled
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



void WaveGraphicsView::deleteSlicePoint( const SharedSlicePointItem slicePointItem )
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



QList<int> WaveGraphicsView::getSlicePointFrameNumList()
{
    QList<int> slicePointFrameNumList;

    foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
    {
        slicePointFrameNumList.append( slicePointItem->getFrameNum() );
    }

    qSort( slicePointFrameNumList );

    return slicePointFrameNumList;
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



void WaveGraphicsView::clearAll()
{
    foreach ( QGraphicsItem* item, items() )
    {
        scene()->removeItem( item );
    }
    scene()->update();

    mWaveformItemList.clear();
    mSlicePointItemList.clear();
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
    qreal scenePosX = frameNum * ( scene()->width() / mSampleBuffer->getNumFrames() );

    if ( scenePosX < 0.0)
        scenePosX = 0.0;

    if ( scenePosX >= scene()->width() )
        scenePosX = scene()->width() - 1;

    return scenePosX;
}



int WaveGraphicsView::getFrameNum( qreal scenePosX ) const
{
    const int numFrames = mSampleBuffer->getNumFrames();

    int frameNum = roundToInt( scenePosX / ( scene()->width() / numFrames ) );

    if ( frameNum < 0 )
        frameNum = 0;

    if ( frameNum >= numFrames )
        frameNum = numFrames - 1;

    return frameNum;
}



void WaveGraphicsView::zoomIn()
{
    const qreal newXScaleFactor = transform().m11() * 2; // m11() returns the current horizontal scale factor

    QTransform matrix;
    matrix.scale( newXScaleFactor, 1.0 );
    setTransform( matrix );

    scaleSlicePointItems( newXScaleFactor );
}



void WaveGraphicsView::zoomOut()
{
    const qreal newXScaleFactor = transform().m11() * 0.5; // m11() returns the current horizontal scale factor

    QTransform matrix;
    matrix.scale( newXScaleFactor, 1.0 );
    setTransform( matrix );

    scaleSlicePointItems( newXScaleFactor );

    if ( newXScaleFactor == 1.0 )
    {
        emit minDetailLevelReached();
    }
}



void WaveGraphicsView::zoomOriginal()
{
    resetTransform();
    scaleSlicePointItems( 1.0 );
}



void WaveGraphicsView::forceRedraw()
{
    // Hack to force redraw of scene
    QSize size( sceneRect().width(), sceneRect().height() );
    QSize oldSize( sceneRect().width(), sceneRect().height() );
    QResizeEvent event( size, oldSize );
    resizeEvent( &event );
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
}



//==================================================================================================
// Protected:

void WaveGraphicsView::resizeEvent ( QResizeEvent* event )
{
    const qreal oldSceneWidth = scene()->width();

    scene()->setSceneRect( 0.0, 0.0, event->size().width(), event->size().height() );

    const qreal scaleFactor = scene()->width() / oldSceneWidth;

    if ( ! mWaveformItemList.isEmpty() )
    {
        foreach ( SharedWaveformItem waveformItem, mWaveformItemList )
        {
            const qreal newWidth = waveformItem->rect().width() * scaleFactor;
            waveformItem->setRect( 0.0, 0.0, newWidth, scene()->height() );
            const qreal newX = waveformItem->scenePos().x() * scaleFactor;
            waveformItem->setPos( newX, 0.0 );
        }
    }

    if ( ! mSlicePointItemList.isEmpty() )
    {
        foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
        {
            slicePointItem->setHeight( scene()->height() - 1 );
            const qreal newX = slicePointItem->scenePos().x() * scaleFactor;
            slicePointItem->setPos( newX, 0.0 );
        }
    }

    QGraphicsView::resizeEvent( event );
}



//==================================================================================================
// Private:

void WaveGraphicsView::scaleSlicePointItems( const qreal newXScaleFactor )
{
    if ( ! mSlicePointItemList.isEmpty() && newXScaleFactor > 0.0 )
    {
        QTransform matrix;
        matrix.scale( 1.0 / newXScaleFactor, 1.0 ); // Slice points remain same width when view is scaled

        foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
        {
            slicePointItem->setTransform( matrix );
        }
    }
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

    // New scene position for this item = sum of the widths of all WaveformItems to the left of this one
    foreach ( QGraphicsItem* const item, scene()->items() )
    {
        if ( item->type() == WaveformItem::Type )
        {
            WaveformItem* const otherWaveformItem = qgraphicsitem_cast<WaveformItem*>( item );

            if ( otherWaveformItem->getOrderPos() < orderPos )
            {
                newScenePosX += otherWaveformItem->rect().width();
            }
        }
    }

    mWaveformItemList[ orderPos ]->setPos( newScenePosX, 0.0 );
}



void WaveGraphicsView::updateSlicePointFrameNum( SlicePointItem* const movedItem )
{
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



void WaveGraphicsView::relayMaxDetailLevelReached()
{
    emit maxDetailLevelReached();
}
