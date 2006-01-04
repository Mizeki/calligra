/* This file is part of the KDE project
   Copyright (C) 2003 - 2005 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; 
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPTCALENDAR_H
#define KPTCALENDAR_H

#include "kptmap.h"
#include "kptduration.h"

#include <qdatetime.h>
#include <qpair.h>
#include <qptrlist.h>

class QDomElement;
class QDateTime;
class QTime;
class QDate;
class QString;

namespace KPlato
{

class DateTime;
class Project;

class CalendarDay {

public:
    CalendarDay();
    CalendarDay(int state);
    CalendarDay(QDate date, int state=0);
    CalendarDay(CalendarDay *day);
    ~CalendarDay();

    bool load(QDomElement &element);
    void save(QDomElement &element);

    const QPtrList<QPair<QTime, QTime> > &workingIntervals() const { return m_workingIntervals; }
    void addInterval(QPair<QTime, QTime> *interval);
    void addInterval(QPair<QTime, QTime> interval) { addInterval(new QPair<QTime, QTime>(interval)); }
    void clearIntervals() { m_workingIntervals.clear(); }
    void setIntervals(QPtrList<QPair<QTime, QTime> > intervals) { 
        m_workingIntervals.clear();
        m_workingIntervals = intervals;
    }
    
    QTime startOfDay() const;
    QTime endOfDay() const;
    
    QDate date() const { return m_date; }
    void setDate(QDate date) { m_date = date; }
    int state() const { return m_state; }
    void setState(int state) { m_state = state; }

    bool operator==(const CalendarDay *day) const;
    bool operator==(const CalendarDay &day) const;
    bool operator!=(const CalendarDay *day) const;
    bool operator!=(const CalendarDay &day) const;

    /**
     * Returns the amount of 'worktime' that can be done on
     * this day between the times start and end.
     */
    Duration effort(const QTime &start, const QTime &end);

    /**
     * Returns the actual 'work interval' for the interval start to end.
     * If no 'work interval' exists, returns the interval start, end.
     * Use @ref hasInterval() to check if a 'work interval' exists.
     */
    QPair<QTime, QTime> interval(const QTime &start, const QTime &end) const;
    
    bool hasInterval() const;

    /**
     * Returns true if at least a part of a 'work interval' exists 
     * for the interval start to end.
     */
    bool hasInterval(const QTime &start, const QTime &end) const;
    
    bool hasIntervalBefore(const QTime &time) const;
    bool hasIntervalAfter(const QTime &time) const;

    Duration duration() const;
    
    const CalendarDay &copy(const CalendarDay &day);

private:
    QDate m_date; //NOTE: inValid if used for weekdays
    int m_state;
    QPtrList<QPair<QTime, QTime> > m_workingIntervals;

#ifndef NDEBUG
public:
    void printDebug(QCString indent="");
#endif
};

class CalendarWeekdays {

public:
    CalendarWeekdays();
    CalendarWeekdays(CalendarWeekdays *weekdays);
    ~CalendarWeekdays();

    bool load(QDomElement &element);
    void save(QDomElement &element);

    void addWeekday(CalendarDay *day) { m_weekdays.append(day); }
    const QPtrList<CalendarDay> &weekdays() const { return m_weekdays; }
    /**
     * Returns the pointer to CalendarDay for @day or 0 if not defined. 
     * day is 0..6.
     */
    CalendarDay *weekday(int day) const;
    CalendarDay *weekday(const QDate &date) const { return weekday(date.dayOfWeek()-1); }
    CalendarDay *replace(int weekday, CalendarDay *day) {
        CalendarDay *d = m_weekdays.at(weekday);
        m_weekdays.replace(weekday, day);
        return d;
    }
    IntMap map();
    
    void setWeekday(IntMap::iterator it, int state) { m_weekdays.at(it.key())->setState(state); }

    int state(const QDate &date) const;
    int state(int weekday) const;
    void setState(int weekday, int state);
    
    const QPtrList<QPair<QTime, QTime> > &intervals(int weekday) const;
    void setIntervals(int weekday, QPtrList<QPair<QTime, QTime> >intervals);
    void clearIntervals(int weekday);
    
    bool operator==(const CalendarWeekdays *weekdays) const;
    bool operator!=(const CalendarWeekdays *weekdays) const;

    Duration effort(const QDate &date, const QTime &start, const QTime &end);
    
    /**
     * Returns the actual 'work interval' on the weekday defined by date
     * for the interval start to end.
     * If no 'work interval' exists, returns the interval start, end.
     * Use @ref hasInterval() to check if a 'work interval' exists.
     */
    QPair<QTime, QTime> interval(const QDate date, const QTime &start, const QTime &end) const;
    /**
     * Returns true if at least a part of a 'work interval' exists 
     * on the weekday defined by date for the interval start to end.
     */
    bool hasInterval(const QDate date, const QTime &start, const QTime &end) const;
    bool hasIntervalAfter(const QDate date, const QTime &time) const;
    bool hasIntervalBefore(const QDate date, const QTime &time) const;

