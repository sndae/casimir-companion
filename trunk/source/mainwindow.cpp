#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winbase.h>
#include <tchar.h>


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>
#include <QWaitCondition>
#include <QTime>
#include <QFileDialog>
#include <QTextStream>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <string>
#include "cio.h"
#include "lowlevel.h"
#include "sd.h"
#include "xcommand.h"
#include "portwindowimpl.h"
#include <qextserialport/qextserialenumerator.h>
#include <qextserialport/qextserialport.h>
#include "precisetimer.h"
#include "pkt.h"





MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
	 ui(new Ui::MainWindow),
	 comTitle("Device communication"),
	 logTitle("Export log"),
	 ke(this)
{

   ConsoleInit();
   printf("Starting up\n");

	port=0;

	PixmapLogging = QPixmap(":/bmp_rec2.png");
	PixmapStopped = QPixmap(":/bmp_paused2.png");
	PixmapNC = QPixmap(":/bmp_nc.png");

	ui->setupUi(this);

	// Set the current path
	ui->lineEdit_LogDirectory->setText(path.absolutePath());

	LoggerResetTimeStats();
	LoggerTimeOffset=0;

	SetDefaultLoggerStatus();
	timer_1 = startTimer(1000);   // 1-second timer
	timer_5 = startTimer(5000);   // 5-second timer

	// Labeling
	Labeling=false;

	connect(&ke,SIGNAL(Key(int)),this,SLOT(LabelKey(int)));
}

MainWindow::~MainWindow()
{
   delete ui;
}


void MainWindow::changeEvent(QEvent *e)
{
   QMainWindow::changeEvent(e);
   switch (e->type()) {
   case QEvent::LanguageChange:
     ui->retranslateUi(this);
     break;
   default:
     break;
   }
}


void MainWindow::on_pushButton_clicked()
{


}




void MainWindow::on_pushButton_2_clicked()
{
   /*QByteArray Volume("C:\\");

   EnumerateVolumeMountPoints(Volume);*/
   /*QString Dir;
   QStringList List;
   //Dir = QString("\\Fat");
   //Dir = QString("\\DosDevices");
   Dir = QString("\\Device");
   NativeDir(Dir,List);
   ParseNativeDir(List);*/
   //QStringList r;
   //GetListOfMountPoints(r);

   //t();

/*#ifdef UNICODE
#error unicode
#endif*/

   QStringList labels;
   labels << "Volume name" << "Mount point" << "Volume link" << "File system" << "Drive type" << "Disk size";
   ui->tableWidget->setColumnCount(labels.size());
   ui->tableWidget->setHorizontalHeaderLabels(labels);
   ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);



}


void MainWindow::on_pushButton_5_clicked()
{
    // NativeDir
   QStringList List;
   bool r = NativeDir(ui->lineEdit->text(),List);
   printf("<%s>\t",ui->lineEdit->text().toStdString().c_str());
   if(!r)
      printf("file not found\n");
   else
   {
      printf("\n");

      for(unsigned i=0;i<List.size();i++)
         printf("\t%s\n",List[i].toStdString().c_str());
   }
}

void MainWindow::on_pushButton_6_clicked()
{
   int GMediaType,GTracksPerCylinder,GSectorsPerTrack,GBytesPerSector;
   long long GCylinders,GDiskSize,GDiskSize2;
   GetGeometryEx(ui->lineEdit->text().toAscii(),GMediaType,GCylinders,GTracksPerCylinder,GSectorsPerTrack,GBytesPerSector,GDiskSize);
}


