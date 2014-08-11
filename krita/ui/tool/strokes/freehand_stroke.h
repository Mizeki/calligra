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
#include "kis_lod_transform.h"

class KisPainter;


class KRITAUI_EXPORT FreehandStrokeStrategy : public KisPainterBasedStrokeStrategy
{
public:
    class Data : public KisStrokeJobData {
    public:
        enum DabType {
            POINT,
            LINE,
            CURVE,
            POLYLINE,
            POLYGON,
            RECT,
            ELLIPSE,
            PAINTER_PATH
        };

        Data(KisNodeSP _node, PainterInfo *_painterInfo,
             const KisPaintInformation &_pi)
            : node(_node), painterInfo(_painterInfo),
              type(POINT), pi1(_pi)
        {}

        Data(KisNodeSP _node, PainterInfo *_painterInfo,
             const KisPaintInformation &_pi1,
             const KisPaintInformation &_pi2)
            : node(_node), painterInfo(_painterInfo),
              type(LINE), pi1(_pi1), pi2(_pi2)
        {}

        Data(KisNodeSP _node, PainterInfo *_painterInfo,
             const KisPaintInformation &_pi1,
             const QPointF &_control1,
             const QPointF &_control2,
             const KisPaintInformation &_pi2)
            : node(_node), painterInfo(_painterInfo),
              type(CURVE), pi1(_pi1), pi2(_pi2),
              control1(_control1), control2(_control2)
        {}

        Data(KisNodeSP _node, PainterInfo *_painterInfo,
             DabType _type,
             const vQPointF &_points)
            : node(_node), painterInfo(_painterInfo),
            type(_type), points(_points)
        {}

        Data(KisNodeSP _node, PainterInfo *_painterInfo,
             DabType _type,
             const QRectF &_rect)
            : node(_node), painterInfo(_painterInfo),
            type(_type), rect(_rect)
        {}

        Data(KisNodeSP _node, PainterInfo *_painterInfo,
             DabType _type,
             const QPainterPath &_path)
            : node(_node), painterInfo(_painterInfo),
            type(_type), path(_path)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) {
            return new Data(*this, levelOfDetail);
        }

    private:
        Data(const Data &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs),
              node(rhs.node),
              painterInfo(rhs.painterInfo),
              type(rhs.type)
        {
            KisLodTransform t(levelOfDetail);

            switch(type) {
            case Data::POINT:
                pi1 = t.map(rhs.pi1);
                break;
            case Data::LINE:
                pi1 = t.map(rhs.pi1);
                pi2 = t.map(rhs.pi2);
                break;
            case Data::CURVE:
                pi1 = t.map(rhs.pi1);
                pi2 = t.map(rhs.pi2);
                control1 = t.map(rhs.control1);
                control2 = t.map(rhs.control2);
                break;
            case Data::POLYLINE:
                points = t.map(rhs.points);
                break;
            case Data::POLYGON:
                points = t.map(rhs.points);
                break;
            case Data::RECT:
                rect = t.map(rhs.rect);
                break;
            case Data::ELLIPSE:
                rect = t.map(rhs.rect);
                break;
            case Data::PAINTER_PATH:
                path = t.map(rhs.path);
            };
        }
    public:
        KisNodeSP node;
        PainterInfo *painterInfo;

        DabType type;
        KisPaintInformation pi1;
        KisPaintInformation pi2;
        QPointF control1;
        QPointF control2;

        vQPointF points;
        QRectF rect;
        QPainterPath path;
    };

public:
    FreehandStrokeStrategy(bool needsIndirectPainting,
                           const QString &indirectPaintingCompositeOp,
                           KisResourcesSnapshotSP resources,
                           PainterInfo *painterInfo,
                           const KUndo2MagicString &name);

    FreehandStrokeStrategy(bool needsIndirectPainting,
                           const QString &indirectPaintingCompositeOp,
                           KisResourcesSnapshotSP resources,
                           QVector<PainterInfo*> painterInfos,
                           const KUndo2MagicString &name);

    void doStrokeCallback(KisStrokeJobData *data);

    KisStrokeStrategy* createLodClone(int levelOfDetail);

protected:
    FreehandStrokeStrategy(const FreehandStrokeStrategy &rhs, int levelOfDetail);

private:
    void init(bool needsIndirectPainting, const QString &indirectPaintingCompositeOp);
};

#endif /* __FREEHAND_STROKE_H */
