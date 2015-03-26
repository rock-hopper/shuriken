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

#include "nsmlistenerthread.h"
#include <QTimer>
#include <QApplication>
//#include <QtDebug>


//==================================================================================================
// Public:

NsmListenerThread::NsmListenerThread() :
    m_nsmClient( NULL )
{
    const char* nsmUrl = getenv( "NSM_URL" );

    if ( nsmUrl != NULL )
    {
        m_nsmClient = nsm_new();

        nsm_set_open_callback( m_nsmClient, openCallback, this );
        nsm_set_save_callback( m_nsmClient, saveCallback, this );

        if ( nsm_init( m_nsmClient, nsmUrl ) == ERR_OK )
        {
            m_isOpenComplete = false;

            nsm_send_announce( m_nsmClient, APPLICATION_NAME, ":dirty:", QApplication::argv()[ 0 ] );

            while ( m_isOpenComplete == false )
            {
                nsm_check_wait( m_nsmClient, 1000 );
            }
        }
        else // nsm could not be initialised
        {
            nsm_free( m_nsmClient );
            m_nsmClient = NULL;
        }
    }
}



NsmListenerThread::~NsmListenerThread()
{
    if ( m_nsmClient != NULL )
    {
        nsm_free( m_nsmClient );
        m_nsmClient = NULL;
    }
}



void NsmListenerThread::sendMessage( const Message message )
{
    if ( m_nsmClient != NULL )
    {
        m_mutex.lock();

        switch ( message )
        {
        case MSG_IS_CLEAN:
            nsm_send_is_clean( m_nsmClient );
            break;
        case MSG_IS_DIRTY:
            nsm_send_is_dirty( m_nsmClient );
            break;
        default:
            break;
        }

        m_mutex.unlock();
    }
}



//==================================================================================================
// Protected:

void NsmListenerThread::run()
{
    QTimer timer;

    if ( m_nsmClient != NULL )
    {
        connect( &timer, SIGNAL(timeout()), this, SLOT(checkForMessages()), Qt::DirectConnection );
        timer.start( 10 );
    }

    exec();

    if ( m_nsmClient != NULL )
    {
        timer.stop();
    }
}



//==================================================================================================
// Private Static:

int NsmListenerThread::openCallback( const char* savePath,
                                     const char* displayName,
                                     const char* clientId,
                                     char** outMessage,
                                     void* userData )
{
    NsmListenerThread* listenerThread = static_cast<NsmListenerThread*>( userData );

    listenerThread->m_savePath = savePath;
    listenerThread->m_clientId = clientId;
    listenerThread->m_isOpenComplete = true;

    return ERR_OK;
}



int NsmListenerThread::saveCallback( char** outMessage, void* userData )
{
    NsmListenerThread* listenerThread = static_cast<NsmListenerThread*>( userData );

    emit listenerThread->save();

    return ERR_OK;
}



//==================================================================================================
// Private Slots:

void NsmListenerThread::checkForMessages()
{
    m_mutex.lock();
    nsm_check_nowait( m_nsmClient );
    m_mutex.unlock();
}

