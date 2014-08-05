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

#ifndef TEXTFILEHANDLER_H
#define TEXTFILEHANDLER_H

#include <QString>
#include <QStringList>
#include "samplebuffer.h"
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;


class TextFileHandler
{
public:
    struct ProjectSettings
    {
        ProjectSettings() :
            originalBpm( 0.0 ),
            newBpm( 0.0 ),
            appliedBpm( 0.0 ),
            isTimeStretchChecked( false ),
            isPitchCorrectionChecked( false ),
            isJackSyncChecked( false ),
            options( 0 )
        {
        }

        QString projectName;
        QList<SharedSampleRange> sampleRangeList;
        QList<int> slicePointFrameNumList;
        QString audioFileName;
        qreal originalBpm;
        qreal newBpm;
        qreal appliedBpm;
        bool isTimeStretchChecked;
        bool isPitchCorrectionChecked;
        bool isJackSyncChecked;
        RubberBandStretcher::Options options;
    };

    static bool createProjectXmlFile( const QString filePath,
                                      const QString projectName,
                                      const ProjectSettings settings,
                                      const QList<SharedSampleRange> sampleRangeList,
                                      const QList<int> slicePointFrameNumList )
    ;
    static bool readProjectXmlFile( const QString filePath, ProjectSettings& settings );

    static bool createH2DrumkitXmlFile( const QString dirPath, const QString kitName, QStringList audioFileNames );
};

#endif // TEXTFILEHANDLER_H
