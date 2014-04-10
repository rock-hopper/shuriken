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
//#include <QGLWidget>


//==================================================================================================
// Public:

WaveGraphicsView::WaveGraphicsView( QWidget* parent ) : QGraphicsView( parent )
{
    // Set up view
//    setViewport( new QGLWidget( QGLFormat(QGL::SampleBuffers) ) );
    setViewport( new QWidget() );
//    setViewportUpdateMode( QGraphicsView::FullViewportUpdate );
    setRenderHint( QPainter::Antialiasing, false );
    setRenderHint( QPainter::HighQualityAntialiasing, false );
    setOptimizationFlags( DontSavePainterState | DontAdjustForAntialiasing );
    setBackgroundBrush( Qt::gray );
    setCacheMode( CacheBackground );

    // Set up the scene
    setScene( new QGraphicsScene( 0.0, 0.0, 1024.0, 768.0 ) );
}



void WaveGraphicsView::createWaveform( const SharedSampleBuffer sampleBuffer )
{
    mNumFrames = sampleBuffer->getNumFrames();

    Q_ASSERT_X( mNumFrames != 0, "WaveGraphicsView::createWaveform()", "No. of frames cannot be zero" );

    const int orderPos = 0;

    WaveformItem* waveformItem = new WaveformItem( sampleBuffer, orderPos, scene()->width(), scene()->height() );
    waveformItem->setPos( 0.0, 0.0 );

    mWaveformItemList.append( SharedWaveformItem( waveformItem ) );

    QObject::connect( waveformItem, SIGNAL( rightMousePressed(int,QPointF) ),
                      this, SLOT( determinePlayPos(int,QPointF) ) );

    QObject::connect( waveformItem, SIGNAL( maxDetailLevelReached() ),
                      this, SLOT( relayMaxDetailLevelReached() ) );

    scene()->addItem( waveformItem );
    scene()->update();
}



QList<SharedWaveformItem> WaveGraphicsView::createWaveformSlices( const QList<SharedSampleBuffer> sampleBufferList )
{
    int totalNumFrames = 0;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        totalNumFrames += sampleBuffer->getNumFrames();
    }

    Q_ASSERT_X( totalNumFrames != 0, "WaveGraphicsView::createWaveformSlices", "division by zero" );

    qreal scenePosX = 0.0;
    int orderPos = 0;

    foreach ( SharedSampleBuffer sampleBuffer, sampleBufferList )
    {
        const qreal sliceWidth = sampleBuffer->getNumFrames() * ( scene()->width() / totalNumFrames );
        WaveformItem* waveformItem = new WaveformItem( sampleBuffer, orderPos, sliceWidth, scene()->height() );
        waveformItem->setPos( scenePosX, 0.0 );

        mWaveformItemList.append( SharedWaveformItem( waveformItem ) );

        QObject::connect( waveformItem, SIGNAL( orderPosIsChanging(int,int) ),
                          this, SLOT( reorderWaveformSlices(int,int) ) );

        QObject::connect( waveformItem, SIGNAL( finishedMoving(int) ),
                          this, SLOT( slideWaveformSliceIntoPlace(int) ) );

        QObject::connect( waveformItem, SIGNAL( rightMousePressed(int,QPointF) ),
                          this, SLOT( determinePlayPos(int,QPointF) ) );

        QObject::connect( waveformItem, SIGNAL( maxDetailLevelReached() ),
                          this, SLOT( relayMaxDetailLevelReached() ) );

        scene()->addItem( waveformItem );
        scene()->update();

        scenePosX += sliceWidth;
        orderPos++;
    }

    return mWaveformItemList;
}



void WaveGraphicsView::moveWaveformSlice( const int oldOrderPos, const int newOrderPos )
{
    Q_ASSERT_X( ! mWaveformItemList.isEmpty(), "WaveGraphicsView::moveWaveformSlice", "mWaveformItemList is empty" );

    reorderWaveformSlices( oldOrderPos, newOrderPos );
    slideWaveformSliceIntoPlace( newOrderPos );
}



