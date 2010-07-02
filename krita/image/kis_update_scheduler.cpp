/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_update_scheduler.h"

#include "kis_merge_walker.h"
#include "kis_full_refresh_walker.h"
#include "kis_simple_update_queue.h"

KisUpdateScheduler::KisUpdateScheduler(KisImageWSP image)
    : m_image(image)
{
    /**
     * FIXME: Make queues configurable with a factory
     */
    m_workQueue = new KisSimpleUpdateQueue();

    connect(&m_updaterContext, SIGNAL(sigContinueUpdate(const QRect&)),
            m_image, SLOT(slotProjectionUpdated(const QRect&)),
            Qt::DirectConnection);

    connect(&m_updaterContext, SIGNAL(sigDoSomeUsefulWork()),
            SLOT(doSomeUsefulWork()), Qt::DirectConnection);

    connect(&m_updaterContext, SIGNAL(sigSpareThreadAppeared()),
            SLOT(spareThreadAppeared()), Qt::DirectConnection);

}

KisUpdateScheduler::~KisUpdateScheduler()
{
}

void KisUpdateScheduler::updateProjection(KisNodeSP node, const QRect& rc)
{
    if(m_workQueue->tryMergeJob(node, rc)) return;

    const QRect cropRect = m_image->bounds();
    KisBaseRectsWalkerSP walker = new KisMergeWalker(cropRect);

    walker->collectRects(node, rc);

    m_workQueue->addJob(walker);
    m_workQueue->processQueue(m_updaterContext);
}

void KisUpdateScheduler::fullRefresh(KisNodeSP root)
{
    const QRect cropRect = m_image->bounds();
    KisBaseRectsWalkerSP walker = new KisFullRefreshWalker(cropRect);

    walker->collectRects(root, cropRect);

    m_workQueue->executeJobSync(walker, m_updaterContext);
}

void KisUpdateScheduler::updateSettings()
{
}

void KisUpdateScheduler::lock()
{
    m_workQueue->blockProcessing(m_updaterContext);
}

void KisUpdateScheduler::unlock()
{
    m_workQueue->startProcessing(m_updaterContext);
}

void KisUpdateScheduler::waitForDone()
{
    m_updaterContext.waitForDone();
    Q_ASSERT(m_workQueue->isEmpty());
}

void KisUpdateScheduler::doSomeUsefulWork()
{
    m_workQueue->optimize();
}

void KisUpdateScheduler::spareThreadAppeared()
{
    m_workQueue->processQueue(m_updaterContext);
}

