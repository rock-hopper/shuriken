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

    WaveformItem( const SharedSampleBuffer sampleBuffer,
                  const SharedSampleRange sampleRange,
                  const qreal width, const qreal height,
                  QGraphicsItem* parent = NULL );

    WaveformItem( const SharedSampleBuffer sampleBuffer,
                  const SharedSampleRange sampleRange,
                  const int orderPos,
                  const qreal width, const qreal height,
                  QGraphicsItem* parent = NULL );

    // Create a new waveform item by joining several waveform items together. The new item's order position
    // and scene position will be the same as the first item in the list and the new item's width will be
    // the sum of the widths of all items in the list. A new SampleRange will also be created.
    WaveformItem( const QList<SharedWaveformItem> items, QGraphicsItem* parent = NULL );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );
    void setRect( const qreal x, const qreal y, const qreal width, const qreal height );

    int type() const                                    { return Type; }

    SharedSampleBuffer getSampleBuffer() const          { return mSampleBuffer; }
    SharedSampleRange getSampleRange() const            { return mSampleRange; }

    int getOrderPos() const                             { return mCurrentOrderPos; }
    void setOrderPos( const int orderPos )              { mCurrentOrderPos = orderPos; }

    // Returns true if this waveform item has been created by joining several waveform items together
    bool isJoined() const                               { return ! mJoinedItems.isEmpty(); }

    QList<SharedWaveformItem> getJoinedItems() const    { return mJoinedItems; }

public:
    // For use with qSort(); sorts by start frame
    static bool isLessThanStartFrame( const SharedWaveformItem item1, const SharedWaveformItem item2 );

    // For use with qSort(); sorts by order position
    static bool isLessThanOrderPos( const WaveformItem* const item1, const WaveformItem* const item2 );

    // Get list of currently selected waveform items sorted by order position
    static QList<WaveformItem*> getSortedListSelectedItems( const QGraphicsScene* const scene );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void mousePressEvent( QGraphicsSceneMouseEvent* event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );

private:
    void init();
    void setBackgroundGradient();

    void resetSampleBins();

    // Both 'startBin' and 'endBin' are inclusive
    void findMinMaxSamples( const int startBin, const int endBin );

    enum DetailLevel { LOW, HIGH, VERY_HIGH };
    DetailLevel mDetailLevel;

    const SharedSampleBuffer mSampleBuffer;
    const SharedSampleRange mSampleRange;

    QList<SharedWaveformItem> mJoinedItems;

    QPen mWavePen;
    QPen mCentreLinePen;

    int mCurrentOrderPos;
    int mOrderPosBeforeMove;

    qreal mScaleFactor;

    int mNumBins;
    qreal mBinSize;
    int mFirstCalculatedBin;
    int mLastCalculatedBin;
    OwnedArray< Array<float> > mMinSampleValues;
    OwnedArray< Array<float> > mMaxSampleValues;

private:
    static const int NOT_SET = -1;
    static const qreal DETAIL_LEVEL_MAX_CUTOFF = 0.05;
    static const qreal DETAIL_LEVEL_VERY_HIGH_CUTOFF = 1.0;
    static const qreal DETAIL_LEVEL_HIGH_CUTOFF = 10.0;

signals:
    // As waveform items are being dragged their old order positions are emitted along with
    // the no. of places which they have moved, allowing other waveform items to be reshuffled.
    // 'orderPositions' is sorted
    // 'oldOrderPositions' is negative if the items have moved left, or positive if the items have moved right
    void orderPosIsChanging( QList<int> oldOrderPositions, const int numPlacesMoved );

    // This signal is emitted when the user has finished moving the waveform item(s) and the
    // final order position(s) is/are different from the starting order position(s)
    // 'oldOrderPositions' is sorted
    // 'numPlacesMoved' is negative if the items have moved left, or positive if the items have moved right
    void orderPosHasChanged( QList<int> oldOrderPositions, const int numPlacesMoved );

    void finishedMoving( const int orderPos );

    void playSampleRange( const int waveformItemStartFrame,
                            const int waveformItemNumFrames,
                            const QPointF mouseScenePos );

    void maxDetailLevelReached();

private:
    JUCE_LEAK_DETECTOR( WaveformItem );
};

#endif // WAVEFORMITEM_H
