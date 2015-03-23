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

#ifndef NSMLISTENERTHREAD_H
#define NSMLISTENERTHREAD_H

#include <QThread>
#include <QMutex>
#include "nonlib/nsm.h"
#include "JuceHeader.h"


class NsmListenerThread : public QThread
{
    Q_OBJECT

public:
    NsmListenerThread();
    ~NsmListenerThread();

    QString getJackClientId() const     { return m_clientId; }
    QString getSavePath() const         { return m_savePath; }

    enum Message { MSG_IS_CLEAN, MSG_IS_DIRTY };
    void sendMessage( Message message );

protected:
    void run();

private:
    nsm_client_t* m_nsmClient;

    QString m_clientId;
    QString m_savePath;

    bool m_isOpenComplete;

    QMutex m_mutex;

private:
    static int openCallback( const char* savePath,
                             const char* displayName,
                             const char* clientId,
                             char** outMessage,
                             void* userData );

    static int saveCallback( char** outMessage, void* userData );

signals:
    void save();

private slots:
    void checkForMessages();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( NsmListenerThread );
};


#endif // NSMLISTENERTHREAD_H
