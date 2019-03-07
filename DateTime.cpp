#include "StdAfx.h"
#include <time.h>
#include <limits.h>

#define	YEAR0		1900			/* the first year */
#define	EPOCH_YR	1970			/* EPOCH = Jan 1 1970 00:00:00 */
#define	SECS_DAY	(24L * 60L * 60L)
#define	LEAPYEAR(year)	(!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define	YEARSIZE(year)	(LEAPYEAR(year) ? 366 : 365)
#define	FIRSTSUNDAY(timp)	(((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define	FIRSTDAYOF(timp)	(((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
#define	TIME_MAX	ULONG_MAX
#define	ABB_LEN		3

const unsigned int _ytab[2][12] = 
{
	{	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	},
	{	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	}
};

struct tm * gmtime(__in time_t *timer, __out struct tm *p_out_time)
{
        struct tm *timep(p_out_time);
        time_t time(*timer);
        unsigned int dayclock, dayno;
        unsigned int year(EPOCH_YR);

        dayclock = (unsigned long)time % SECS_DAY;
        dayno = (unsigned long)time / SECS_DAY;

        timep->tm_sec = dayclock % 60;
        timep->tm_min = (dayclock % 3600) / 60;
        timep->tm_hour = dayclock / 3600;
        timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */

        while (dayno >= static_cast<unsigned int>(YEARSIZE(year))) 
		{
			dayno -= YEARSIZE(year);
            ++year;
        }

        timep->tm_year = year - YEAR0;
        timep->tm_yday = dayno;
        timep->tm_mon = 0;

        while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) 
		{
			dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
            ++timep->tm_mon;
        }

        timep->tm_mday = dayno + 1;
        timep->tm_isdst = 0;

        return timep;
}

struct tm * gmtime_t(__in __time32_t timer, __out struct tm *p_out_time)
{
        struct tm *timep(p_out_time);
        __time32_t time(timer);
        unsigned int dayclock, dayno;
        unsigned int year(EPOCH_YR);

        dayclock = (unsigned long)time % SECS_DAY;
        dayno = (unsigned long)time / SECS_DAY;

        timep->tm_sec = dayclock % 60;
        timep->tm_min = (dayclock % 3600) / 60;
        timep->tm_hour = dayclock / 3600;
        timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */

        while (dayno >= static_cast<unsigned int>(YEARSIZE(year)))
		{
			dayno -= YEARSIZE(year);
            ++year;
        }

        timep->tm_year = year - YEAR0;
        timep->tm_yday = dayno;
        timep->tm_mon = 0;

        while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) 
		{
			dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
			++timep->tm_mon;
        }

        timep->tm_mday = dayno + 1;
        timep->tm_isdst = 0;

        return timep;
}



