/*
 | LuaRT - A Windows programming framework for Lua
 | Luart.org, Copyright (c) Tine Samir 2026
 | See Copyright Notice in LICENSE.TXT
 |-------------------------------------------------
 | Date.c | LuaRT Date object implementation
*/

#define LUA_LIB

#include <Date.h>
#include "lrtapi.h"
#include <luart.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <oleauto.h>
#include <math.h>


__declspec(dllexport) luart_type TDatetime;
LCID locale = LOCALE_USER_DEFAULT;

typedef union  {
		FILETIME fTime;
		ULARGE_INTEGER ul;
} Timeunion;

const char *intervals[] = {"seconds", "minutes", "hours", "days", "months", "years", NULL};
const __int64 intervals_values[] = { _SECOND, _MINUTE, _HOUR, _DAY, _MONTH, _YEAR };

//-------------------------------------[ Date Constructor ]
LUA_CONSTRUCTOR(Datetime) {
	Datetime *d = calloc(1, sizeof(Datetime));

	if (lua_gettop(L) == 4) {
		d->st = lua_touserdata(L, 4);
		d->update = lua_touserdata(L, 3);
		d->owner = lua_touserdata(L, 2);
	}
	else {
		d->st = calloc(1, sizeof(SYSTEMTIME));
		if (lua_gettop(L) == 1)
			GetLocalTime(d->st);
		else if (lua_isuserdata(L, 2))
			*d->st = *(SYSTEMTIME*)lua_touserdata(L, 2);
		else {
			wchar_t *str = lua_towstring(L, 2);
			DATE date;
			if (FAILED(VarDateFromStr(str, locale, 0, &date)))
				luaL_error(L, "Date format not recognized");
			VariantTimeToSystemTime(date, d->st);
			free(str);
		}
	}
	lua_newinstance(L, d, Datetime);
	return 1;
}

//-------------------------------------[ Date.format() ]
#ifndef _MSC_VER
typedef int (__attribute__((stdcall)) *formatfunc)(LCID,  DWORD,  const SYSTEMTIME *, const WCHAR *, WCHAR *, int);
#else
typedef int (__stdcall *formatfunc)(LCID,  DWORD,  const SYSTEMTIME *, const WCHAR *, WCHAR *, int);
#endif

static void Datetimeformat(lua_State *L, wchar_t *format, formatfunc ffunc) {
	wchar_t * buffer;
	Datetime *d = lua_self(L, 1, Datetime);
	int len = ffunc(locale, 0, d->st, format, NULL, 0);
	buffer = malloc(sizeof(wchar_t)*len);
	ffunc(locale, 0, d->st, format, buffer, len);
	lua_pushlwstring(L, buffer, len-1);
	free(buffer);
	free(format);
}

LUA_METHOD(Datetime, format) {
	int n, len = -1;
	const char *datef = luaL_optstring(L, 2, NULL);
	const char *timef = luaL_optstring(L, 3, NULL);
	
	n = lua_gettop(L);
	if (datef)
		Datetimeformat(L, utf8_towchar(datef, &len), GetDateFormatW);
	if (timef)
		Datetimeformat(L, utf8_towchar(timef, &len), GetTimeFormatW);
	len = lua_gettop(L)-n;
	if (len == 0) {
		Datetimeformat(L, NULL, GetDateFormatW);
		Datetimeformat(L, NULL, GetTimeFormatW);
		return 2;
	}
	return len;
}

//-------------------------------------[ Date.interval() ]
LUA_METHOD(Datetime, interval) {
	Timeunion t1, t2;
	int interval = luaL_checkoption(L, 3, "days", intervals);

	SystemTimeToFileTime(lua_self(L, 1, Datetime)->st, &t2.fTime);
	SystemTimeToFileTime(lua_self(L, 2, Datetime)->st, &t1.fTime);
	lua_pushinteger(L, (lua_Integer)round(((double)(t2.ul.QuadPart - t1.ul.QuadPart))/intervals_values[interval]));
	return 1;
}

//-------------------------------------[ Date Properties ]
LUA_PROPERTY_GET(Datetime, time) {
	Datetimeformat(L, NULL, GetTimeFormatW);
	return 1;
}

LUA_PROPERTY_GET(Datetime, date) {
	Datetimeformat(L, NULL, GetDateFormatW);
	return 1;
}

