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

#ifndef OFFLINETIMESTRETCHER_H
#define OFFLINETIMESTRETCHER_H

#include "samplebuffer.h"
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;


class OfflineTimeStretcher
{
public:
    static int stretch( SharedSampleBuffer sampleBuffer,
                        int sampleRate,
                        int numChans,
                        RubberBandStretcher::Options options,
                        qreal timeRatio,
                        qreal pitchScale );
};


#endif // OFFLINETIMESTRETCHER_H
