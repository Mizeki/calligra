/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_shape_controller_test.h"

#include <qtest_kde.h>

#include "kis_doc2.h"
#include "kis_part2.h"
#include "kis_name_server.h"
#include "flake/kis_shape_controller.h"


KisShapeControllerTest::~KisShapeControllerTest()
{
}

KisDummiesFacadeBase* KisShapeControllerTest::dummiesFacadeFactory()
{

    KisPart2 *p = new KisPart2();
    KisDoc2 *doc = new KisDoc2(p);
    p->setDocument(doc);
    m_nameServer = new KisNameServer();
    return new KisShapeController(m_doc, m_nameServer);
}

void KisShapeControllerTest::destroyDummiesFacade(KisDummiesFacadeBase *dummiesFacade)
{
    delete dummiesFacade;
    delete m_nameServer;
    delete m_doc;
}

QTEST_KDEMAIN(KisShapeControllerTest, GUI)
#include "kis_shape_controller_test.moc"
