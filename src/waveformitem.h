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

#ifndef WAVEFORMITEM_H
#define WAVEFORMITEM_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QList>
#include "JuceHeader.h"
#include "samplebuffer.h"
#include "globals.h"


class WaveformItem;
typedef QSharedPointer<WaveformItem> SharedWaveformItem;


class WaveformItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    enum { Type = UserTypes::WAVEFORM };

    WaveformItem( SharedSampleBuffer sampleBuffer,
                  int orderPos,
                  qreal width, qreal height,
                  QGraphicsItem* parent = NULL );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );
    void setRect( qreal x, qreal y, qreal width, qreal height );

    int type() const                                                { return Type; }

    SharedSampleBuffer getSampleBuffer() const                      { return m_sampleBuffer; }

    int getOrderPos() const                                         { return m_currentOrderPos; }
    void setOrderPos( int orderPos )                                { m_currentOrderPos = orderPos; }

public:
    // For use with qSort(); sorts by order position
    static bool isLessThanOrderPos( const WaveformItem* item1, const WaveformItem* item2 );

    // Get list of currently selected waveform items sorted by order position
    static QList<WaveformItem*> getSelectedWaveformItems( const QGraphicsScene* scene );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void mousePressEvent( QGraphicsSceneMouseEvent* event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );

private:
    void setBackgroundGradient();

    void resetSampleBins();

    // Both 'startBin' and 'endBin' are inclusive
    void findMinMaxSamples( int startBin, int endBin );

    enum DetailLevel { LOW, HIGH, VERY_HIGH };
    DetailLevel m_detailLevel;

    const SharedSampleBuffer m_sampleBuffer;

    QPen m_wavePen;
    QPen m_centreLinePen;

    int m_currentOrderPos;
    int m_orderPosBeforeMove;

    qreal m_scaleFactor;

    int m_numBins;
    qreal m_binSize;
    int m_firstCalculatedBin;
    int m_lastCalculatedBin;
    OwnedArray< Array<float> > m_minSampleValues;
    OwnedArray< Array<float> > m_maxSampleValues;

private:
    static const int NOT_SET = -1;
    static const qreal DETAIL_LEVEL_MAX_CUTOFF = 0.05;
    static const qreal DETAIL_LEVEL_VERY_HIGH_CUTOFF = 1.0;
    static const qreal DETAIL_LEVEL_HIGH_CUTOFF = 10.0;

signals:
    // As waveform items are being dragged, their old order positions are emitted along with
    // the no. of places which they have moved, allowing other waveform items to be reshuffled.
    // 'orderPositions' is sorted
    // 'oldOrderPositions' is negative if the items have moved left, or positive if the items have moved right
    void orderPosIsChanging( QList<int> oldOrderPositions, int numPlacesMoved );

    // This signal is emitted when the user has finished moving the waveform item(s) and the
    // final order position(s) is/are different from the starting order position(s)
    // 'oldOrderPositions' is sorted
    // 'numPlacesMoved' is negative if the items have moved left, or positive if the items have moved right
    void orderPosHasChanged( QList<int> oldOrderPositions, int numPlacesMoved );

    void finishedMoving( int orderPos );

    void clicked( const WaveformItem* waveformItem, QPointF mouseScenePos );

    void maxDetailLevelReached();

private:
    JUCE_LEAK_DETECTOR( WaveformItem );
};

#endif // WAVEFORMITEM_H
