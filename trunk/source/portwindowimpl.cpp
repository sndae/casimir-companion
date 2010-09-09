/*
   DScope Application

   Copyright (C) 2008,2009:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QDebug>
#include "portwindowimpl.h"



#include <qextserialport/qextserialenumerator.h>
#include <qextserialport/qextserialport.h>

using namespace std;


PortWindowImpl::PortWindowImpl(QWidget * parent) 
	: QDialog(parent)
{
	
	setupUi(this);
	
	// Fill the dialog
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
	uitwPorts->setRowCount(ports.size());
	
	for (int i = 0; i < ports.size(); i++) {
		QTableWidgetItem *newItem;
		// Fix a bug of QExtSerialPort: portName is null terminated but string things it is larger. We truncate the string at null character. 
		QString s(ports.at(i).portName.toStdString().c_str());		
		newItem = new QTableWidgetItem(s);
		newItem->setFlags(Qt::NoItemFlags);
		uitwPorts->setItem(i,0, newItem);
		newItem = new QTableWidgetItem(ports.at(i).friendName);
		newItem->setFlags(Qt::NoItemFlags);
		uitwPorts->setItem(i,1, newItem);
		newItem = new QTableWidgetItem(ports.at(i).physName);
		newItem->setFlags(Qt::NoItemFlags);
		uitwPorts->setItem(i,2, newItem);
		newItem = new QTableWidgetItem(ports.at(i).enumName);
		newItem->setFlags(Qt::NoItemFlags);
		uitwPorts->setItem(i,3, newItem);
		
	}
	
}


