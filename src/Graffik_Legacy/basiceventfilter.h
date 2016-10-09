/*

  Graffik Motion Control Application

  Copyright (c) 2011-2013 Dynamic Perception

 This file is part of Graffik.

    Graffik is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Graffik is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Graffik.  If not, see <http://www.gnu.org/licenses/>.

    */

#ifndef BASICEVENTFILTER_H
#define BASICEVENTFILTER_H

#include <QObject>
#include <QWidget>
#include <QEvent>

/** A basic EventFilter Class for Triggering Updates on All Events

  For certain widget types, drawing updates may not be effectively passed down.  They may use this
  event filter for capturing and updating on all draw-related events.

  This Event Filter should only be used when necessary.

  @author
  C. A. Church
  */

class BasicEventFilter : public QObject
{
    Q_OBJECT
public:
    BasicEventFilter(QWidget *c_watch, QObject *parent = 0);
    
signals:
    
public slots:
    
protected:
    bool eventFilter(QObject *p_obj, QEvent *p_event);

private:
    QWidget* m_watch;
};

#endif // BASICEVENTFILTER_H
