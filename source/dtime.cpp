#include <stdio.h>
#include <ctime>
#include "dtime.h"


DTime::DTime()
{
	//dt = QDateTime::currentDateTime();
	dt.setTime_t(0);
}
double DTime::GetTime()
{
	QDateTime t;
	t = QDateTime::currentDateTime();
	long deltams;
	double deltaf;
	deltams = dt.time().msecsTo(t.time());
	deltaf = dt.secsTo(t) + t.time().msec()/1000.0;

	/*
	printf("DTime::GetTime. secsto: %d. deltaf: %lf\n",dt.secsTo(t),deltaf);
	time_t seconds;
	seconds = time (NULL);
	printf ("%ld seconds since January 1, 1970\n", seconds);
	*/

	return deltaf;
}
