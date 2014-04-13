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
#include "samplebuffer.h"


typedef QSharedPointer<QPainterPath> SharedPainterPath;


class WaveformItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 1 };

    WaveformItem( const SharedSampleBuffer sampleBuffer, const int orderPos,
                  const qreal width, const qreal height, QGraphicsItem* parent = NULL );

    void setRect( const qreal x, const qreal y, const qreal width, const qreal height );

    int getOrderPos() const                             { return mCurrentOrderPos; }
    void setOrderPos( const int orderPos )              { mCurrentOrderPos = orderPos; }

    const SharedSampleBuffer getSampleBuffer() const    { return mSampleBuffer; }

    int type() const                                    { return Type; }

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void mousePressEvent( QGraphicsSceneMouseEvent* event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );

private:
    void setBackgroundGradient();
    void resetSampleBins();
    void findMinMaxSamples( const int startBin, const int endBin );

    enum DetailLevel { LOW, HIGH, VERY_HIGH };
    DetailLevel mDetailLevel;

    const SharedSampleBuffer mSampleBuffer;
    QList<SharedPainterPath> mWavePathList;
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
    static const qreal DETAIL_LEVEL_HIGH_CUTOFF = 5.0;

signals:
    // As waveform items are moved their old and new order positions are emitted, allowing other
    // waveform items to be reshuffled and their associated sample buffers to be reordered
    void orderPosIsChanging( const int oldOrderPos, const int newOrderPos );

    // When the user finishes moving the waveform item its start and destination order positions
    // are emitted, allowing the positions to be recorded in the undo stack
    void orderPosHasChanged( const int startOrderPos, const int destOrderPos );

    void finishedMoving( const int orderPos );
    void rightMousePressed( const int itemOrderPos, const QPointF mouseScenePos );
    void maxDetailLevelReached();

};

#endif // WAVEFORMITEM_H
