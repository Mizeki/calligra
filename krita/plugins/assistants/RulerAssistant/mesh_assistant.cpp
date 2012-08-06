/*
 * Copyright (c) 2012 Shivaraman Aiyer <sra392@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#include "mesh_assistant.h"
#include <klocale.h>

#include <assimp.hpp>
#include <aiScene.h>
#include <aiMesh.h>
#include <aiPostProcess.h>

MeshAssistant::MeshAssistant()
    : KisPaintingAssistant("mesh",i18n("Mesh assistant"))
{
}

void MeshAssistant::initialize(char* file){
    Assimp::Importer imp;
        const aiScene* scene = imp.ReadFile( file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs );
        if(!scene)
        {
            return;
        }
        else
            qDebug() << "import complete";
}

QPointF MeshAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/)
{
    return QPointF();
}

void MeshAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter)
{

}

QRect MeshAssistant::boundingRect() const
{
    return QRect();
}

QPointF MeshAssistant::buttonPosition() const
{
    return QPointF();
}

MeshAssistantFactory::MeshAssistantFactory()
{
}

MeshAssistantFactory::~MeshAssistantFactory()
{
}

QString MeshAssistantFactory::id() const
{
    return "mesh";
}

QString MeshAssistantFactory::name() const
{
    return i18n("Mesh assistant");
}

KisPaintingAssistant* MeshAssistantFactory::createPaintingAssistant() const
{
    KUrl file = KFileDialog::getOpenUrl(KUrl(), QString("*.blend"));
    MeshAssistant* assistant = new MeshAssistant;
    assistant->initialize(file.toLocalFile().toUtf8().data());
    return assistant;
}