SharedSlicePointItem WaveGraphicsView::createSlicePoint( const int frameNum )
{
    const qreal scenePosX = getScenePosX( frameNum );

    SharedSlicePointItem sharedSlicePoint;

    if ( scenePosX >= 0.0 && scenePosX <= scene()->width() - 1 )
    {
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
                          this, SLOT( reorderSlicePoints(SlicePointItem*const) ) );

        scene()->addItem( slicePointItem );
        scene()->update();
    }

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
    return frameNum * ( scene()->width() / mNumFrames );
}



int WaveGraphicsView::getFrameNum( qreal scenePosX ) const
{
    if ( scenePosX < 0.0)
        scenePosX = 0.0;

    if ( scenePosX > scene()->width() - 1 )
        scenePosX = scene()->width() - 1;

    return scenePosX / ( scene()->width() / mNumFrames );
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

void WaveGraphicsView::reorderWaveformSlices( const int oldOrderPos, const int newOrderPos )
{
    mWaveformItemList[ oldOrderPos ]->setOrderPos( newOrderPos );

    const qreal distanceToMove = mWaveformItemList[ oldOrderPos ]->rect().width();

    // If a slice has been dragged to the left
    if ( newOrderPos < oldOrderPos )
    {
        for ( int orderPos = newOrderPos; orderPos < oldOrderPos; orderPos++ )
        {
            const qreal currentScenePosX = mWaveformItemList[ orderPos ]->scenePos().x();
            mWaveformItemList[ orderPos ]->setPos( currentScenePosX + distanceToMove, 0.0 );
            mWaveformItemList[ orderPos ]->setOrderPos( orderPos + 1 );
        }
    }

    // If a slice has been dragged to the right
    if ( newOrderPos > oldOrderPos )
    {
        for ( int orderPos = newOrderPos; orderPos > oldOrderPos; orderPos-- )
        {
            const qreal currentScenePosX = mWaveformItemList[ orderPos ]->scenePos().x();
            mWaveformItemList[ orderPos ]->setPos( currentScenePosX - distanceToMove, 0.0 );
            mWaveformItemList[ orderPos ]->setOrderPos( orderPos - 1 );
        }
    }

    mWaveformItemList.move( oldOrderPos, newOrderPos );

    emit waveformSliceOrderChanged( oldOrderPos, newOrderPos );
}



void WaveGraphicsView::slideWaveformSliceIntoPlace( const int orderPos )
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



void WaveGraphicsView::reorderSlicePoints( SlicePointItem* const movedItem )
{
    SharedSlicePointItem slicePointItem;

    foreach ( SharedSlicePointItem sharedSlicePointItem, mSlicePointItemList )
    {
        if ( sharedSlicePointItem == movedItem )
        {
            slicePointItem = sharedSlicePointItem;
        }
    }

    const int oldFrameNum = slicePointItem->getFrameNum();
    const int newFrameNum = getFrameNum( slicePointItem->pos().x() );
    slicePointItem->setFrameNum( newFrameNum );

    emit slicePointOrderChanged( slicePointItem, oldFrameNum, newFrameNum );
}



void WaveGraphicsView::determinePlayPos( const int waveformItemOrderPos, const QPointF mouseScenePos )
{
    if ( ! mWaveformItemList.isEmpty() )
    {
        int startFrame = 0;
        int endFrame = mWaveformItemList.at( waveformItemOrderPos )->getSampleBuffer()->getNumFrames() - 1;

        // If slice points are present and the waveform has not yet been sliced...
        if ( mWaveformItemList.size() == 1 && ! mSlicePointItemList.isEmpty() )
        {
            const int mousePosFrameNum = getFrameNum( mouseScenePos.x() );

            const QList<int> slicePointFrameNumList = getSlicePointFrameNumList();

            foreach (  int slicePointFrameNum, slicePointFrameNumList )
            {
                if ( slicePointFrameNum <= mousePosFrameNum )
                {
                    startFrame = slicePointFrameNum;
                }
                else
                {
                    endFrame = slicePointFrameNum;
                    break;
                }
            }
        }

        emit rightMousePressed( waveformItemOrderPos, startFrame, endFrame );
    }
}



void WaveGraphicsView::relayMaxDetailLevelReached()
{
    emit maxDetailLevelReached();
}
