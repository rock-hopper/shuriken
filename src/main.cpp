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

#include <QtGui/QApplication>
#include "mainwindow.h"
#include <signal.h>
#include "signallistener.h"
#include <QThread>
#include <QFileInfo>
#include <QDebug>


static int setupSignalHandlers()
{
    struct sigaction sigusr1Action;
    struct sigaction sigtermAction;

    sigusr1Action.sa_handler = SignalListener::sigusr1Handler;
    sigemptyset( &sigusr1Action.sa_mask );
    sigusr1Action.sa_flags = SA_RESTART;

    if ( sigaction( SIGUSR1, &sigusr1Action, NULL ) > 0 )
    {
        return 1;
    }

    sigtermAction.sa_handler = SignalListener::sigtermHandler;
    sigemptyset( &sigtermAction.sa_mask );
    sigtermAction.sa_flags = SA_RESTART;

    if ( sigaction( SIGTERM, &sigtermAction, NULL ) > 0 )
    {
        return 2;
    }

    return 0;
}


int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );
    MainWindow window;
    SignalListener listener;
    QThread listenerThread;

    listenerThread.start();
    listener.moveToThread( &listenerThread );

    setupSignalHandlers();

    QObject::connect( &listener, SIGNAL( save() ),
                      &window, SLOT( on_actionSave_Project_triggered() ),
                      Qt::BlockingQueuedConnection );

    QObject::connect( &listener, SIGNAL( quit() ),
                      &window, SLOT( close() ),
                      Qt::BlockingQueuedConnection );

    window.show();

    if ( app.arguments().size() > 1 )
    {
        QString filePath = app.arguments().at( 1 );
        QFileInfo projFile( filePath );

        if ( projFile.exists() )
        {
            window.openProject( filePath );
        }
    }

    const int exitValue = app.exec();

    listenerThread.quit();
    listenerThread.wait( 2000 );

    return exitValue;
}
