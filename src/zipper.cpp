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


//==================================================================================================
// Public Static:

void Zipper::compress( File sourceDir, File zipFile )
{
    ZipFile::Builder zipBuilder;
    Array<File> files;
    sourceDir.findChildFiles( files, File::findFiles, false );

    for ( int i = 0; i < files.size(); i++ )
    {
        zipBuilder.addFile( files[i], 9, files[i].getRelativePathFrom( zipFile ) );
    }

    if ( zipFile.exists() )
    {
        zipFile.deleteFile();
    }

    FileOutputStream stream( zipFile );
    zipBuilder.writeToStream( stream, NULL );
}



void Zipper::decompress()
{
}
