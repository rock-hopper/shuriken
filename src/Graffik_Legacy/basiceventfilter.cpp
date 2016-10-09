#include "basiceventfilter.h"

BasicEventFilter::BasicEventFilter(QWidget* c_watch, QObject *parent) : QObject(parent) {
    m_watch = c_watch;
}


bool BasicEventFilter::eventFilter(QObject *p_obj, QEvent *p_event) {

    if( p_event->type() == QEvent::Resize ||
            p_event->type() == QEvent::ActivationChange ||
            p_event->type() == QEvent::Paint ||
            p_event->type() == QEvent::Show ||
            p_event->type() == QEvent::ApplicationActivate ||
            p_event->type() == QEvent::ContentsRectChange ||
            p_event->type() == QEvent::LayoutRequest ||
            p_event->type() == QEvent::MacSizeChange ||
            p_event->type() == QEvent::Polish ||
            p_event->type() == QEvent::StyleChange ||
            p_event->type() == QEvent::WindowActivate ||
            p_event->type() == QEvent::WindowUnblocked
            ) {

        m_watch->update();
    }


    return QObject::eventFilter( p_obj, p_event );
}
