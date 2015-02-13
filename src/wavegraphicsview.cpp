/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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


//==================================================================================================
// Public:

WaveGraphicsView::WaveGraphicsView( QWidget* parent ) :
    QGraphicsView( parent ),
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

    m_scene = new WaveGraphicsScene( 0.0, 0.0, 1024.0, 768.0 );
    setScene( m_scene );
}



WaveGraphicsScene* WaveGraphicsView::getScene() const
{
    return m_scene;
}



void WaveGraphicsView::redrawWaveforms()
{
    m_scene->resizeWaveformItems( 1.0 );
    viewport()->update();
}



void WaveGraphicsView::zoomIn()
{
    m_isViewZoomedIn = true;

    const qreal newXScaleFactor = transform().m11() * 2; // m11() returns the current horizontal scale factor

    QTransform matrix;
    matrix.scale( newXScaleFactor, 1.0 );
    setTransform( matrix );

    m_scene->scaleItems( newXScaleFactor );
}



void WaveGraphicsView::zoomOut()
{
    const qreal newXScaleFactor = transform().m11() * 0.5; // m11() returns the current horizontal scale factor

    QTransform matrix;
    matrix.scale( newXScaleFactor, 1.0 );
    setTransform( matrix );

    m_scene->scaleItems( newXScaleFactor );

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
    m_scene->scaleItems( 1.0 );
}



void WaveGraphicsView::setInteractionMode( const InteractionMode mode )
{
    const QList<SharedWaveformItem> waveformList = m_scene->getWaveformList();

    switch ( mode )
    {
    case SELECT_MOVE_ITEMS:
        foreach ( SharedWaveformItem item, waveformList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, true );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        setDragMode( NoDrag );
        break;
    case MULTI_SELECT_ITEMS:
        foreach ( SharedWaveformItem item, waveformList )
        {
            item->setFlag( QGraphicsItem::ItemIsMovable, false );
            item->setFlag( QGraphicsItem::ItemIsSelectable, true );
        }
        setDragMode( RubberBandDrag );
        break;
    case AUDITION_ITEMS:
        foreach ( SharedWaveformItem item, waveformList )
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



//==================================================================================================
// Protected:

void WaveGraphicsView::resizeEvent ( QResizeEvent* event )
{
    m_scene->setSceneRect( 0.0, 0.0, event->size().width(), event->size().height() );

    if ( event->oldSize().width() > 0 )
    {
        const qreal scaleFactorX = m_scene->width() / event->oldSize().width();

        m_scene->resizeWaveformItems( scaleFactorX );
        m_scene->resizeSlicePointItems( scaleFactorX );
        m_scene->resizePlayhead();
        m_scene->resizeRuler( scaleFactorX );
    }

    QGraphicsView::resizeEvent( event );
}



//==================================================================================================
// Private Slots:

void WaveGraphicsView::relayMaxDetailLevelReached()
{
    if ( m_isViewZoomedIn )
    {
        emit maxDetailLevelReached();
    }
}
