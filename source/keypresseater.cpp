#include <QWidget>
#include <stdio.h>

#include "keypresseater.h"


Q_INVOKABLE KeyPressEater::KeyPressEater (QObject * parent)
		: QObject(parent)
{
}
KeyPressEater::~KeyPressEater()
{
}

 bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
 {
	  if (event->type() == QEvent::KeyPress) {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//printf("Ate key press %d. repeat %d. mod: %d\n", keyEvent->key(),(int)keyEvent->isAutoRepeat(),(int)keyEvent->modifiers());
			if(!keyEvent->isAutoRepeat() && keyEvent->modifiers()==Qt::NoModifier)
				emit Key(keyEvent->key());
			if(keyEvent->modifiers()==Qt::ControlModifier && keyEvent->key()==Qt::Key_C)
				emit Key(0);
			return true;
	  } else {
			// standard event processing
			return QObject::eventFilter(obj, event);
	  }
 }