LUA_PROPERTY_GET(Datetime, dayname) { Datetimeformat(L, _wcsdup(L"dddd"), GetDateFormatW); return 1; }
LUA_PROPERTY_GET(Datetime, monthname) { Datetimeformat(L, _wcsdup(L"MMMM"), GetDateFormatW); return 1; }

typedef struct {
	WORD	*field;
	int		offset;
	int 	idx;
} Datefield;

static const char *props[] = { "day", "weekday", "hour", "milliseconds", "minute", "month", "second", "year", NULL };

static Datefield get_datefield(lua_State *L, Datetime *d) {
	WORD *value[] = { &d->st->wDay, &d->st->wDayOfWeek, &d->st->wHour, &d->st->wMilliseconds, &d->st->wMinute, &d->st->wMonth, &d->st->wSecond, &d->st->wYear };
	int idx = lua_optstring(L, 2, props, -1);
	Datefield df;
	
	df.field = idx == -1 ? NULL : value[idx];
	df.offset = idx == 1;
	df.idx = idx;
	return df;
}

#ifdef _WIN32

#define TICKSPERSEC        10000000
#define TICKSPERMSEC       10000
#define SECSPERDAY         86400
#define SECSPERHOUR        3600
#define SECSPERMIN         60
#define MINSPERHOUR        60
#define HOURSPERDAY        24
#define EPOCHWEEKDAY       1  /* Jan 1, 1601 was Monday */
#define DAYSPERWEEK        7
#define MONSPERYEAR        12
#define DAYSPERQUADRICENTENNIUM (365 * 400 + 97)
#define DAYSPERNORMALQUADRENNIUM (365 * 4 + 1)

/* 1601 to 1970 is 369 years plus 89 leap days */
#define SECS_1601_TO_1970  ((369 * 365 + 89) * (ULONGLONG)SECSPERDAY)
#define TICKS_1601_TO_1970 (SECS_1601_TO_1970 * TICKSPERSEC)
static double set_time[] = { SECSPERDAY, -1, SECSPERHOUR, 0.001, SECSPERMIN, 30.44 * SECSPERDAY * 10000000, 1, 0 };

static inline BOOL IsLeapYear(int Year)
{
    return Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0);
}

#endif

LUA_METHOD(Datetime, index) {	
	Datetime *d = lua_self(L, 1, Datetime);
	Datefield df = get_datefield(L, d);
	if (df.field)
		lua_pushinteger(L, (*df.field) + df.offset);
	return df.field != NULL;	
}

int getDaysInMonth(lua_State *L, int year, int month) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return IsLeapYear(year) ? 29 : 28;
        default:
            return luaL_error(L, "invalid month value");
    }
}

typedef struct {
	int year;
	int month;
	int day;
	int weekday;
	int hour;
	int minute;
	int second;
	int millisecond;
} iSYSTEMTIME;

int calculateDayOfWeek(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int k = year % 100; 
    int j = year / 100;
    int weekday = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) - (2 * j)) % 7;
    return (weekday + 7) % 7;
}

void NormalizeSystemTime(lua_State *L, SYSTEMTIME *st, iSYSTEMTIME *ist) {
    ist->second += ist->millisecond / 1000;
    ist->millisecond %= 1000;
    if (ist->millisecond < 0) {
        ist->millisecond += 1000;
        ist->second -= 1;
    }

    ist->minute += ist->second / 60;
    ist->second %= 60;
    if (ist->second < 0) {
        ist->second += 60;
		ist-> minute -= 1;
    }

    ist->hour += ist->minute / 60;
    ist->minute %= 60;
    if (ist->minute < 0) {
        ist->minute += 60;
        ist->hour -= 1;
    }

    ist->day += ist->hour / 24;
    ist->hour %= 24;
    if (ist->hour < 0) {
        ist->hour += 24;
        ist->day -= 1;
    }

    while (ist->day > getDaysInMonth(L, ist->year, ist->month)) {
        ist->day -= getDaysInMonth(L, ist->year, ist->month);
        if (++ist->month > 12) {
            ist->month = 1;
            ist->year++;
        }
    }
    while (ist->day <= 0) {
        if (--ist->month < 1) {
            ist->month = 12;
            ist->year--;
        }
        ist->day += getDaysInMonth(L, ist->year, ist->month);
    }
	ist->weekday = calculateDayOfWeek(ist->year, ist->month, ist->day);

    while (ist->month > 12) {
        ist->month -= 12;
        ist->year++;
    }
    while (ist->month < 1) {
        ist->month += 12;
        ist->year--;
    }

    st->wYear = (WORD)ist->year;
    st->wMonth = (WORD)ist->month;
    st->wDay = (WORD)ist->day;
    st->wDayOfWeek = (WORD)ist->weekday;
    st->wHour = (WORD)ist->hour;
    st->wMinute = (WORD)ist->minute;
    st->wSecond = (WORD)ist->second;
    st->wMilliseconds = (WORD)ist->millisecond;
}

