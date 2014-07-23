/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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

#ifndef SEQUENCE_GENERATOR_H
#define SEQUENCE_GENERATOR_H

#include <kis_animation_doc.h>

class SequenceGenerator
{
public:
    SequenceGenerator(KisAnimationDoc* doc, QString filename);

    bool generate(bool keyFramesOnly, int startFrame, int stopFrame);

    void cache(bool keyFramesOnly, int startFrame, int stopFrame);

private:
    KisAnimationDoc* m_doc;
    QString m_dirname;
    QList<QHash<int, KisLayerSP> > m_cache;
};

#endif // SEQUENCE_GENERATOR_H