/******************************************************************************

******************************************************************************/
void MainWindow::GetAllVolumeDefinitions()
{
	std::vector<int> allowedtype;
	allowedtype.push_back(DRIVE_REMOVABLE);

	std::vector<std::string> allowedlink;
	allowedlink.push_back("Harddisk");

	AllVolumeDefinitions = GetListOfVolumes(allowedtype,allowedlink);

	printf("All volumes:\n");
	for(unsigned i=0;i<AllVolumeDefinitions.size();i++)
	{
		printf("Volume#%d. volumeName %s\n",i, AllVolumeDefinitions[i].volumeName.data());
		printf("\tname '%s'\n",AllVolumeDefinitions[i].Name.data());
		printf("\tfilesystem '%s'\n",AllVolumeDefinitions[i].fileSystem.data());
		printf("\tmountPoint %s\n",AllVolumeDefinitions[i].mountPoint.data());
		printf("\tvolumeLink '%s'\n",AllVolumeDefinitions[i].volumeLink.data());
		printf("\tdriveType: %d \tSN: %X \tFlags: %X\n",AllVolumeDefinitions[i].driveType,AllVolumeDefinitions[i].serialNumber,AllVolumeDefinitions[i].dwSysFlags);
		printf("\tMediaType %d\n",AllVolumeDefinitions[i].MediaType);
		printf("\tCylinders %lld. Tracks %d. Sectors/track %d. Bytes/sector %d.\n",AllVolumeDefinitions[i].Cylinders,AllVolumeDefinitions[i].TracksPerCylinder,AllVolumeDefinitions[i].SectorsPerTrack,AllVolumeDefinitions[i].BytesPerSector);
		printf("\tDisk size %lld. Alternate disk size: %lld\n",AllVolumeDefinitions[i].DiskSize,AllVolumeDefinitions[i].DiskSize2);
		if(AllVolumeDefinitions[i].driveType==DRIVE_REMOVABLE)
			printf("\tRemovable\n");
		printf("\n\n");
	}

	// Select the suitable volumes (removable, with a non-null and non-negative size)
	SuitableVolumeDefinitions.clear();
	for(unsigned i=0;i<AllVolumeDefinitions.size();i++)
	{
		if(AllVolumeDefinitions[i].driveType == DRIVE_REMOVABLE)
		{
			if(AllVolumeDefinitions[i].DiskSize!=-1 && AllVolumeDefinitions[i].DiskSize!=0)
				SuitableVolumeDefinitions.push_back(AllVolumeDefinitions[i]);
		}
	}

	// No current volume
	currentvolume=-1;


}
void MainWindow::DisplayFileSystem()
{
	ui->tableWidget_FileSystem->clear();
	QStringList labels;
	labels << "Log#" << "Device" << "Time" << "Start block" << "End block" << "Size";
	ui->tableWidget_FileSystem->setColumnCount(labels.size());
	ui->tableWidget_FileSystem->setHorizontalHeaderLabels(labels);
	ui->tableWidget_FileSystem->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	if(!filesystem.ok)
	{
		ui->tableWidget_FileSystem->setRowCount(0);
		ui->pushButton_ReadAllLogs->setEnabled(false);
		//ui->pushButton_ReadSelectedLog->setEnabled(false);
		return;
	}
	ui->tableWidget_FileSystem->setRowCount(filesystem.logentry.size());
	for(unsigned i=0;i<filesystem.logentry.size();i++)
	{
		QTableWidgetItem *newItem;
		newItem = new QTableWidgetItem(QString("%1").arg(i));
		ui->tableWidget_FileSystem->setItem(i,0, newItem);
		newItem = new QTableWidgetItem(QString("").sprintf("%08X",filesystem.logentry[i].device));
		ui->tableWidget_FileSystem->setItem(i,1, newItem);
		newItem = new QTableWidgetItem(QString("%1").arg(filesystem.logentry[i].starttime));
		ui->tableWidget_FileSystem->setItem(i,2, newItem);
		newItem = new QTableWidgetItem(QString("%1").arg(filesystem.logentry[i].blockstart));
		ui->tableWidget_FileSystem->setItem(i,3, newItem);
		newItem = new QTableWidgetItem(QString("%1").arg(filesystem.logentry[i].blockend));
		ui->tableWidget_FileSystem->setItem(i,4, newItem);
		newItem = new QTableWidgetItem(QString("%1").arg(filesystem.logentry[i].blockend-filesystem.logentry[i].blockstart+1));
		ui->tableWidget_FileSystem->setItem(i,5, newItem);
	}
	if(filesystem.logentry.size())
		ui->pushButton_ReadAllLogs->setEnabled(true);
	else
		ui->pushButton_ReadAllLogs->setEnabled(false);


}

void MainWindow::DisplaySuitableVolumeDefinitions()
{
	ui->tableWidget->clear();
	QStringList labels;
	labels << "Volume name" << "Mount point" << "Volume link" << "File system" << "Drive type" << "Disk size";
	ui->tableWidget->setColumnCount(labels.size());
	ui->tableWidget->setHorizontalHeaderLabels(labels);
	ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui->tableWidget->setRowCount(SuitableVolumeDefinitions.size());
	for(unsigned i=0;i<SuitableVolumeDefinitions.size();i++)
	{
		QTableWidgetItem *newItem;
		newItem = new QTableWidgetItem(QString(SuitableVolumeDefinitions[i].volumeName));
		ui->tableWidget->setItem(i,0, newItem);
		newItem = new QTableWidgetItem(QString(SuitableVolumeDefinitions[i].mountPoint));
		ui->tableWidget->setItem(i,1, newItem);
		newItem = new QTableWidgetItem(QString(SuitableVolumeDefinitions[i].volumeLink));
		ui->tableWidget->setItem(i,2, newItem);
		newItem = new QTableWidgetItem(QString(SuitableVolumeDefinitions[i].fileSystem));
		ui->tableWidget->setItem(i,3, newItem);
		newItem = new QTableWidgetItem(QString("%1").arg(SuitableVolumeDefinitions[i].driveType));
		ui->tableWidget->setItem(i,4, newItem);
		newItem = new QTableWidgetItem(QString("%1").arg(SuitableVolumeDefinitions[i].DiskSize));
		ui->tableWidget->setItem(i,5, newItem);
	}
	if(SuitableVolumeDefinitions.size())
		ui->tableWidget->selectRow(0);


	printf("Done\n\n");

}
void MainWindow::ReadFileSystem(int volume)
{
	if(volume<0)
	{
		filesystem.ok=false;
		DisplayFileSystem();
		return;
	}
	printf("Reading data from %s %s\n",SuitableVolumeDefinitions[volume].volumeName.data(),SuitableVolumeDefinitions[volume].volumeLink.data());
	unsigned char data[512];
	bool rv;
	int read;
	LowLevelFile file(SuitableVolumeDefinitions[volume].volumeLink);
	rv = file.open();
	rv = file.read(512,data,read);
	if(rv)
	{
		for(unsigned j=0;j<32;j++)
		{
			printf("%08X\t",j*16);
			for(unsigned i=0;i<16;i++)
			{
				printf("%02X ",data[j*16+i]);
			}
			printf("\n");
		}

		int rv2 = fs_parseroot(data,filesystem);
		DisplayFileSystem();
	}
	else
		printf("Error reading data\n");

}
//bool MainWindow::CopyRaw(QByteArray src,QByteArray dst,int blockstart,int blockend)
bool MainWindow::CopyRaw(QByteArray src,QByteArray dst,int entry)
{
   int copyunitblock = 128;
   int size;
   unsigned char buffer[copyunitblock*512];
	unsigned blockstart = filesystem.logentry[entry].blockstart;
	unsigned blockend = filesystem.logentry[entry].blockend;
   int nblockstocopy = blockend-blockstart+1;      // Number of blocks to copy
   int nblockscopied = 0;                          // Blocks copied so far
	bool ok;
	int len;
	qint64 len64;
   QString title = QString("Exporting log on %1[%2-%3] to %4").arg(QString(src)).arg(blockstart).arg(blockend).arg(QString(dst));


   // Input / output files
	LowLevelFile filein(src);
	filein.open();
	QFile fileout(dst);
	fileout.open(QIODevice::WriteOnly|QIODevice::Truncate);

   // Progress dialog
   QProgressDialog pd(title,"Abort export", 0, nblockstocopy, this,Qt::Tool);
   pd.setWindowModality(Qt::WindowModal);
   pd.setAutoReset(false);

	// First copy the ROOT
	strcpy((char*)buffer,"LOG");
	fileout.write((char*)buffer,4);
   memset(buffer,0,10);
   fileout.write((char*)buffer,10);
	// Copy the log entry
	printf("Writing logentry. Sizeof: %d\n",sizeof(fs_logentry));
   printf("sizeof logentry: %d\n",sizeof(fs_logentry));
	fileout.write((char*)&filesystem.logentry[entry],sizeof(fs_logentry));


   unsigned long long o,no;
   o = blockstart;
   o<<=9;
   ok = filein.seek(o,no);
   if(!ok)
   {
      QMessageBox::critical(this,title,"Error seeking card");
      return false;
   }

   printf("Copying %d blocks\n",nblockstocopy);
   while(nblockscopied < nblockstocopy)
   {
      if (pd.wasCanceled())
         break;
      pd.setValue(nblockscopied);

      // See how many blocks we can copy
      //printf("unit: %d. nblockscopied: %d. nblockstocopy: %d\n",copyunitblock,nblockscopied,nblockstocopy);
      while( nblockscopied+copyunitblock > nblockstocopy)
      {
         copyunitblock>>=1;
         //printf("Resize unit. unit: %d. nblockscopied: %d. nblockstocopy: %d\n",copyunitblock,nblockscopied,nblockstocopy);
      }
      //printf("copyunitblock is now: %d\n",copyunitblock);

      size = copyunitblock*512;
      ok = filein.read(size,buffer,len);
      if(!ok || len!=size)
      {
         QMessageBox::critical(this,title,"Error reading card");
         return false;
      }
      len64 = fileout.write((char*)buffer,size);
      if(len64!=size)
      {
         QMessageBox::critical(this,title,"Error writing log");
         return false;
      }
      nblockscopied += copyunitblock;
   }
   bool cancelled = pd.wasCanceled();
   printf("Copy done. cancelled: %d\n",cancelled);
	filein.close();
	fileout.close();
	pd.close();
   if(cancelled)
      return false;
	return true;
}


