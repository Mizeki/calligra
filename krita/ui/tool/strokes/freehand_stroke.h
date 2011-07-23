/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __FREEHAND_STROKE_H
#define __FREEHAND_STROKE_H

#include "krita_export.h"
#include "kis_types.h"
#include "kis_node.h"
#include "kis_painter_based_stroke_strategy.h"
#include "kis_distance_information.h"
#include "kis_paint_information.h"

class KisPainter;


class KRITAUI_EXPORT FreehandStrokeStrategy : public KisPainterBasedStrokeStrategy
{
public:
    class Data : public KisStrokeJobData {
    public:
        enum DabType {
            POINT,
            LINE,
            CURVE
        };

        Data(KisNodeSP _node, KisPainter *_painter,
             const KisPaintInformation &_pi,
             KisDistanceInformation &_dragDistance)
            : node(_node), painter(_painter),
              type(POINT), pi1(_pi),
              dragDistance(_dragDistance)
        {}

        Data(KisNodeSP _node, KisPainter *_painter,
             const KisPaintInformation &_pi1,
             const KisPaintInformation &_pi2,
             KisDistanceInformation &_dragDistance)
            : node(_node), painter(_painter),
              type(LINE), pi1(_pi1), pi2(_pi2),
              dragDistance(_dragDistance)
        {}

        Data(KisNodeSP _node, KisPainter *_painter,
             const KisPaintInformation &_pi1,
             const QPointF &_control1,
             const QPointF &_control2,
             const KisPaintInformation &_pi2,
             KisDistanceInformation &_dragDistance)
            : node(_node), painter(_painter),
              type(CURVE), pi1(_pi1), pi2(_pi2),
              control1(_control1), control2(_control2),
              dragDistance(_dragDistance)
        {}


        KisNodeSP node;
        KisPainter *painter;

        DabType type;
        KisPaintInformation pi1;
        KisPaintInformation pi2;
        QPointF control1;
        QPointF control2;

        KisDistanceInformation &dragDistance;
    };

public:
    FreehandStrokeStrategy(bool needsIndirectPainting,
                           KisResourcesSnapshotSP resources,
                           KisPainter *painter);

    void doStrokeCallback(KisStrokeJobData *data);
};

#endif /* __FREEHAND_STROKE_H */
