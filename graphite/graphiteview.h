/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <wtrobin@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef GRAPHITE_VIEW_H
#define GRAPHITE_VIEW_H

#include <koView.h>
#include <gcanvas.h>

class QResizeEvent;
class QMouseEvent;
class KoRuler;
class GraphitePart;


class GraphiteView : public KoView {

    Q_OBJECT

public:
    GraphiteView(GraphitePart *doc, QWidget *parent=0,
		 const char *name=0);
    virtual ~GraphiteView();

    virtual QWidget *canvas() { return m_canvas; }

protected slots:
    void slotViewZoom(int item);

    void recalcRulers(int x, int y);

protected:
    void resizeEvent(QResizeEvent *e);

    virtual void updateReadWrite(bool readwrite);

private:
    GCanvas *m_canvas;
    KoRuler *m_vert, *m_horiz;
    int m_oldX, m_oldY;
};
#endif
