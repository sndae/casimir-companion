#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QDir>
#include "sd.h"
#include "lowlevel.h"
#include <qextserialport/qextserialenumerator.h>
#include <qextserialport/qextserialport.h>
#include "xcommand.h"
#include "dtime.h"
#include "keypresseater.h"
#include "pkt.h"

namespace Ui {
    class MainWindow;
}



class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
		Ui::MainWindow *ui;

		QDir path;
		QString logTitle;

		std::vector<VOLUMEDEF> AllVolumeDefinitions;
		std::vector<VOLUMEDEF> SuitableVolumeDefinitions;
		sd_filesystem filesystem;
		int currentvolume,currentlog;

		QString comTitle;
		QextSerialPort *port;
		char xcommand_buffer[XCOMMAND_SIZE];
		QByteArray rcvbuffer;
		QTimer *timer;
		QPixmap PixmapLogging,PixmapStopped,PixmapNC;
		unsigned LoggerTimeLastSync,LoggerTimeFirstSync,LoggerTimeOffset;
		double PCTimeLastSync,PCTimeFirstSync;
		bool TimeSyncd;
		DTime CurrentTime;
		int timer_1,timer_5;



		double GetCurrentTime();

		//bool CopyRaw(QByteArray src,QByteArray dst,int blockstart,int blockend);
		bool CopyRaw(QByteArray src,QByteArray dst,int entry);
		bool CopyLog(int i,QString dst);
		void ProcessLog(int i);
		void ReadFileSystem(int volume);
		void DisplayFileSystem();
		void DisplaySuitableVolumeDefinitions();
		void GetAllVolumeDefinitions();

		bool LoggerCheckConnected(bool displayerror=true);
		bool LoggerSendXCommand(XCOMMAND cmd);
		bool LoggerSendXCommandWait(XCOMMAND cmd,bool displayerror=true, double timeout=1.0);
		void LoggerReceivedXCommand(XCOMMAND cmd);
		void LoggerReceivedTime(unsigned to,unsigned tt);
		void LoggerReceivedLoggingStatus(bool logging);
		bool LoggerReadAll(bool displayerror=true);
		bool ReadXCommand(std::vector<XCOMMAND> &cmdv);
		void LoggerResetTimeStats();

		bool LoggerCmdGetID(bool displayerror=true,double timeout=1.0);
		bool LoggerCmdGetFlash(bool displayerror=true,double timeout=1.0);
		bool LoggerCmdGetTime(bool displayerror=true,double timeout=1.0);
		bool LoggerCmdSetTime(unsigned time);


		void timerEvent(QTimerEvent *event);

		void SetDefaultLoggerStatus();
		unsigned LoggerEstimateCurrentTick(bool last=true);
		double LoggerEstimateCurrentTime(bool last=true);

		void LoggerDisconnect();
		bool LoggerConnect(QString port);

		// Label
		KeyPressEater ke;
		bool Labeling;
		//std::vector<std::pair<double,int> > Labels;
		std::vector<std::pair<std::pair<double,double>,int> > Labels;		// Pair of times (pc time and device time) paired with label.

		void StartStopLabeling();
		void DisplayLabels();

		// Data demultiplexing
		bool DemuxLog(QString logfile);


private slots:
		void on_checkBox_LogDemux_clicked();
  void on_pushButton_LogSelectDirectory_clicked();
  void on_pushButton_3_clicked();
  void on_pushButton_LabelSave_clicked();
  void on_pushButton_LabelClear_clicked();
  void on_pushButton_LabelStartStop_clicked();
	  void on_pushButton_LoggerSetTime_clicked();
	  void on_pushButton_LoggerResetTime_clicked();
	  void on_pushButton_LoggerResetTimeEstimate_clicked();
	  void on_pushButton_LoggerAutodetect_clicked();
	  void on_pushButton_LoggerReadAllStatus_clicked();
		void on_pushButton_LoggerSetID_clicked();
		void on_pushButton_LoggerFormat_clicked();
		void on_pushButton_LoggerStop_clicked();
		void on_pushButton_LoggerStart_clicked();
		void on_pushButton_LoggerGetTime_clicked();
		void on_pushButton_LoggerGetFlash_clicked();
		void on_pushButton_LoggerGetID_clicked();
		void serialRead();
		void serialDSRChanged(bool);
		void serialAboutToClose();
		void serialBytesWritten(qint64);


	void on_pushButton_LoggerConnect_clicked();
	void on_pushButton_ReadSelectedLog_clicked();
	void on_tableWidget_FileSystem_itemSelectionChanged();
	void on_pushButton_ReadAllLogs_clicked();
	void on_pushButton_ReadFileSystem_clicked();
	void on_tableWidget_itemSelectionChanged();
	void on_pushButton_RefreshVolumes_clicked();
	void on_pushButton_6_clicked();
	void on_pushButton_5_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_clicked();
	void on_actionList_ports_triggered();

		// Label
		void LabelKey(int key);
};

#endif // MAINWINDOW_H
