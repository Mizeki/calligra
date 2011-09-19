/*
  Copyright 2011 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

// Own
#include "EmfplusParser.h"

// Qt
#include <QColor>
#include <QFile>
#include <QBuffer>

// KDE
#include <KDebug>

// LibEmf
#include "EmfplusEnums.h"
#include "EmfplusDeviceContext.h"
//#include "EmfRecords.h"
//#include "EmfObjects.h"


// 0 - No debug
// 1 - Print a lot of debug info
// 2 - Just print all the records instead of parsing them
#define DEBUG_EMFPARSER 1


namespace Libemf
{


static void soakBytes(QDataStream &stream, int numBytes)
{
    quint8 scratch;
    for (int i = 0; i < numBytes; ++i) {
        stream >> scratch;
    }
}

static void outputBytes(QDataStream &stream, int numBytes)
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
	//qDebug("byte(%i):%c", i, scratch);
    }
}


// ================================================================


EmfplusParser::EmfplusParser()
    : m_backend( 0 )
{
}

EmfplusParser::~EmfplusParser()
{
}


bool EmfplusParser::parse( QDataStream &stream ) 
{
    stream.setByteOrder( QDataStream::LittleEndian );

    Header *header = new Header( stream );
    if ( ! header->isValid() ) {
        kWarning() << "Failed to parse header, perhaps not an EMF file";
        delete header;
        return false;
    }

    m_backend->init( header );

    EmfplusDeviceContext  deviceContext;

#if DEBUG_EMFPARSER
    kDebug(31000) << "========================================================== Starting EMF";
#endif

    int numRecords = header->recordCount();
    for ( int i = 1; i < numRecords; ++i ) {
        // kDebug(33100) << "Record" << i << "of" << numRecords;
        if ( !parseRecord(stream, deviceContext) ) {
            break;
        }
    }

    m_backend->cleanup( header );

    delete header;

    return true;
}

void EmfplusParser::setBackend(EmfAbstractBackend *backend)
{
    m_backend = backend;
}

/**
   The RecordType enumeration defines record types used in EMF+ metafiles.

   See [MS-EMFPLUS] Section 2.1.1.1 for more information.
*/
enum RecordType {
    EmfPlusHeader                  = 0x4001,
    EmfPlusEndOfFile               = 0x4002,
    EmfPlusComment                 = 0x4003,
    EmfPlusGetDC                   = 0x4004,
    EmfPlusMultiFormatStart        = 0x4005,
    EmfPlusMultiFormatSection      = 0x4006,
    EmfPlusMultiFormatEnd          = 0x4007,
    EmfPlusObject                  = 0x4008,
    EmfPlusClear                   = 0x4009,
    EmfPlusFillRects               = 0x400A,
    EmfPlusDrawRects               = 0x400B,
    EmfPlusFillPolygon             = 0x400C,
    EmfPlusDrawLines               = 0x400D,
    EmfPlusFillEllipse             = 0x400E,
    EmfPlusDrawEllipse             = 0x400F,
    EmfPlusFillPie                 = 0x4010,
    EmfPlusDrawPie                 = 0x4011,
    EmfPlusDrawArc                 = 0x4012,
    EmfPlusFillRegion              = 0x4013,
    EmfPlusFillPath                = 0x4014,
    EmfPlusDrawPath                = 0x4015,
    EmfPlusFillClosedCurve         = 0x4016,
    EmfPlusDrawClosedCurve         = 0x4017,
    EmfPlusDrawCurve               = 0x4018,
    EmfPlusDrawBeziers             = 0x4019,
    EmfPlusDrawImage               = 0x401A,
    EmfPlusDrawImagePoints         = 0x401B,
    EmfPlusDrawString              = 0x401C,
    EmfPlusSetRenderingOrigin      = 0x401D,
    EmfPlusSetAntiAliasMode        = 0x401E,
    EmfPlusSetTextRenderingHint    = 0x401F,
    EmfPlusSetTextContrast         = 0x4020,
    EmfPlusSetInterpolationMode    = 0x4021,
    EmfPlusSetPixelOffsetMode      = 0x4022,
    EmfPlusSetCompositingMode      = 0x4023,
    EmfPlusSetCompositingQuality   = 0x4024,
    EmfPlusSave                    = 0x4025,
    EmfPlusRestore                 = 0x4026,
    EmfPlusBeginContainer          = 0x4027,
    EmfPlusBeginContainerNoParams  = 0x4028,
    EmfPlusEndContainer            = 0x4029,
    EmfPlusSetWorldTransform       = 0x402A,
    EmfPlusResetWorldTransform     = 0x402B,
    EmfPlusMultiplyWorldTransform  = 0x402C,
    EmfPlusTranslateWorldTransform = 0x402D,
    EmfPlusScaleWorldTransform     = 0x402E,
    EmfPlusRotateWorldTransform    = 0x402F,
    EmfPlusSetPageTransform        = 0x4030,
    EmfPlusResetClip               = 0x4031,
    EmfPlusSetClipRect             = 0x4032,
    EmfPlusSetClipPath             = 0x4033,
    EmfPlusSetClipRegion           = 0x4034,
    EmfPlusOffsetClip              = 0x4035,
    EmfPlusDrawDriverstring        = 0x4036,
    EmfPlusStrokeFillPath          = 0x4037,
    EmfPlusSerializableObject      = 0x4038,
    EmfPlusSetTSGraphics           = 0x4039,
    EmfPlusSetTSClip               = 0x403A,

