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

#include "zipper.h"
#include "JuceHeader.h"
#include <QDebug>


//==================================================================================================
// Public Static:

void Zipper::compress( const QString sourceDirPath, const QString zipFilePath )
{
    const File sourceDir( sourceDirPath.toLocal8Bit().data() );
    const File zipFile( zipFilePath.toLocal8Bit().data() );
    const File parentDir = zipFile.getParentDirectory();

    ZipFile::Builder zipBuilder;
    Array<File> files;

    sourceDir.findChildFiles( files, File::findFiles, false );

    for ( int i = 0; i < files.size(); i++ )
    {
        zipBuilder.addFile( files[i], 9, files[i].getRelativePathFrom( parentDir ) );
    }

    if ( zipFile.exists() )
    {
        zipFile.deleteFile();
    }

    FileOutputStream stream( zipFile );
    zipBuilder.writeToStream( stream, NULL );
}



void Zipper::decompress( const QString zipFilePath, const QString destDirPath )
{
    const File zipFile( zipFilePath.toLocal8Bit().data() );
    const File destDir( destDirPath.toLocal8Bit().data() );

    ZipFile( zipFile ).uncompressTo( destDir );
}
