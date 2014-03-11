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
#include <QPainterPath>
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

    int getOrderPos() const                         { return mOrderPos; }
    void setOrderPos( const int orderPos )          { mOrderPos = orderPos; }
    int type() const                                { return Type; }

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL );

private:
    void setBackgroundGradient();
    void resetSampleBins();
    void findMinMaxSamples( const int startBin, const int endBin );

    const SharedSampleBuffer mSampleBuffer;
    QList<SharedPainterPath> mWavePathList;
    QPen mWavePen;
    QPen mCentreLinePen;
    int mOrderPos;
    qreal mScaleFactor;
    int mNumBins;
    qreal mBinSize;
    int mFirstCalculatedBin;
    int mLastCalculatedBin;
    OwnedArray< Array<float> > mMinSampleValues;
    OwnedArray< Array<float> > mMaxSampleValues;

private:
    static const int NOT_SET = -1;

signals:
    void orderPosChanged( const int oldOrderPos, const int newOrderPos );
};

#endif // WAVEFORMITEM_H
