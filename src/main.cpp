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
#include <QDesktopWidget>
#include <QDebug>


static void setupSignalHandlers()
{
    struct sigaction sigusr1Action;
    struct sigaction sigtermAction;

    sigusr1Action.sa_handler = SignalListener::sigusr1Handler;
    sigemptyset( &sigusr1Action.sa_mask );
    sigusr1Action.sa_flags = SA_RESTART;

    if ( sigaction( SIGUSR1, &sigusr1Action, NULL ) > 0 )
    {
        std::cerr << "Failed to set up signal handler for SIGUSR1 \n";
    }

    sigtermAction.sa_handler = SignalListener::sigtermHandler;
    sigemptyset( &sigtermAction.sa_mask );
    sigtermAction.sa_flags = SA_RESTART;

    if ( sigaction( SIGTERM, &sigtermAction, NULL ) > 0 )
    {
        std::cerr << "Failed to set up signal handler for SIGTERM \n";
    }
}


int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );
    MainWindow window;
    SignalListener listener;
    QThread listenerThread;


    // Set up signal listeners
    listenerThread.start();
    listener.moveToThread( &listenerThread );

    setupSignalHandlers();

    QObject::connect( &listener, SIGNAL( save() ),
                      &window, SLOT( on_actionSave_Project_triggered() ),
                      Qt::BlockingQueuedConnection );

    QObject::connect( &listener, SIGNAL( quit() ),
                      &window, SLOT( close() ),
                      Qt::BlockingQueuedConnection );


    // Set up window geometry
    const int desktopWidth = app.desktop()->availableGeometry().width();
    const int desktopHeight = app.desktop()->availableGeometry().height();

    const int frameWidth = window.frameSize().width();
    const int frameHeight = window.frameSize().height();

    int windowWidth = window.size().width();
    int windowHeight = window.size().height();

    if ( frameWidth > desktopWidth )
    {
        windowWidth = desktopWidth - ( frameWidth - windowWidth );
    }

    if ( frameHeight > desktopHeight )
    {
        windowHeight = desktopHeight - ( frameHeight - windowHeight );
    }

    window.resize( windowWidth, windowHeight );

    window.setGeometry
    (
        QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, window.size(), app.desktop()->availableGeometry() )
    );

    window.show();


    // Try to open project if a file name has been passed on the command line
    if ( app.arguments().size() > 1 )
    {
        QString filePath = app.arguments().at( 1 );
        QFileInfo projFile( filePath );

        if ( projFile.exists() )
        {
            window.openProject( filePath );
        }
    }


    // Start main event loop
    const int exitValue = app.exec();


    // Wait for listener thread to finish before exiting
    listenerThread.quit();
    listenerThread.wait( 2000 );

    return exitValue;
}
