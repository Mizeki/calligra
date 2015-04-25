/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_XMLVECTOR_H
#define KO_XMLVECTOR_H

// comment it to test this class without the compression
#define KOXMLVECTOR_USE_LZF

#ifdef KOXMLVECTOR_USE_LZF
#include "KoLZF.h"
#endif

#include <QVector>
#include <QByteArray>
#include <QDataStream>
#include <QBuffer>

/**
 * KoXmlVector
 *
 * similar to QVector, but using LZF compression to save memory space
 * this class is however not reentrant
 *
 * @param uncompressedItemCount when number of buffered items reach this,
 *      compression will start small value will give better memory usage at the
 *      cost of speed bigger value will be better in term of speed, but use
 *      more memory
 */
template <typename T, int uncompressedItemCount = 256, int reservedBufferSize = 1024*1024>
class KoXmlVector
{
private:
    unsigned totalItems;
    QVector<unsigned> startIndex;
    QVector<QByteArray> blocks;

    unsigned bufferStartIndex;
    QVector<T> bufferItems;
    QByteArray bufferData;

protected:
    /**
     * fetch given item index to the buffer
     * will INVALIDATE all references to the buffer
     */
    void fetchItem(unsigned index) {
        // already in the buffer ?
        if (index >= bufferStartIndex)
            if (index - bufferStartIndex < (unsigned)bufferItems.count())
                return;

        // search in the stored blocks
        // TODO: binary search to speed up
        int loc = startIndex.count() - 1;
        for (int c = 0; c < startIndex.count() - 1; ++c)
            if (index >= startIndex[c])
                if (index < startIndex[c+1]) {
                    loc = c;
                    break;
                }

        bufferStartIndex = startIndex[loc];
#ifdef KOXMLVECTOR_USE_LZF
        KoLZF::decompress(blocks[loc], bufferData);
#else
        bufferData = blocks[loc];
#endif
        QBuffer buffer(&bufferData);
        buffer.open(QIODevice::ReadOnly);
        QDataStream in(&buffer);
        bufferItems.clear();
        in >> bufferItems;
    }

    /**
     * store data in the buffer to main blocks
     */
    void storeBuffer() {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);
        out << bufferItems;

        startIndex.append(bufferStartIndex);
#ifdef KOXMLVECTOR_USE_LZF
        blocks.append(KoLZF::compress(buffer.data()));
#else
        blocks.append(buffer.data());
#endif

        bufferStartIndex += bufferItems.count();
        bufferItems.clear();
    }

public:
    inline KoXmlVector(): totalItems(0), bufferStartIndex(0) {};

    void clear() {
        totalItems = 0;
        startIndex.clear();
        blocks.clear();

        bufferStartIndex = 0;
        bufferItems.clear();
        bufferData.reserve(reservedBufferSize);
    }

    inline int count() const {
        return (int)totalItems;
    }
    inline int size() const {
        return (int)totalItems;
    }
    inline bool isEmpty() const {
        return totalItems == 0;
    }

    /**
     * append a new item
     * WARNING: use the return value as soon as possible
     * it may be invalid if another function is invoked
     */
    T& newItem() {
        // buffer full?
        if (bufferItems.count() >= uncompressedItemCount - 1)
            storeBuffer();

        ++totalItems;
        bufferItems.resize(bufferItems.count() + 1);
        return bufferItems[bufferItems.count()-1];
    }

    /**
     * WARNING: use the return value as soon as possible
     * it may be invalid if another function is invoked
     */
    const T &operator[](int i) const {
        ((KoXmlVector*)this)->fetchItem((unsigned)i);
        return bufferItems[i - bufferStartIndex];
    }

    /**
     * optimize memory usage
     * will INVALIDATE all references to the buffer
     */
    void squeeze() {
        storeBuffer();
    }

};

#endif