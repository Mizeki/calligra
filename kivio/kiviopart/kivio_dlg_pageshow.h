/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef __kivio_dlg_pageshow__
#define __kivio_dlg_pageshow__

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>

class KivioView;

class KivioPageShow: public QDialog
{ Q_OBJECT
public:
  KivioPageShow( KivioView* parent, const char* name );

public slots:
  void slotOk();
  void slotClose();
  void slotDoubleClicked(QListBoxItem *);

protected:
  KivioView* m_pView;
  QListBox * list;
  QPushButton* m_pOk;
  QPushButton* m_pClose;
};

#endif