    EMFPLUS_LASTRECORD
};



static const struct {
    int  recordType;
    QString name;
} EmfplusRecords[] = {
    { 0x0000, "NULL" },

    { 0x4001, "EmfPlusHeader" }, 
    { 0x4002, "EmfPlusEndOfFile" }, 
    { 0x4003, "EmfPlusComment" }, 
    { 0x4004, "EmfPlusGetDC" }, 
    { 0x4005, "EmfPlusMultiFormatStart" }, 
    { 0x4006, "EmfPlusMultiFormatSection" }, 
    { 0x4007, "EmfPlusMultiFormatEnd" }, 
    { 0x4008, "EmfPlusObject" }, 
    { 0x4009, "EmfPlusClear" }, 
    { 0x400A, "EmfPlusFillRects" }, 
    { 0x400B, "EmfPlusDrawRects" }, 
    { 0x400C, "EmfPlusFillPolygon" }, 
    { 0x400D, "EmfPlusDrawLines" }, 
    { 0x400E, "EmfPlusFillEllipse" }, 
    { 0x400F, "EmfPlusDrawEllipse" }, 
    { 0x4010, "EmfPlusFillPie" }, 
    { 0x4011, "EmfPlusDrawPie" }, 
    { 0x4012, "EmfPlusDrawArc" }, 
    { 0x4013, "EmfPlusFillRegion" }, 
    { 0x4014, "EmfPlusFillPath" }, 
    { 0x4015, "EmfPlusDrawPath" }, 
    { 0x4016, "EmfPlusFillClosedCurve" }, 
    { 0x4017, "EmfPlusDrawClosedCurve" }, 
    { 0x4018, "EmfPlusDrawCurve" }, 
    { 0x4019, "EmfPlusDrawBeziers" }, 
    { 0x401A, "EmfPlusDrawImage" }, 
    { 0x401B, "EmfPlusDrawImagePoints" }, 
    { 0x401C, "EmfPlusDrawString" }, 
    { 0x401D, "EmfPlusSetRenderingOrigin" }, 
    { 0x401E, "EmfPlusSetAntiAliasMode" }, 
    { 0x401F, "EmfPlusSetTextRenderingHint" }, 
    { 0x4020, "EmfPlusSetTextContrast" }, 
    { 0x4021, "EmfPlusSetInterpolationMode" }, 
    { 0x4022, "EmfPlusSetPixelOffsetMode" }, 
    { 0x4023, "EmfPlusSetCompositingMode" }, 
    { 0x4024, "EmfPlusSetCompositingQuality" }, 
    { 0x4025, "EmfPlusSave" }, 
    { 0x4026, "EmfPlusRestore" }, 
    { 0x4027, "EmfPlusBeginContainer" }, 
    { 0x4028, "EmfPlusBeginContainerNoParams" }, 
    { 0x4029, "EmfPlusEndContainer" }, 
    { 0x402A, "EmfPlusSetWorldTransform" }, 
    { 0x402B, "EmfPlusResetWorldTransform" }, 
    { 0x402C, "EmfPlusMultiplyWorldTransform" }, 
    { 0x402D, "EmfPlusTranslateWorldTransform" }, 
    { 0x402E, "EmfPlusScaleWorldTransform" }, 
    { 0x402F, "EmfPlusRotateWorldTransform" }, 
    { 0x4030, "EmfPlusSetPageTransform" }, 
    { 0x4031, "EmfPlusResetClip" }, 
    { 0x4032, "EmfPlusSetClipRect" }, 
    { 0x4033, "EmfPlusSetClipPath" }, 
    { 0x4034, "EmfPlusSetClipRegion" }, 
    { 0x4035, "EmfPlusOffsetClip" }, 
    { 0x4036, "EmfPlusDrawDriverstring" }, 
    { 0x4037, "EmfPlusStrokeFillPath" }, 
    { 0x4038, "EmfPlusSerializableObject" }, 
    { 0x4039, "EmfPlusSetTSGraphics" }, 
    { 0x403A, "EmfPlusSetTSClip" }, 
};

