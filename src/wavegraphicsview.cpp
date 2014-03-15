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
    const int numFrames = sampleBuffer->getNumFrames();

    Q_ASSERT_X( numFrames != 0, "WaveGraphicsView::createWaveform()", "division by zero" );

    const int orderPos = 0;
    const qreal waveformWidth = numFrames * ( scene()->width() / numFrames );

    WaveformItem* waveformItem = new WaveformItem( sampleBuffer, orderPos, waveformWidth, scene()->height() );
    waveformItem->setPos( 0.0, 0.0 );

    mWaveformItemList.append( SharedWaveformItem( waveformItem ) );
    scene()->addItem( waveformItem );
}



QList<SharedWaveformItem> WaveGraphicsView::createWaveformSlices( const QList<SharedSampleBuffer> sampleBufferList )
{
    uint totalNumFrames = 0;

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
        scene()->addItem( waveformItem );

        QObject::connect( waveformItem, SIGNAL( orderPosIsChanging(int,int) ),
                          this, SLOT( reorderWaveformSlices(int,int) ) );

        QObject::connect( waveformItem, SIGNAL( finishedMoving(int) ),
                          this, SLOT( slideWaveformSliceIntoPlace(int) ) );

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



SharedSlicePointItem WaveGraphicsView::createSlicePoint( const qreal scenePosX )
{
    SharedSlicePointItem sharedSlicePoint;

    if ( scenePosX >= 0.0 && scenePosX <= scene()->width() - 1 )
    {
        SlicePointItem* pSlicePointItem = new SlicePointItem( scene()->height() - 1 );
        pSlicePointItem->setPos( scenePosX, 0.0 );

        QTransform matrix;
        const qreal currentScaleFactor = transform().m11(); // m11() returns horizontal scale factor
        matrix.scale( 1.0 / currentScaleFactor, 1.0 ); // slice point remains same width when view is scaled
        pSlicePointItem->setTransform( matrix );

        scene()->addItem( pSlicePointItem );
        sharedSlicePoint = SharedSlicePointItem( pSlicePointItem );
        mSlicePointItemList.append( sharedSlicePoint );
    }

    return sharedSlicePoint;
}



void WaveGraphicsView::deleteSlicePoint( const SharedSlicePointItem slicePointItem )
{
    scene()->removeItem( slicePointItem.data() );
    mSlicePointItemList.removeOne( slicePointItem );
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



QList<qreal> WaveGraphicsView::getSlicePointScenePosList()
{
    QList<qreal> slicePointScenePosList;

    foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
    {
        slicePointScenePosList.append( slicePointItem->scenePos().x() );
    }

    qSort( slicePointScenePosList );

    return slicePointScenePosList;
}



void WaveGraphicsView::clearAll()
{
    foreach ( QGraphicsItem* item, items() )
    {
        scene()->removeItem( item );
    }
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
    mWaveformItemList.clear();
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
// Private Slots:

void WaveGraphicsView::setZoom( const int zoomFactor )
{
    QTransform matrix;
    matrix.scale( qreal( zoomFactor ), 1.0 );
    setTransform( matrix );

    if ( ! mSlicePointItemList.isEmpty() )
    {
        QTransform matrix;
        matrix.scale( 1.0 / zoomFactor, 1.0 ); // slice points remain same width when view is scaled

        foreach ( SharedSlicePointItem slicePointItem, mSlicePointItemList )
        {
            slicePointItem->setTransform( matrix );
        }
    }
}



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

    emit orderPosChanged( oldOrderPos, newOrderPos );
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