    bool hasInterval() const;

    Duration duration() const;
    Duration duration(int weekday) const;

    /// Returns the time when the  weekday starts
    QTime startOfDay(int weekday) const;
    /// Returns the time when the  weekday ends
    QTime endOfDay(int weekday) const;

    const CalendarWeekdays &copy(const CalendarWeekdays &weekdays);

private:
    QPtrList<CalendarDay> m_weekdays;
    double m_workHours;

#ifndef NDEBUG
public:
    void printDebug(QCString indent="");
#endif
};

class CalendarWeeks {

public:
    CalendarWeeks();
    CalendarWeeks(CalendarWeeks *weeks);
    ~CalendarWeeks();

    bool load(QDomElement &element);
    void save(QDomElement &element);

    void setWeek(int week, int year, int type);
    void setWeek(WeekMap::iterator it, int state){ m_weeks.insert(it, state); }
    void setWeeks(WeekMap m){ m_weeks = m; }
    WeekMap weeks() const { return m_weeks; }
    
    bool operator==(const CalendarWeeks *w) const { 
        return weeks() == w->weeks(); 
    }
    bool operator!=(const CalendarWeeks *w) const { 
        return weeks() != w->weeks(); 
    }
    
    int state(const QDate &date);

    CalendarWeeks &copy(CalendarWeeks &weeks);

private:
    WeekMap m_weeks;

#ifndef NDEBUG
public:
    void printDebug(QCString indent="");
#endif
};

class StandardWorktime;
/**
 * Calendar defines the working and nonworking days and hours.
 * A day can have the three states None (Undefined), NonWorking, or Working.
 * A calendar can have a parent calendar that defines the days that are 
 * undefined in this calendar. If a day is still undefined, it defaults
 * to Nonworking.
 * A Working day has one or more work intervals to define the work hours.
 *
 * The definition can consist of three parts: Weekdays, Week, and Day.
 * Day has highest priority, Weekdays lowest.
 * Week can only have states None and Nonworking. (Should maybe change this)
 * The total priorities including parent(s), are:
 *  1. Own Day
 *  2. Own Weeks
 *  3. Parent Day
 *  4. Parent Weeks
 *  5. Own Weekdays
 *  6. Parent weekdays
 * 
 * A typical calendar hierarchy could include calendars on three levels:
 *  1. Definition of normal weekdays and national holidays/vacation days.
 *  2. Definition of the company's special workdays/-time and vacation days.
 *  3. Definitions for groups of resources/individual resources.
 *
 */
class Calendar {

public:
    Calendar();
    Calendar(QString name, Calendar *parent=0);
    Calendar(Calendar *calendar);
    ~Calendar();

    QString name() const { return m_name; }
    void setName(QString name) { m_name = name; }

    Calendar *parent() const { return m_parent; }
    void setParent(Calendar *parent) { m_parent = parent; }
    
    Project *project() const { return m_project; }
    void setProject(Project *project);

    bool isDeleted() const { return m_deleted; }
    void setDeleted(bool yes);

    QString id() const { return m_id; }
    bool setId(QString id);
    void generateId();
    
    bool load(QDomElement &element);
    void save(QDomElement &element);

    /**
     * Find the definition for the day date.
     * If skipNone=true the day is NOT returned if it has state None.
     */
    CalendarDay *findDay(const QDate &date, bool skipNone=false) const;
    bool addDay(CalendarDay *day) { return m_days.insert(0, day); }
    bool removeDay(CalendarDay *day) { return m_days.removeRef(day); }
    CalendarDay *takeDay(CalendarDay *day) { return m_days.take(m_days.find(day)); }
    const QPtrList<CalendarDay> &days() const { return m_days; }
    
    /**
     * Returns the state of definition for the day and week with date in it.
     * Also checks parent.
     */
    int parentDayWeekState(const QDate &date) const;
    /**
     * Returns the state of definition for the date's weekday.
     * Also checks parent.
     */
    int parentWeekdayState(const QDate &date) const;
    /**
     * Returns the state of definition for the week with date in it.
     * State can be None or Nonworking.
     */
    int weekState(const QDate &date) const;
    
    void setWeek(int week, int year, int type) { m_weeks->setWeek(week, year, type); }
    void setWeek(WeekMap::iterator it, int state){ m_weeks->setWeek(it, state); }
    WeekMap weekMap() { return m_weeks->weeks(); }
    CalendarWeeks *weeks() { return m_weeks; }
    
    IntMap weekdaysMap() { return m_weekdays->map(); }
    void setWeekday(IntMap::iterator it, int state) { m_weekdays->setWeekday(it, state); }
    CalendarWeekdays *weekdays() { return m_weekdays; }
    CalendarDay *weekday(int day) const { return m_weekdays->weekday(day); }

    QString parentId() const { return m_parentId; }
    void setParentId(QString id) { m_parentId = id; }

