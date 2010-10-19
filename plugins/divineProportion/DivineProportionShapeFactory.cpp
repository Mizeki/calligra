/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "DivineProportionShapeFactory.h"
#include "DivineProportionShape.h"
#include "Intro.h"

#include <klocale.h>
#include <KoShapeLoadingContext.h>

DivineProportionShapeFactory::DivineProportionShapeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, DivineProportionShape_SHAPEID, i18n("DivineProportion"))
{
    setToolTip(i18n("A Shape That Shows DivineProportion"));
    setIcon( "divine-shape" );
}

KoShape *DivineProportionShapeFactory::createDefaultShape(KoResourceManager *) const
{
    DivineProportionShape *shape = new DivineProportionShape();
    shape->setShapeId(DivineProportionShape_SHAPEID);
    return shape;
}

QList<KoShapeConfigWidgetBase*> DivineProportionShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> answer;
    answer.append(new Intro());
    return answer;
}

bool DivineProportionShapeFactory::supports(const KoXmlElement & e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return false;
}

#include <DivineProportionShapeFactory.moc>