LUA_METHOD(Datetime, newindex) {	
	Datetime *d = lua_self(L, 1, Datetime);
	const char *field = lua_tostring(L, 2);
	iSYSTEMTIME ist;
	
	ist.year = d->st->wYear;
    ist.month = d->st->wMonth;
    ist.day = d->st->wDay;
    ist.hour = d->st->wHour;
    ist.minute = d->st->wMinute;
    ist.second = d->st->wSecond;
    ist.millisecond = d->st->wMilliseconds;
    ist.weekday = d->st->wDayOfWeek;

	Datefield df = get_datefield(L, d);
	if (df.field) { 
		lua_Integer num = luaL_checkinteger(L, 3)-df.offset;

		switch(df.idx) {
			case 0:	ist.day = num; break;
			case 1: ist.weekday = num; break;
			case 2: ist.hour = num; break;
			case 3: ist.millisecond = num; break;
			case 4: ist.minute = num; break;
			case 5: ist.month = num; break;
			case 6: ist.second = num; break;
			case 7: ist.year = num; break;
		}
		
		NormalizeSystemTime(L, d->st, &ist);
		if (d->owner)
			d->update(L, d->owner);
	}
	return 0;
}

LUA_METHOD(Datetime, tostring) {
	Datetime_format(L);
	lua_pushstring(L, " ");
	lua_insert(L, -2);
	lua_concat(L, 3);
	return 1;
}

LUA_METHOD(Datetime, concat) {
	luaL_tolstring(L, 1, NULL);
	luaL_tolstring(L, 2, NULL);
	lua_concat(L, 2);
	return 1;
}

LUA_METHOD(Datetime, eq) {	
	lua_pushboolean(L, memcmp(lua_self(L, 1, Datetime)->st, lua_self(L, 2, Datetime)->st, sizeof(SYSTEMTIME)) == 0);
	return 1;
}

LUA_METHOD(Datetime, lt) {	
	FILETIME ft1, ft2;

	SystemTimeToFileTime(lua_self(L, 1, Datetime)->st, &ft1);
	SystemTimeToFileTime(lua_self(L, 2, Datetime)->st, &ft2);
	lua_pushboolean(L, CompareFileTime(&ft1, &ft2) == -1);
	return 1;
}

LUA_METHOD(Datetime, le) {	
	FILETIME ft1, ft2;

	SystemTimeToFileTime(lua_self(L, 1, Datetime)->st, &ft1);
	SystemTimeToFileTime(lua_self(L, 2, Datetime)->st, &ft2);
	lua_pushboolean(L, CompareFileTime(&ft1, &ft2) <= 0 );
	return 1;
}


LUA_METHOD(Datetime, gc) {
	Datetime *d = lua_self(L, 1, Datetime);
	if (!d->owner)
		free(d->st);
	free(d);
	return 0;
}

const luaL_Reg Datetime_methods[] = {
	{"format",			Datetime_format},
	{"interval",		Datetime_interval},
	{"get_time",		Datetime_gettime},
	{"get_date",		Datetime_getdate},
	{"get_dayname",		Datetime_getdayname},
	{"get_monthname",	Datetime_getmonthname},
	{NULL, NULL}
};

const luaL_Reg Datetime_metafields[] = {
	{"__metaindex", Datetime_index},
	{"__metanewindex", Datetime_newindex},
	{"__tostring",	Datetime_tostring},
	{"__concat",	Datetime_concat},
	{"__gc",		Datetime_gc},
	{"__eq",		Datetime_eq},
	{"__lt",		Datetime_lt},
	{"__le",		Datetime_le},
	{NULL, NULL}
};