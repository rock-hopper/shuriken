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

#include "signallistener.h"
#include <unistd.h>
#include <sys/socket.h>
//#include <QDebug>


int SignalListener::sigusr1SocketID[ 2 ];
int SignalListener::sigtermSocketID[ 2 ];


//==================================================================================================
// Public:

SignalListener::SignalListener( QObject* parent ) :
    QObject( parent ),
    m_isAppQuitting( false )
{
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, sigusr1SocketID ) )
    {
        qFatal( "Couldn't create SIGUSR1 socketpair" );
    }

    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, sigtermSocketID ) )
    {
        qFatal( "Couldn't create SIGTERM socketpair" );
    }

    m_sigusr1Notifier = new QSocketNotifier( sigusr1SocketID[ READ ], QSocketNotifier::Read, this );

    QObject::connect( m_sigusr1Notifier, SIGNAL( activated(int) ),
                      this, SLOT( handleSigusr1() ) );

    m_sigtermNotifier = new QSocketNotifier( sigtermSocketID[ READ ], QSocketNotifier::Read, this );

    QObject::connect( m_sigtermNotifier, SIGNAL( activated(int) ),
                      this, SLOT( handleSigterm() ) );
}



//==================================================================================================
// Public Static:

void SignalListener::sigusr1Handler( int /* sigNum */ )
{
    char c = 1;
    write( sigusr1SocketID[ WRITE ], &c, sizeof( c ) );
}



void SignalListener::sigtermHandler( int /* sigNum */ )
{
    char c = 1;
    write( sigtermSocketID[ WRITE ], &c, sizeof( c ) );
}



//==================================================================================================
// Public Slots:

void SignalListener::handleSigusr1()
{
    m_sigusr1Notifier->setEnabled( false );
    char c;
    read( sigusr1SocketID[ READ ], &c, sizeof( c ) );

    if ( ! m_isAppQuitting )
    {
        emit save();
    }

    m_sigusr1Notifier->setEnabled( true );
}



void SignalListener::handleSigterm()
{
    m_sigtermNotifier->setEnabled( false );
    char c;
    read( sigtermSocketID[ READ ], &c, sizeof( c ) );

    m_isAppQuitting = true;
    emit quit();

    m_sigtermNotifier->setEnabled( true );
}
