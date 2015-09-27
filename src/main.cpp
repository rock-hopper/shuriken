/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014, 2015 Andrew M Taylor <a.m.taylor303@gmail.com>

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

#include <QApplication>
#include "mainwindow.h"
#include <signal.h>
#include "signallistener.h"
#include "JuceHeader.h"
#include <QtDebug>
#include <QFile>
#include <QTextStream>


void messageHandler( QtMsgType messageType, const char* message )
{
    QString text;

    switch ( messageType )
    {
    case QtDebugMsg:
        text = QString( "Debug: %1" ).arg( message );
        break;
    case QtWarningMsg:
        text = QString( "Warning: %1" ).arg( message );
        break;
    case QtCriticalMsg:
        text = QString( "Critical: %1" ).arg( message );
        break;
    case QtFatalMsg:
        text = QString( "Fatal: %1" ).arg( message );
        break;
    }

    QFile outFile( "/dev/shm/debuglog.txt" );

    if ( outFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
    {
        QTextStream textStream( &outFile );
        textStream << text << endl;
    }

    if ( messageType == QtFatalMsg )
    {
        abort();
    }
}



static void setupSignalHandlers()
{
    struct sigaction sigusr1Action;
    struct sigaction sigtermAction;

    sigusr1Action.sa_handler = SignalListener::sigusr1Callback;
    sigemptyset( &sigusr1Action.sa_mask );
    sigusr1Action.sa_flags = SA_RESTART;

    if ( sigaction( SIGUSR1, &sigusr1Action, NULL ) > 0 )
    {
        qCritical() << "Failed to set up signal handler for SIGUSR1";
    }

    sigtermAction.sa_handler = SignalListener::sigtermCallback;
    sigemptyset( &sigtermAction.sa_mask );
    sigtermAction.sa_flags = SA_RESTART;

    if ( sigaction( SIGTERM, &sigtermAction, NULL ) > 0 )
    {
        qCritical() << "Failed to set up signal handler for SIGTERM";
    }
}



int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    // Register custom message handler. This is useful for debugging when Shuriken is launched by NSM
    //qInstallMsgHandler( messageHandler );

    // Set style sheet
#if QT_VERSION < 0x040700
    QFile file( ":/resources/stylesheets/blue_theme_4.6.qss" );
#else
    QFile file( ":/resources/stylesheets/blue_theme.qss" );
#endif

    if ( file.open( QFile::ReadOnly ) )
    {
        const QString styleSheet = QLatin1String( file.readAll() );
        file.close();
        app.setStyleSheet( styleSheet );
    }

    // Create main window
    MainWindow window;

    // Set up signal handlers
    SignalListener signalListener;

    setupSignalHandlers();

    QObject::connect( &signalListener, SIGNAL( save() ),
                      &window, SLOT( on_actionSave_Project_triggered() ) );

    QObject::connect( &signalListener, SIGNAL( quit() ),
                      &window, SLOT( on_actionQuit_triggered() ) );

    // Show main window
    window.show();

    // Start application event loop
    const int exitValue = app.exec();

    return exitValue;
}
