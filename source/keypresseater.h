#ifndef KEYPRESSEATER_H
#define KEYPRESSEATER_H


#include <QObject>
#include <QKeyEvent>
#include <QMutex>


 class KeyPressEater : public QObject
 {
	Q_OBJECT
	protected:
		bool eventFilter(QObject *obj, QEvent *event);
	public:
		Q_INVOKABLE KeyPressEater ( QObject * parent = 0 );
		virtual ~KeyPressEater ();

	signals:
		void Key(int key);
 };


#endif // KEYPRESSEATER_H