    bool hasParent(Calendar *cal);

    /**
     * Returns the amount of 'worktime' that can be done on
     * the date  date between the times  start and  end.
     */
    Duration effort(const QDate &date, const QTime &start, const QTime &end) const;
    /**
     * Returns the amount of 'worktime' that can be done in the
     * interval from start with the duration  duration
     */
    Duration effort(const DateTime &start, const Duration &duration) const;

    /**
     * Returns the first 'work interval' for the interval 
     * starting at start with duration duration.
     * If no 'work interval' exists, returns the interval (start, end).
     * Use @ref hasInterval() to check if a 'work interval' exists.
     */
    QPair<DateTime, DateTime> interval(const DateTime &start, const DateTime &end) const;
    
    /// Checks if a work interval exists before time
    bool hasIntervalBefore(const DateTime &time) const;
    /// Checks if a work interval exists after time
    bool hasIntervalAfter(const DateTime &time) const;

    /**
     * Returns true if at least a part of a 'work interval' exists 
     * for the interval starting at start and ending at end.
     */
    bool hasInterval(const DateTime &start, const DateTime &end) const;
        
    /**
     * Returns true if at least a part of a 'work interval' exists 
     * for the interval on date, starting at start and ending at end.
     */
    bool hasInterval(const QDate &date, const QTime &start, const QTime &end) const;
        
    DateTime availableAfter(const DateTime &time);
    DateTime availableBefore(const DateTime &time);

    Calendar *findCalendar() const { return findCalendar(m_id); }
    Calendar *findCalendar(const QString &id) const;
    bool removeId() { return removeId(m_id); }
    bool removeId(const QString &id);
    void insertId(const QString &id);
    
    Duration parentDayWeekEffort(const QDate &date, const QTime &start, const QTime &end) const;
    Duration parentWeekdayEffort(const QDate &date, const QTime &start, const QTime &end) const;
    int parentDayWeekHasInterval(const QDate &date, const QTime &start, const QTime &end) const;
    bool parentWeekdayHasInterval(const QDate &date, const QTime &start, const QTime &end) const;
    QPair<QTime, QTime> parentDayWeekInterval(const QDate &date, const QTime &start, const QTime &end) const;
    QPair<QTime, QTime> parentWeekdayInterval(const QDate &date, const QTime &start, const QTime &end) const;

protected:
    const Calendar &copy(Calendar &calendar);
    void init();
    
private:
    QString m_name;
    Calendar *m_parent;
    Project *m_project;
    bool m_deleted;
    QString m_id;
    QString m_parentId;

    QPtrList<CalendarDay> m_days;
    CalendarWeeks *m_weeks;
    CalendarWeekdays *m_weekdays;

#ifndef NDEBUG
public:
    void printDebug(QCString indent="");
#endif
};

class StandardWorktime
{
public:
    StandardWorktime();
    StandardWorktime(StandardWorktime* worktime);
    ~StandardWorktime();
    
    /// The work time of a normal year.
    Duration durationYear() const { return m_year; }
    /// The work time of a normal year.
    double year() const { return m_year.toDouble(Duration::Unit_h); }
    /// Set the work time of a normal year.
    void setYear(const Duration year) { m_year = year; }
    /// Set the work time of a normal year.
    void setYear(double hours) { m_year = Duration((Q_INT64)(hours*60.0*60.0)); }
    
    /// The work time of a normal month
    Duration durationMonth() const { return m_month; }
    /// The work time of a normal month
    double month() const  { return m_month.toDouble(Duration::Unit_h); }
    /// Set the work time of a normal month
    void setMonth(const Duration month) { m_month = month; }
    /// Set the work time of a normal month
    void setMonth(double hours) { m_month = Duration((Q_INT64)(hours*60.0*60.0)); }
    
    /// The work time of a normal week
    Duration durationWeek() const { return m_week; }
    /// The work time of a normal week
    double week() const { return m_week.toDouble(Duration::Unit_h); }
    /// Set the work time of a normal week
    void setWeek(const Duration week) { m_week = week; }
    /// Set the work time of a normal week
    void setWeek(double hours) { m_week = Duration((Q_INT64)(hours*60.0*60.0)); }
    
    /// The work time of a normal day
    Duration durationDay() const { return m_day; }
    /// The work time of a normal day
    double day() const { return m_day.toDouble(Duration::Unit_h); }
    /// Set the work time of a normal day
    void setDay(const Duration day) { m_day = day; }
    /// Set the work time of a normal day
    void setDay(double hours) { m_day = Duration((Q_INT64)(hours*60.0*60.0)); }
    
    bool load(QDomElement &element);
    void save(QDomElement &element);

protected:
    void init();
    
private:
    Duration m_year;
    Duration m_month;
    Duration m_week;
    Duration m_day;
    
#ifndef NDEBUG
public:
    void printDebug(QCString indent="");
#endif
};

}  //KPlato namespace

#endif
