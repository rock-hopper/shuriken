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

#ifndef SAMPLEUTILS_H
#define SAMPLEUTILS_H

#include "samplebuffer.h"


class SampleUtils
{
public:
    static SharedSampleBuffer joinSampleBuffers( QList<SharedSampleBuffer> sampleBufferList );

    // Split a sample buffer into multiple sample buffers at the specified slice points.
    // Slice points greater than or equal to the length of the sample buffer are ignored
    static QList<SharedSampleBuffer> splitSampleBuffer( SharedSampleBuffer sampleBuffer, QList<int> slicePointFrameNums );

    static int getTotalNumFrames( QList<SharedSampleBuffer> sampleBufferList );

    static int getPrevZeroCrossing( SharedSampleBuffer sampleBuffer, int startFrameNum );

    static int getNextZeroCrossing( SharedSampleBuffer sampleBuffer, int startFrameNum );
};


#endif // SAMPLEUTILS_H
