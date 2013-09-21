/*
 *     kostep -- handles changetracking using operational transformation for calligra
 *     Copyright (C) 2013  Luke Wolf <Lukewolf101010@gmail.com>
 *
 *     This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU Lesser General Public
 *     License as published by the Free Software Foundation; either
 *     version 2.1 of the License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public
 *     License along with this library; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "StepDeleteTextStep.h"
#include "StepDeleteTextStep_p.h"
#include "StepStepLocation.h"
#include "QStringList"
#include "QDebug"

StepDeleteTextStepPrivate::StepDeleteTextStepPrivate (StepDeleteTextStep *q):q (q)
{
}

StepDeleteTextStepPrivate::~StepDeleteTextStepPrivate ()
{
}

StepDeleteTextStep::StepDeleteTextStep (QObject *parent):StepStepBase ("Delete Text",parent),
  d(new StepDeleteTextStepPrivate(this))
{
    d->length = 0;

}


StepDeleteTextStep::StepDeleteTextStep (int length, QObject *parent):d (new
   StepDeleteTextStepPrivate (this)), StepStepBase("Delete Text",parent)
{
    d->length = length;


}

StepDeleteTextStep::StepDeleteTextStep (const StepDeleteTextStep &other):
d (new StepDeleteTextStepPrivate (this))
{

}

StepDeleteTextStep::~StepDeleteTextStep ()
{
    delete d;

}


QString StepDeleteTextStep::toXML ()
{
    QStringList locationList = location().toString().split('/');
#ifdef DEBUG
    qDebug() << " End of Stringlist: "<< locationList.last();
#endif DEBUG
    QString temp = locationList.last().remove("\"");
    int end = temp.toInt();
#ifdef DEBUG
    qDebug() << "End: "<< end;
#endif DEBUG
    locationList.removeLast();

    locationList.append(QString::number(end + d->length));
    QString locationEnd = "";
    foreach (QString positionPart, locationList)
    {
#ifdef DEBUG
        qDebug()<< positionPart;
#endif DEBUG
        locationEnd += positionPart;
        locationEnd += "/";
    }
    locationEnd.remove(locationEnd.length()-1,1);
    locationEnd.append("\"");

    return "<del type=\"text\" "+location().toString() + " " + d->endLocation.toString() + " />" ;
}

void StepDeleteTextStep::setEndLocation(StepStepLocation endLocation)
{
    d->endLocation = endLocation;

}
