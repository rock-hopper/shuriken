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

#ifndef SIGNALLISTENER_H
#define SIGNALLISTENER_H

#include <QObject>
#include <QSocketNotifier>
#include "JuceHeader.h"


class SignalListener : public QObject
{
    Q_OBJECT

public:
    SignalListener( QObject* parent = NULL );

public:
    static void sigusr1Handler( int sigNum );
    static void sigtermHandler( int sigNum );

public slots:
    void handleSigusr1();
    void handleSigterm();

signals:
    void save();
    void quit();

private:
    ScopedPointer<QSocketNotifier> m_sigusr1Notifier;
    ScopedPointer<QSocketNotifier> m_sigtermNotifier;

    volatile bool m_isAppQuitting;

    static const int WRITE = 0;
    static const int READ = 1;
    static int sigusr1SocketID[ 2 ];
    static int sigtermSocketID[ 2 ];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( SignalListener );
};

#endif // SIGNALLISTENER_H