bool EmfplusParser::parseRecord(QDataStream &stream, EmfplusDeviceContext &context)
{
    if ( ! m_backend ) {
        qWarning() << "backend not set";
        return false;
    }
    quint32 type;
    quint32 size;

    stream >> type;
    stream >> size;

    {
        QString name;
        if (0x4000 < type && type < EMFPLUS_LASTRECORD)
            name = EmfplusRecords[type &0xff].name;
        else
            name = "(out of bounds)";
#if DEBUG_EMFPARSER
        kDebug(31000) << "type " << hex << type << "(" << dec << type << ")" << name;
#endif
    }

#if DEBUG_EMFPARSER == 2
    soakBytes(stream, size - 8);
#else
    switch ( type ) {
    case EmfPlusHeader:
        {
        }
        break;
    case EmfPlusEndOfFile:
        {
        }
        break;
    case EmfPlusComment:
        {
        }
        break;
    case EmfPlusGetDC:
        {
        }
        break;
    case EmfPlusMultiFormatStart:
        {
        }
        break;
    case EmfPlusMultiFormatSection:
        {
        }
        break;
    case EmfPlusMultiFormatEnd:
        {
        }
        break;
    case EmfPlusObject:
        {
        }
        break;
    case EmfPlusClear:
        {
        }
        break;
    case EmfPlusFillRects:
        {
        }
        break;
    case EmfPlusDrawRects:
        {
        }
        break;
    case EmfPlusFillPolygon:
        {
        }
        break;
    case EmfPlusDrawLines:
        {
        }
        break;
    case EmfPlusFillEllipse:
        {
        }
        break;
    case EmfPlusDrawEllipse:
        {
        }
        break;
    case EmfPlusFillPie:
        {
        }
        break;
    case EmfPlusDrawPie:
        {
        }
        break;
    case EmfPlusDrawArc:
        {
        }
        break;
    case EmfPlusFillRegion:
        {
        }
        break;
    case EmfPlusFillPath:
        {
        }
        break;
    case EmfPlusDrawPath:
        {
        }
        break;
    case EmfPlusFillClosedCurve:
        {
        }
        break;
    case EmfPlusDrawClosedCurve:
        {
        }
        break;
    case EmfPlusDrawCurve:
        {
        }
        break;
    case EmfPlusDrawBeziers:
        {
        }
        break;
    case EmfPlusDrawImage:
        {
        }
        break;
    case EmfPlusDrawImagePoints:
        {
        }
        break;
    case EmfPlusDrawString:
        {
        }
        break;
    case EmfPlusSetRenderingOrigin:
        {
        }
        break;
    case EmfPlusSetAntiAliasMode:
        {
        }
        break;
    case EmfPlusSetTextRenderingHint:
        {
        }
        break;
    case EmfPlusSetTextContrast:
        {
        }
        break;
    case EmfPlusSetInterpolationMode:
        {
        }
        break;
    case EmfPlusSetPixelOffsetMode:
        {
        }
        break;
    case EmfPlusSetCompositingMode:
        {
        }
        break;
    case EmfPlusSetCompositingQuality:
        {
        }
        break;
    case EmfPlusSave:
        {
        }
        break;
    case EmfPlusRestore:
        {
        }
        break;
    case EmfPlusBeginContainer:
        {
        }
        break;
    case EmfPlusBeginContainerNoParams:
        {
        }
        break;
    case EmfPlusEndContainer:
        {
        }
        break;
    case EmfPlusSetWorldTransform:
        {
        }
        break;
    case EmfPlusResetWorldTransform:
        {
        }
        break;
    case EmfPlusMultiplyWorldTransform:
        {
        }
        break;
    case EmfPlusTranslateWorldTransform:
        {
        }
        break;
    case EmfPlusScaleWorldTransform:
        {
        }
        break;
    case EmfPlusRotateWorldTransform:
        {
        }
        break;
    case EmfPlusSetPageTransform:
        {
        }
        break;
    case EmfPlusResetClip:
        {
        }
        break;
    case EmfPlusSetClipRect:
        {
        }
        break;
    case EmfPlusSetClipPath:
        {
        }
        break;
    case EmfPlusSetClipRegion:
        {
        }
        break;
    case EmfPlusOffsetClip:
        {
        }
        break;
    case EmfPlusDrawDriverstring:
        {
        }
        break;
    case EmfPlusStrokeFillPath:
        {
        }
        break;
    case EmfPlusSerializableObject:
        {
        }
        break;
    case EmfPlusSetTSGraphics:
        {
        }
        break;
    case EmfPlusSetTSClip:
        {
        }
        break;
    default:
#if DEBUG_EMFPARSER
        kDebug(31000) << "unknown record type:" << type;
#endif
	soakBytes( stream, size-8 ); // because we already took 8.
	Q_ASSERT( type );
    }
#endif

    return true;
}

}
