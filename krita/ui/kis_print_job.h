/*
 *  This file is part of Krita
 *
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KISPRINTJOB_H
#define KISPRINTJOB_H

#include <KoPrintingDialog.h>

class KisView2;

class KisPrintJob : public KoPrintJob {
public:
    KisPrintJob(KisView2 *view);

protected:
    virtual QPrinter &printer() { return m_printer; }
    virtual QList<QWidget*> createOptionWidgets() const;
    virtual void startPrinting(RemovePolicy removePolicy = DoNotDelete);

private:
    KisView2 *m_view;
    QPrinter m_printer;
};

#endif