/******************************************************************************
*******************************************************************************
UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI
*******************************************************************************
******************************************************************************/
void MainWindow::on_pushButton_RefreshVolumes_clicked()
{
	// Search for folumes
	GetAllVolumeDefinitions();
	// Display them
	DisplaySuitableVolumeDefinitions();
	// Read and display the filesystem
	ReadFileSystem(currentvolume);
}

void MainWindow::on_tableWidget_itemSelectionChanged()
{
	printf(":on_tableWidget_itemSelectionChanged\n");
	QList<QTableWidgetItem *> si = ui->tableWidget->selectedItems();
	if(si.size())
	{
		currentvolume = si[0]->row();
		printf("Loading filesystem from device %d\n",currentvolume);
		ReadFileSystem(currentvolume);
	}
	else
		currentvolume=-1;

}


void MainWindow::on_pushButton_ReadFileSystem_clicked()
{
	 ReadFileSystem(currentvolume);
}


bool MainWindow::CopyLog(int i,QString dst)
{
	bool ok;

	// Check if we overwrite an existing file, and we don't have the option overwrite w/o prompting set
	QFile fileout(dst);
	if(ui->checkBox_LogOverwrite->isChecked()==false && fileout.exists())
	{
		if(QMessageBox::question(this,logTitle,QString("File %1 already exist. Are you sure you want to overwrite it?").arg(QString(dst)), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
			return false;
	}
	//ok = CopyRaw(SuitableVolumeDefinitions[currentvolume].volumeLink,dst.toAscii(),filesystem.logentry[i].blockstart,filesystem.logentry[i].blockend);
	ok = CopyRaw(SuitableVolumeDefinitions[currentvolume].volumeLink,dst.toAscii(),i);
	if(!ok)
	{
		QMessageBox::critical(this,logTitle,QString("Could not save log entry %1 to %2").arg(i).arg(dst));
		return false;
	}
	return true;
}

void MainWindow::on_pushButton_ReadAllLogs_clicked()
{
	for(unsigned i=0;i<filesystem.logentry.size();i++)
	{
		ProcessLog(i);
	}
}
void MainWindow::on_pushButton_ReadSelectedLog_clicked()
{
	ProcessLog(currentlog);

}
void MainWindow::ProcessLog(int currentlog)
{
	bool rv;
	// Copy the log to the disk (raw mode)
	QString dst;
	if(ui->checkBox_LogIncludeDeviceID->isChecked())
		dst.sprintf("log_%08X_%02d.bin",filesystem.logentry[currentlog].device,currentlog);
	else
		dst.sprintf("log_%02d.bin",currentlog);
	rv = CopyLog(currentlog,dst);
	// Demux the log, if the copy was successful
	if(rv && ui->checkBox_LogDemux->isChecked())
	{
		DemuxLog(dst);

	}


}

void MainWindow::on_tableWidget_FileSystem_itemSelectionChanged()
{
	QList<QTableWidgetItem *> si = ui->tableWidget_FileSystem->selectedItems();
	if(si.size()==0)
	{
		ui->pushButton_ReadSelectedLog->setEnabled(false);
		currentlog=-1;
	}
	else
	{
		ui->pushButton_ReadSelectedLog->setEnabled(true);
		currentlog=si[0]->row();
	}
}



void MainWindow::on_actionList_ports_triggered()
{
	PortWindowImpl dialog(this);
	int ret = dialog.exec();
	if (ret == QDialog::Rejected)
		return;
}

/******************************************************************************
*******************************************************************************
Device communication   Device communication   Device communication
*******************************************************************************
******************************************************************************/

void MainWindow::SetDefaultLoggerStatus()
{
	ui->label_LoggerBattery->setText("N/A");
	ui->label_LoggerID->setText("N/A");
	ui->label_LoggerLogs->setText("N/A");
	ui->label_LoggerMemCapacity->setText("N/A");
	ui->label_LoggerMemUsed->setText("N/A");
	ui->label_LoggerTimeDev->setText("N/A");
	ui->label_LoggerTimeOffset->setText("N/A");
	ui->label_LoggerTimeOffset_2->setText("N/A");
	ui->label_LoggerTimeTick->setText("N/A");
	ui->label_LoggerStatus->setPixmap(PixmapNC);
}

/******************************************************************************
	Connect to the device
******************************************************************************/
bool MainWindow::LoggerConnect(QString portstring)
{
//	printf("Create port %s\n",portstring.toStdString().c_str());

	//port = new QextSerialPort(portstring,QextSerialPort::EventDriven);
	port = new QextSerialPort(portstring,QextSerialPort::Polling);

	port->open(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Append);
	//port->open(QIODevice::WriteOnly);

	if(port->isOpen())
	{
		port->setBaudRate(BAUD115200);
		port->setDataBits(DATA_8);
		port->setFlowControl(FLOW_OFF);
		port->setParity(PAR_NONE);
		port->setStopBits(STOP_1);
		port->setTimeout(500);
		connect(port,SIGNAL(readyRead()),this,SLOT(serialRead()));
		connect(port,SIGNAL(aboutToClose()),this,SLOT(serialAboutToClose()));
		connect(port,SIGNAL(dsrChanged(bool)),this,SLOT(serialDSRChanged(bool)));
		connect(port,SIGNAL(bytesWritten(qint64)),this,SLOT(serialBytesWritten(qint64)));

		ui->pushButton_LoggerConnect->setText("Dis&connect");
		return true;
	}
	return false;
}
void MainWindow::on_pushButton_LoggerConnect_clicked()
{
	if(port && port->isOpen())
	{
		LoggerDisconnect();
	}
	else
	{
		QString p = ui->lineEdit_ComPort->text();
		// Quick sanity check: there should be no terminating colon
		if(p.contains(":"))
		{
			QMessageBox::critical(this,comTitle,QString("The com port should be in the following format:\n   com<n>\t Windows. Connects to com port n (no colon).\n   /dev/<dev>\t Linux. Connects to dev."));
			return;
		}

		QString portstring=QString("\\\\.\\%1").arg(p);
		if(!LoggerConnect(portstring))
		{
			QMessageBox::critical(this,comTitle,QString("Could not connect to device %1").arg(portstring));
			LoggerDisconnect();
		}
		else
		{
			on_pushButton_LoggerReadAllStatus_clicked();
		}
	}
}


void MainWindow::serialRead()
{
	XCOMMAND cmd;

	if(!port || !port->isOpen())
		return;

	QByteArray ba=port->readAll();

	for(int i=0;i<ba.size();i++)
	{
		// Check if we have a response to a command
		for(unsigned char i=0;i<XCOMMAND_SIZE-1;i++)
			xcommand_buffer[i]=xcommand_buffer[i+1];
		xcommand_buffer[XCOMMAND_SIZE-1]=ba[i];
		if(xcommand_buffer[0]=='S' &&
			xcommand_buffer[XCOMMAND_SIZE-2]=='X' &&
			xcommand_buffer[XCOMMAND_SIZE-1]=='E')
		{
			// Got a response to a command
			cmd.cmd = xcommand_buffer[1];
			memcpy(cmd.payload,xcommand_buffer+2,XCOMMAND_SIZE-4);
			cmd.xsum = xcommand_buffer[XCOMMAND_SIZE-2];
			LoggerReceivedXCommand(cmd);
		}
	}


/*	printf("Read %d bytes:",ba.size());
	for(int i=0;i<ba.size();i++)
		printf("%c",(int)ba[i]);
	printf("\n");*/
}

bool MainWindow::ReadXCommand(std::vector<XCOMMAND> &cmdv)
{
	XCOMMAND cmd;

	if(!port || !port->isOpen())
		return false;

	QByteArray ba=port->readAll();

	for(int i=0;i<ba.size();i++)
	{
		// Check if we have a response to a command
		for(unsigned char i=0;i<XCOMMAND_SIZE-1;i++)
			xcommand_buffer[i]=xcommand_buffer[i+1];
		xcommand_buffer[XCOMMAND_SIZE-1]=ba[i];
		if(xcommand_buffer[0]=='S' &&
			xcommand_buffer[XCOMMAND_SIZE-2]=='X' &&
			xcommand_buffer[XCOMMAND_SIZE-1]=='E')
		{
			// Got a response to a command
			cmd.cmd = xcommand_buffer[1];
			memcpy(cmd.payload,xcommand_buffer+2,XCOMMAND_SIZE-4);
			cmd.xsum = xcommand_buffer[XCOMMAND_SIZE-2];
			cmdv.push_back(cmd);
		}
	}

	if(cmdv.size()==0)
		return false;

	for(int i=0;i<cmdv.size();i++)
		LoggerReceivedXCommand(cmdv[i]);

	return true;

}
// Received the time: update the UI, and start the open-loop estimation of the device time
void MainWindow::LoggerReceivedTime(unsigned to,unsigned tt)
{
	printf("Time offset: %u. Time tick: %u\n",to,tt);

	LoggerTimeOffset = to;

	double ct = GetCurrentTime();

	// Get the estimated current device time...
	unsigned estl = LoggerEstimateCurrentTick(1);
	unsigned estf = LoggerEstimateCurrentTick(0);
	double elapsedl = ct-PCTimeLastSync;
	double elapsedf = ct-PCTimeFirstSync;

	// Reset the time estimatation to the new data
	if(TimeSyncd==false)
	{
		LoggerTimeFirstSync = tt;
		PCTimeFirstSync = ct;
	}
	LoggerTimeLastSync = tt;
	PCTimeLastSync = ct;

	// Compute the tick difference...
	signed diffl,difff;
	diffl = (signed)((signed long)estl-(signed long)tt);
	difff = (signed)((signed long)estf-(signed long)tt);
	double diffperdayl = (double)diffl * 24.0*3600.0 / elapsedl;
	double diffperdayf = (double)difff * 24.0*3600.0 / elapsedf;


	ui->label_LoggerTimeOffset->setText(QString().sprintf("%010u",to));
	ui->label_LoggerTimeOffset_2->setText(QString().sprintf("%010u",to));
	ui->label_LoggerTimeTick->setText(QString().sprintf("%010u",tt));
	ui->label_LoggerTimeTick_2->setText(QString().sprintf("%010u",tt));
	if(TimeSyncd)
		ui->label_LoggerTimeDev->setText(QString().sprintf("%d (%.1lf \"/day). %d (%.1lf \"/day)",diffl,diffperdayl/8000.0,difff,diffperdayf/8000.0));
	else
		ui->label_LoggerTimeDev->setText("N/A");

	TimeSyncd=true;

}
void MainWindow::LoggerReceivedLoggingStatus(bool logging)
{
	if(logging)
		ui->label_LoggerStatus->setPixmap(PixmapLogging);
	else
		ui->label_LoggerStatus->setPixmap(PixmapStopped);
}

void MainWindow::LoggerReceivedXCommand(XCOMMAND xcmd)
{
	char cmd = xcmd.cmd&0xDF;					// Mask the success bit
	bool success = (xcmd.cmd&0x20)?false:true;	// Success bit
	printf("Received XCOMMAND %c. Success: %d\n",cmd,(int)success);

	switch(cmd)
	{
		case 'I':
			ui->label_LoggerID->setText(QString().sprintf("%04X",*(unsigned *)xcmd.payload));
			break;
		case 'T':
			unsigned to,tt;
			to = *(unsigned *)xcmd.payload;
			tt = *(unsigned *)(xcmd.payload+4);
			LoggerReceivedTime(to,tt);
			LoggerReceivedLoggingStatus(xcmd.payload[8]);
			break;
		case 'F':
			ui->label_LoggerMemCapacity->setText(QString().sprintf("%010u",*(unsigned *)xcmd.payload));
			ui->label_LoggerMemUsed->setText(QString().sprintf("%010u",*(unsigned *)(xcmd.payload+4)));
			ui->label_LoggerLogs->setText(QString().sprintf("%u",(int)xcmd.payload[8]));
			ui->label_LoggerBattery->setText(QString().sprintf("%04u mV",*(short*)(xcmd.payload+10)));
			LoggerReceivedLoggingStatus(xcmd.payload[9]);

			break;
		case 'L':
			printf("Start log: %s\n",success?"success":"fail");
			break;
		case 'E':
			printf("Stop log: %s\n",success?"success":"fail");
			break;
		case 'K':
			printf("Format: %s\n",success?"success":"fail");
			break;
		case 'S':
			printf("Set time: %s\n",success?"success":"fail");
			break;
		case 'J':
			printf("Set ID: %s\n",success?"success":"fail");
			break;
		default:
			printf("Unknown answer to command %d\n",(int)cmd);
	}

}

void MainWindow::serialDSRChanged(bool s)
{
	printf("dsrChanged status: %d\n",s);
}

void MainWindow::serialAboutToClose()
{
	printf("MainWindow::aboutToClose\n");
}
void MainWindow::serialBytesWritten(qint64 n)
{
	printf("serialBytesWritten: %ld\n",n);
}


bool MainWindow::LoggerCheckConnected(bool displayerror)
{
	if(port==0 || !port->isOpen())
	{
		if(displayerror)
			QMessageBox::critical(this,comTitle,QString("Connect to device first"));
		return false;
	}
	return true;
}
bool MainWindow::LoggerSendXCommand(XCOMMAND cmd)
{
	char c;
	int i;

	c='S';
	i=port->write(&c,1);
	//printf("Send %d",i);
	i=port->write((char*)&cmd.cmd,1);
	//printf(", %d",i);
	i = port->write((char*)cmd.payload,XCOMMAND_SIZE-4);
	//printf(", %d",i);
	c='X';
	i=port->write(&c,1);
	//printf(", %d",i);
	c='E';
	i=port->write(&c,1);
	//printf(", %d\n",i);

	return true;
}
bool MainWindow::LoggerSendXCommandWait(XCOMMAND cmd,bool displayerror, double timeout)
{
	LoggerSendXCommand(cmd);

	// Wait for an answer...
	double t1;
	bool rv=false;
	std::vector<XCOMMAND> cmdv;
	t1 = PreciseTimer::QueryTimer();
	if(1)
	{
		while(PreciseTimer::QueryTimer()-t1<timeout && rv==false)
		{
			rv = ReadXCommand(cmdv);
		}
		if(rv==false)
		{
			if(displayerror)
				QMessageBox::critical(this,comTitle,"Communication with device failed: no answer.");
			return false;
		}
		//printf("Got answer: %d packets\n",cmdv.size());
	}
	return true;
}

/******************************************************************************
	Device command: Get device ID
******************************************************************************/
void MainWindow::on_pushButton_LoggerGetID_clicked()
{
	if(!LoggerCheckConnected())
		return;

	LoggerCmdGetID();

}

bool MainWindow::LoggerCmdGetID(bool displayerror,double timeout)
{
	XCOMMAND cmd;
	cmd.cmd = 'I';

	return LoggerSendXCommandWait(cmd,displayerror,timeout);

}
bool MainWindow::LoggerCmdGetFlash(bool displayerror,double timeout)
{
	XCOMMAND cmd;
	cmd.cmd = 'F';

	return LoggerSendXCommandWait(cmd,displayerror,timeout);
}
bool MainWindow::LoggerCmdGetTime(bool displayerror,double timeout)
{
	XCOMMAND cmd;
	cmd.cmd = 'T';

	return LoggerSendXCommandWait(cmd,displayerror,timeout);
}
bool MainWindow::LoggerCmdSetTime(unsigned time)
{
	XCOMMAND cmd;
	cmd.cmd = 'S';
	*(unsigned*)cmd.payload = time;

	return LoggerSendXCommandWait(cmd);
}

/******************************************************************************
	Device command: Get Flash state
******************************************************************************/
void MainWindow::on_pushButton_LoggerGetFlash_clicked()
{
	if(!LoggerCheckConnected())
		return;
	LoggerCmdGetFlash();
}


void MainWindow::on_pushButton_LoggerGetTime_clicked()
{
	if(!LoggerCheckConnected())
		return;
	LoggerCmdGetTime();
}



void MainWindow::on_pushButton_LoggerStart_clicked()
{
	if(!LoggerCheckConnected())
		return;
	XCOMMAND cmd;
	cmd.cmd = 'L';
	LoggerSendXCommandWait(cmd);
}

void MainWindow::on_pushButton_LoggerStop_clicked()
{
	if(!LoggerCheckConnected())
		return;
	XCOMMAND cmd;
	cmd.cmd = 'E';
	LoggerSendXCommandWait(cmd);
}

void MainWindow::on_pushButton_LoggerFormat_clicked()
{
	if(!LoggerCheckConnected())
		return;
	XCOMMAND cmd;
	cmd.cmd = 'K';
	LoggerSendXCommandWait(cmd);
}

void MainWindow::on_pushButton_LoggerSetID_clicked()
{
	if(QMessageBox::question(this,comTitle,"The device ID ought to be a unique number.\nAre you sure you want to change the device ID?", QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
		return;

	if(!LoggerCheckConnected())
		return;
	XCOMMAND cmd;
	cmd.cmd = 'J';

	bool ok;
	unsigned did;
	int n;
	QString str = ui->lineEdit_Logger_NewID->text();
	n=sscanf(str.toStdString().c_str(),"%x",&did);
	printf("n: %d. %lX %lx\n",n,did,did>>32);
	if(n!=1)
	{
		QMessageBox::critical(this,comTitle,QString("Invalid device ID. The ID must be a 32-bit hex number.\nFor instance: 55AADE68, 1, 00000001, ..."));
		return;
	}
	*(unsigned *)cmd.payload = did;

	LoggerSendXCommandWait(cmd);
}


// label_LoggerDeviceTime
unsigned MainWindow::LoggerEstimateCurrentTick(bool last)
{
	if(!TimeSyncd)
		return 0;
	double t = GetCurrentTime();
	if(last)
		return LoggerTimeLastSync + (unsigned)((t-PCTimeLastSync)*8000.0);
	return LoggerTimeFirstSync + (unsigned)((t-PCTimeFirstSync)*8000.0);
}
double MainWindow::LoggerEstimateCurrentTime(bool last)
{
	if(!TimeSyncd)
		return 0;
	unsigned et = LoggerEstimateCurrentTick(last);
	return LoggerTimeOffset + et/8000.0;
}
double MainWindow::GetCurrentTime()
{
	//return PreciseTimer::QueryTimer();
	return CurrentTime.GetTime();
}

void MainWindow::timerEvent ( QTimerEvent * event )
{
	//static double t=0,tavg=0;
	//double nt = PreciseTimer::QueryTimer();

	if(event->timerId()==timer_1)
	{
		// One second timer: empty the serial buffer, display estimated time
		//tavg = 0.9*tavg+0.1*(nt-t);

		//printf("Timer event %lf. Elapsed: %lf (%lf). Estimated device time: %u\n",nt,nt-t,tavg,LoggerEstimateCurrentTick());
		//printf("DTime: %lf\n",CurrentTime.GetTime());

		//t = nt;
		if(TimeSyncd)
		{
			ui->label_LoggerTimeTick->setText(QString().sprintf("%010u",LoggerEstimateCurrentTick()));
			ui->label_LoggerTimeTick_2->setText(QString().sprintf("%010u",LoggerEstimateCurrentTick()));
		}
		else
		{
			ui->label_LoggerTimeTick->setText("N/A");
			ui->label_LoggerTimeTick_2->setText("N/A");
		}
		serialRead();
	}
	if(event->timerId()==timer_5)
	{
		// Five second timer: read device status

		if(!LoggerCheckConnected(false))
			return;
		printf("Calling read all with displayerror false\n");
		if(!LoggerReadAll(false))
		{
			// Problem: we didn't manage to communicate....
			QMessageBox::critical(this,comTitle,QString("Communication error with device... disconnecting. Was the device removed from the slot or turned off?\n\nReconnect after checking this."));
			LoggerDisconnect();
		}
	}
}









bool MainWindow::LoggerReadAll(bool displayerror)
{
	if(!LoggerCmdGetID(displayerror))
		return false;
	if(!LoggerCmdGetFlash(displayerror))
		return false;
	if(!LoggerCmdGetTime(displayerror))
		return false;
	return true;
}

void MainWindow::on_pushButton_LoggerReadAllStatus_clicked()
{
	if(!LoggerCheckConnected())
		return;

	LoggerReadAll();

}


void MainWindow::LoggerDisconnect()
{
	ui->pushButton_LoggerConnect->setText("&Connect");
	if(!port)
		return;
	if(port->isOpen())
		port->close();
	delete port;
	port=0;
	SetDefaultLoggerStatus();
}

void MainWindow::on_pushButton_LoggerAutodetect_clicked()
{
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
	ui->lineEdit_ComPort->setText("");



	for (int i = 0; i < ports.size(); i++)
	{
		LoggerDisconnect();

		bool rv;
		printf("Probing port %s\n",ports[i].portName.toStdString().c_str());
		rv = LoggerConnect(ports.at(i).portName);
		printf("Connect: %d\n",(int)rv);
		if(rv)
		{
			rv = LoggerCmdGetID(false,0.05);
			if(rv)
			{
				printf("Found device on port %s\n",ports[i].portName.toStdString().c_str());
				ui->lineEdit_ComPort->setText(ports[i].portName);
				on_pushButton_LoggerReadAllStatus_clicked();
				return;
			}

			printf("No device on port %s\n",ports[i].portName.toStdString().c_str());
		}
	}
	QMessageBox::critical(this,comTitle, "No device detected");
}

void MainWindow::LoggerResetTimeStats()
{
	LoggerTimeLastSync=0;
	LoggerTimeFirstSync=0;
	PCTimeLastSync=0;
	PCTimeFirstSync=0;
	TimeSyncd=false;
	ui->label_LoggerTimeTick->setText("N/A");
	ui->label_LoggerTimeTick_2->setText("N/A");
	ui->label_LoggerTimeDev->setText("N/A");
}

void MainWindow::on_pushButton_LoggerResetTimeEstimate_clicked()
{
	// Ask the user whether it's okay to stop the time estimate only when we are not connected to the device.
	// If we are connected, we simply reset the estimate, which will continue with the data obtained at the next device readout.
	if(!LoggerCheckConnected(false))
		if(QMessageBox::question(this,comTitle,"By stopping the device time estimation the application will loose time synchronization with the logger. Labeling with the device time will not be possible anymore.\n\nAre you sure you want to stop the device time estimation?", QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
			return;
	LoggerResetTimeStats();
}

void MainWindow::on_pushButton_LoggerResetTime_clicked()
{
	LoggerCmdSetTime(0);
	LoggerResetTimeStats();
	LoggerCmdGetTime();
}

void MainWindow::on_pushButton_LoggerSetTime_clicked()
{
	// Set the time to the current PC time...
	QDateTime t0;
	t0.setTime_t(0);
	QDateTime ct = QDateTime::currentDateTime();
	int dt = t0.secsTo(ct)+1;
	// Wait until time changes to the next second
	printf("Wait until %d\n",dt);
	while(t0.secsTo(QDateTime::currentDateTime())<dt)
		//printf("\t%d\n",t0.secsTo(QDateTime::currentDateTime()));
		;
	printf("Time of device was set to offset %d\n",dt);
	printf("Time of PC is: %lf\n",CurrentTime.GetTime());
	LoggerCmdSetTime(dt);
	LoggerResetTimeStats();
	LoggerCmdGetTime();
}







void MainWindow::LabelKey(int key)
{
	if(key==0)
		StartStopLabeling();
	else
	{
		std::pair<double,double> t;
		std::pair<std::pair<double,double>,int> m;
		t.first = CurrentTime.GetTime();
		//t.second = LoggerTimeOffset + LoggerEstimateCurrentTick(0)/8000.0;
		t.second = LoggerEstimateCurrentTime();
		m.first = t;
		m.second = key;
		Labels.push_back(m);
		DisplayLabels();
	}
}

void MainWindow::StartStopLabeling()
{
	if(!Labeling)
	{
		ui->label_LogCurrentLabel->grabKeyboard();
		ui->label_LogCurrentLabel->installEventFilter(&ke);
		Labeling=true;
		ui->pushButton_LabelStartStop->setText("&Stop labeling (or [ctrl-C])");
	}
	else
	{
		ui->label_LogCurrentLabel->removeEventFilter(&ke);
		ui->label_LogCurrentLabel->releaseKeyboard();
		Labeling=false;
		ui->pushButton_LabelStartStop->setText("&Start labeling");
	}
}
void MainWindow::DisplayLabels()
{
	// Display the label in the list
	ui->listWidget_LogLabels->clear();
	for(unsigned i=0;i<Labels.size();i++)
	{
		QString str = QString().sprintf("%.01lf \t %.01lf \t %u",Labels[i].first.first,Labels[i].first.second,Labels[i].second);
		ui->listWidget_LogLabels->addItem(str);
	}
	if(ui->listWidget_LogLabels->count())
		ui->listWidget_LogLabels->setCurrentRow(ui->listWidget_LogLabels->count()-1);

	// Display the last label in large font
	if(Labels.size())
		ui->label_LogCurrentLabel->setText(QString("%1").arg(Labels[Labels.size()-1].second));
	else
		ui->label_LogCurrentLabel->setText("");
}

void MainWindow::on_pushButton_LabelStartStop_clicked()
{
	StartStopLabeling();
}



void MainWindow::on_pushButton_LabelClear_clicked()
{
	if(QMessageBox::question(this,"Labling","Are you sure you want to erase all the labels?", QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
		return;
	 Labels.clear();
	 DisplayLabels();
}

void MainWindow::on_pushButton_LabelSave_clicked()
{
	QString saveTitle="Save labels";
	QString fileName = QFileDialog::getSaveFileName(this,saveTitle,QString(),"Data (*.dat)");
	if(!fileName.isNull())
	{
		QFile file(fileName);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
		{
			// Stream writer...
			QTextStream out(&file);
			for(unsigned i=0;i<Labels.size();i++)
			{
				QString str = QString().sprintf("%lf %lf %u",Labels[i].first.first,Labels[i].first.second,Labels[i].second);
				out << str << endl;
			}
			file.close();
		}
		else
		{
			QMessageBox::critical(this,saveTitle, "Cannot write to file");
		}
	}
}

/******************************************************************************
*******************************************************************************
Data Demultiplexing   Data Demultiplexing   Data Demultiplexing   Data Demultiplexing
*******************************************************************************
******************************************************************************/


bool MainWindow::DemuxLog(QString logfile)
{
	QString logfilewoext = logfile;
	logfilewoext.chop(4);

	QString stream_names[7];
	stream_names[0]=logfilewoext+"_aud.dat";
	stream_names[1]=logfilewoext+"_acc.dat";
	stream_names[2]=logfilewoext+"_hmc.dat";
	stream_names[3]=logfilewoext+"_sys.dat";
	stream_names[4]=logfilewoext+"_lht.dat";
	stream_names[5]=logfilewoext+"_tmp.dat";
	stream_names[6]=logfilewoext+"_merged.dat";

   for(unsigned i=0;i<7;i++)
   printf("%s\n",stream_names[i].toStdString().c_str());

	// Check file already existing / overwrite
	if(ui->checkBox_LogOverwrite->isChecked()==false)
	{
		for(unsigned i=0;i<7;i++)
		{
			if(QFile(stream_names[i]).exists())
			{
				if(QMessageBox::question(this,logTitle,QString("Log demultiplexing file %1 already exist. Are you sure you want to overwrite it?").arg(QString(stream_names[i])), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
				{
					QMessageBox::information(this,logTitle,QString("Aborting the demultiplexing of %1.\n\n").arg(logfile));
					return false;
				}
			}
		}
	}


	QFile file(logfile);
	if(!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(this,logTitle,QString("Could not open log file %1 ").arg(logfile));
		return false;
	}
   QByteArray fileheader = file.read(LOG_HEADER_SIZE);
   if(fileheader.size()!=LOG_HEADER_SIZE)
	{
		QMessageBox::critical(this,logTitle,QString("This does not appear to be a log file: invalid header").arg(logfile));
		return false;
	}
	QByteArray data = file.readAll();
	file.close();
	printf("Read %d bytes\n",data.size());

	// Extract the interesting parameters
   unsigned time_offset = *(unsigned*)(fileheader.constData()+LOG_OFFSET_TIME);
	printf("Time offset of log: %u\n",time_offset);


	STREAM_GENERIC stream_aud,stream_acc,stream_hmc,stream_sys,stream_lht,stream_tmp;

   printf("Calling demux");
	demux(data,time_offset,stream_aud,stream_acc,stream_hmc,stream_sys,stream_lht,stream_tmp);
	printf("Demux done\n");


	StreamSave(stream_aud,stream_names[0]);
	StreamSave(stream_acc,stream_names[1]);
	StreamSave(stream_hmc,stream_names[2]);
	StreamSave(stream_sys,stream_names[3]);
	StreamSave(stream_lht,stream_names[4]);
	StreamSave(stream_tmp,stream_names[5]);
	printf("Save done\n");


	//

	printf("Testing merge\n");
	std::vector<STREAM_GENERIC *> streams;
	if(ui->radioButton_LogMergeAll->isChecked())
		streams.push_back(&stream_aud);
	if(ui->radioButton_LogMergeAll->isChecked() || ui->radioButton_LogMergeWOAudio->isChecked())
	{
		streams.push_back(&stream_acc);
		streams.push_back(&stream_hmc);
		streams.push_back(&stream_sys);
		streams.push_back(&stream_lht);
		streams.push_back(&stream_tmp);
	}
	if(streams.size())
	{
		STREAM_GENERIC stream_merged;
		StreamMerge(streams,stream_merged);
		printf("Merge done\n");
		StreamSave(stream_merged,stream_names[6]);
		printf("Merge saved\n");
	}
	return true;
}


void MainWindow::on_pushButton_3_clicked()
{

	 DemuxLog(ui->lineEdit_2->text());


}





void MainWindow::on_pushButton_LogSelectDirectory_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	printf("dir: %s\n",dir.toStdString().c_str());
	path.setCurrent(dir);
	ui->lineEdit_LogDirectory->setText(path.absolutePath());
}

void MainWindow::on_checkBox_LogDemux_clicked()
{
	if(ui->checkBox_LogDemux->isChecked())
		ui->groupBox_MergeOptions->setEnabled(true);
	else
		ui->groupBox_MergeOptions->setEnabled(false);
}

void MainWindow::on_pushButton_LogDemux_clicked()
{
   QString fileName = QFileDialog::getOpenFileName(this,"Load a raw log file", QString(),"Raw log (*.bin)");

   if(!fileName.isNull())
   {
      printf("Demuxing %s\n",fileName.toStdString().c_str());
      DemuxLog(fileName);

   }


}
