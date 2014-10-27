/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <QTableView>

/**
 * @brief The TimelineView class shows an animation timeline in tabular form.
 */
class TimelineView : public QTableView
{
    Q_OBJECT
public:
    explicit TimelineView(QWidget *parent = 0);

signals:

public slots:

};

#endif // TIMELINEVIEW_H
