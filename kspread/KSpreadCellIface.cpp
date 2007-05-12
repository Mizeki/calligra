/* This file is part of the KDE project

   Copyright 2002-2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2002-2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2002 Laurent Montel <montel@kde.org>
   Copyright 2000-2001 David Faure <faure@kde.org>
   Copyright 1999-2000 Torben Weis <weis@kde.org>

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

#include "KSpreadCellIface.h"

#include "Doc.h"
#include "Sheet.h"
#include "Value.h"
#include "Region.h"
#include "ValueConverter.h"

using namespace KSpread;

CellIface::CellIface()
  : m_point( 0, 0 ), m_sheet( 0 )
{
}

void CellIface::setCell( Sheet* sheet, const QPoint& point )
{
    m_sheet = sheet;
    m_point = point;
}

bool CellIface::isDefault() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_Cell( sheet, m_point );
    return cell->isDefault();
}

bool CellIface::isEmpty() const
{
	if (!m_sheet) return true;
	Cell *cell=m_Cell( sheet,m_point);
	return cell->isEmpty();
}

QString CellIface::text() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return m_sheet->doc()->converter()->asString (cell->value()).asString();
}

void CellIface::setText( const QString& text )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setCellText( text );
}

QString CellIface::visibleContentAsString() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if (cell->isEmpty()) return QString();
    QString ret;
    ret=cell->value().asString();

    if (ret.isEmpty())
    {
	ret=QString::number(cell->value().asFloat());
    }
    return ret;
}

QString CellIface::comment() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->comment(m_point.x(), m_point.y());
}

void CellIface::setComment( const QString& comment )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setComment( comment);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setValue( int value )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setValue( (double)value );
}

void CellIface::setValue( double value )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->setValue( value );
}

double CellIface::value() const
{
    if( !m_sheet ) return 0.0;
    Cell* cell = m_Cell( sheet, m_point );
    return m_sheet->doc()->converter()->asFloat (cell->value()).asFloat();
}

void CellIface::setBgColor(const QString& _c)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(_c);
    cell->format()->setBgColor(c);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setBgColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setBgColor(c);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::bgColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->bgColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::textColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->textColor( m_point.x(), m_point.y() ).name();
}

void CellIface::setTextColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setTextColor(c);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setTextColor(const QString& _c)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(_c);
    cell->format()->setTextColor(c);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setAngle(int angle)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setAngle(angle);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int  CellIface::angle() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->getAngle(m_point.x(), m_point.y());
}

void CellIface::setVerticalText(bool _vertical)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setVerticalText(_vertical);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

bool CellIface::verticalText() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->verticalText( m_point.x(), m_point.y() );
}


void CellIface::setMultiRow(bool _multi)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setMultiRow( _multi );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

bool CellIface::multiRow() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->multiRow( m_point.x(), m_point.y() );
}

void CellIface::setAlign( const QString& _Align )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    Style::HAlign Align;
    if(_Align=="Left")
        Align=Style::Left;
    else if(_Align=="Right")
        Align=Style::Right;
    else if(_Align=="Center")
        Align=Style::Center;
    else
        Align=Style::HAlignUndefined;
    cell->format()->setAlign( Align);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::align() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QString alignString;
    switch( cell->format()->align( m_point.x(), m_point.y() ) )
        {
        case Style::Left :
                alignString="Left";
                break;
        case Style::Right :
                alignString="Right";
                break;
        case Style::Center :
                alignString="Center";
                break;
        case Style::HAlignUndefined :
                alignString="Undefined";
                break;
        }
    return alignString;
}

void CellIface::setAlignY( const QString& _AlignY )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    Style::VAlign AlignY;
    if(_AlignY=="Top")
        AlignY=Style::Top;
    else if(_AlignY=="Middle")
        AlignY=Style::Middle;
    else if(_AlignY=="Bottom")
        AlignY=Style::Bottom;
    else
        AlignY=Style::Middle;
    cell->format()->setAlignY( AlignY);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::alignY() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QString alignString;
    switch( cell->format()->alignY( m_point.x(), m_point.y() ) )
        {
        case Style::Top :
                alignString="Top";
                break;
        case Style::Middle :
                alignString="Middle";
                break;
        case Style::Bottom :
                alignString="Bottom";
                break;
        case Style::VAlignUndefined :
                alignString="UndefinedY";
                break;
        }
    return alignString;
}

void CellIface::setPostfix(const QString &_postfix)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setPostfix( _postfix );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::prefix() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->prefix( m_point.x(), m_point.y() );
}

void CellIface::setPrefix(const QString &_prefix)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setPrefix( _prefix );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::postfix() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->postfix( m_point.x(), m_point.y() );
}

void CellIface::setFormatType(const QString& formatType)
{
    if( !m_sheet ) return;

    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    FormatType format;
    cell->format()->setPrecision(2);
    if (formatType=="Generic")
        format = Format::Generic;
    else if(formatType=="Number")
        format=Format::Number;
    else if (formatType=="Text")
        format=Format::Text;
    else if(formatType=="Money")
        format=Format::Money;
    else if(formatType=="Percentage")
        format=Format::Percentage;
    else if(formatType=="Scientific")
        format=Format::Scientific;
    else if(formatType=="ShortDate")
        format=Format::ShortDate;
    else if(formatType=="TextDate")
        format=Format::TextDate;
    else if(formatType=="Time")
        format=Format::Time;
    else if(formatType=="SecondeTime")
        format=Format::SecondeTime;
    else if(Type=="fraction_half")
        format=Format::fraction_half;
    else if(Type=="fraction_quarter")
        format=Format::fraction_quarter;
    else if(Type=="fraction_eighth")
        format=Format::fraction_eighth;
    else if(Type=="fraction_sixteenth")
        format=Format::fraction_sixteenth;
    else if(Type=="fraction_tenth")
        format=Format::fraction_tenth;
    else if(Type=="fraction_hundredth")
        format=Format::fraction_hundredth;
    else if(Type=="fraction_one_digit")
        format=Format::fraction_one_digit;
    else if(Type=="fraction_two_digits")
        format=Format::fraction_two_digits;
    else if(Type=="fraction_three_digits")
        format=Format::fraction_three_digits;
    else
        format=Format::Generic;
    cell->format()->setFormatType( format);
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::getFormatType() const
{
    if( !m_sheet ) return QString();

    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QString stringFormat;
    switch( cell->format()->getFormatType(m_point.x(), m_point.y()))
        {
        case Format::Text:
            stringFormat="Text";
            break;
        case Format::Number:
                stringFormat="Number";
                break;
        case Format::Money:
                stringFormat="Money";
                break;
        case Format::Percentage:
                stringFormat="Percentage";
                break;
        case Format::Scientific:
                stringFormat="Scientific";
                break;
        case Format::ShortDate:
                stringFormat="ShortDate";
                break;
        case Format::TextDate:
                stringFormat="TextDate";
                break;
        case Format::Date1:
        case Format::Date2:
        case Format::Date3:
        case Format::Date4:
        case Format::Date5:
        case Format::Date6:
        case Format::Date7:
        case Format::Date8:
        case Format::Date9:
        case Format::Date10:
        case Format::Date11:
        case Format::Date12:
        case Format::Date13:
        case Format::Date14:
        case Format::Date15:
        case Format::Date16:
	case Format::Date17:
	case Format::Date18:
	case Format::Date19:
	case Format::Date20:
	case Format::Date21:
	case Format::Date22:
	case Format::Date23:
	case Format::Date24:
	case Format::Date25:
	case Format::Date26:
                stringFormat="date format";
                break;
        case Format::Time:
                stringFormat="Time";
                break;
        case Format::SecondeTime:
                stringFormat="SecondeTime";
                break;
        case Format::Time1:
        case Format::Time2:
        case Format::Time3:
        case Format::Time4:
        case Format::Time5:
        case Format::Time6:
        case Format::Time7:
        case Format::Time8:
                stringFormat="time format";
                break;
        case fraction_half:
                stringFormat="fraction_half";
                break;
        case fraction_quarter:
                stringFormat="fraction_quarter";
                break;
        case fraction_eighth:
                stringFormat="fraction_eighth";
                break;
        case fraction_sixteenth:
                stringFormat="fraction_sixteenth";
                break;
        case fraction_tenth:
                stringFormat="fraction_tenth";
                break;
        case fraction_hundredth:
                stringFormat="fraction_hundredth";
                break;
        case fraction_one_digit:
                stringFormat="fraction_one_digit";
                break;
        case fraction_two_digits:
                stringFormat="fraction_two_digits";
                break;
        case fraction_three_digits:
                stringFormat="fraction_three_digits";
                break;
        default:
                break;
        }
    return stringFormat;
}

void CellIface::setPrecision(int _p)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setPrecision( _p );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int CellIface::precision() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->precision( m_point.x(), m_point.y() );
}

void CellIface::setTextFontBold(bool _b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTextFontBold( _b );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

bool CellIface::textFontBold() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->textFontBold( m_point.x(), m_point.y() );
}

void CellIface::setTextFontItalic(bool _b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTextFontItalic( _b );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

bool CellIface::textFontItalic() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->textFontItalic( m_point.x(), m_point.y() );
}


void CellIface::setTextFontUnderline(bool _b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTextFontUnderline( _b );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

bool CellIface::textFontUnderline() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->textFontUnderline( m_point.x(), m_point.y() );
}

void CellIface::setTextFontStrike(bool _b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTextFontStrike( _b );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

bool CellIface::textFontStrike() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->textFontStrike( m_point.x(), m_point.y() );
}

void CellIface::setTextFontSize( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTextFontSize( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int CellIface::textFontSize() const
{
    if( !m_sheet ) return 10;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->textFontSize( m_point.x(), m_point.y() );
}

void CellIface::setTextFontFamily( const QString& _font )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTextFontFamily( _font );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

QString CellIface::textFontFamily() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->textFontFamily( m_point.x(), m_point.y() );
}

//border left
void CellIface::setLeftBorderStyle( const QString& _style )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
      cell->format()->setLeftBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
      cell->format()->setLeftBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
      cell->format()->setLeftBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
      cell->format()->setLeftBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
      cell->format()->setLeftBorderStyle(Qt::SolidLine);
    else
      cell->format()->setLeftBorderStyle(Qt::SolidLine);
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setLeftBorderColor(const QString& _c)
{
    if( !m_sheet ) return;
    QColor c(_c);
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setLeftBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setLeftBorderColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setLeftBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setLeftBorderWidth( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setLeftBorderWidth( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}


int  CellIface::leftBorderWidth() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->leftBorderWidth(m_point.x(), m_point.y());
}

QString CellIface::leftBorderColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->leftBorderColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::leftBorderStyle() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    Qt::PenStyle penStyle=cell->format()->leftBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//border right
void CellIface::setRightBorderStyle( const QString& _style )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
      cell->format()->setRightBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
      cell->format()->setRightBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
      cell->format()->setRightBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
      cell->format()->setRightBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
      cell->format()->setRightBorderStyle(Qt::SolidLine);
    else
      cell->format()->setRightBorderStyle(Qt::SolidLine);
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setRightBorderColor(const QString& _c)
{
    if( !m_sheet ) return;
    QColor c(_c);
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setRightBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}
void CellIface::setRightBorderColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setRightBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setRightBorderWidth( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setRightBorderWidth( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

int  CellIface::rightBorderWidth() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->rightBorderWidth(m_point.x(), m_point.y());
}

QString CellIface::rightBorderColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->rightBorderColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::rightBorderStyle() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    Qt::PenStyle penStyle=cell->format()->rightBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//border top
void CellIface::setTopBorderStyle( const QString& _style )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
      cell->format()->setTopBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
      cell->format()->setTopBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
      cell->format()->setTopBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
      cell->format()->setTopBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
      cell->format()->setTopBorderStyle(Qt::SolidLine);
    else
      cell->format()->setTopBorderStyle(Qt::SolidLine);
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setTopBorderColor(const QString& _c)
{
    if( !m_sheet ) return;
    QColor c(_c);
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTopBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}
void CellIface::setTopBorderColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setTopBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setTopBorderWidth( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setTopBorderWidth( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int  CellIface::topBorderWidth() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->topBorderWidth(m_point.x(), m_point.y());
}

QString CellIface::topBorderColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->topBorderColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::topBorderStyle() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    Qt::PenStyle penStyle=cell->format()->topBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//border bottom
void CellIface::setBottomBorderStyle( const QString& _style )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
      cell->format()->setBottomBorderStyle(Qt::DotLine);
    else if(_style=="DashLine")
      cell->format()->setBottomBorderStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
      cell->format()->setBottomBorderStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
      cell->format()->setBottomBorderStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
      cell->format()->setBottomBorderStyle(Qt::SolidLine);
    else
      cell->format()->setBottomBorderStyle(Qt::SolidLine);
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setBottomBorderColor(const QString& _c)
{
    if( !m_sheet ) return;
    QColor c(_c);
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setBottomBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}
void CellIface::setBottomBorderColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setBottomBorderColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setBottomBorderWidth( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setBottomBorderWidth( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int  CellIface::bottomBorderWidth() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->bottomBorderWidth(m_point.x(), m_point.y());
}

QString CellIface::bottomBorderColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->bottomBorderColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::bottomBorderStyle() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    Qt::PenStyle penStyle=cell->format()->bottomBorderStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

//fall back diagonal
void CellIface::setFallDiagonalStyle( const QString& _style )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
      cell->format()->setFallDiagonalStyle(Qt::DotLine);
    else if(_style=="DashLine")
      cell->format()->setFallDiagonalStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
      cell->format()->setFallDiagonalStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
      cell->format()->setFallDiagonalStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
      cell->format()->setFallDiagonalStyle(Qt::SolidLine);
    else
      cell->format()->setFallDiagonalStyle(Qt::SolidLine);
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setFallDiagonalColor(const QString& _c)
{
    if( !m_sheet ) return;
    QColor c(_c);
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setFallDiagonalColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}
void CellIface::setFallDiagonalColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setFallDiagonalColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setFallDiagonalWidth( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setFallDiagonalWidth( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int  CellIface::fallDiagonalWidth() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->fallDiagonalWidth(m_point.x(), m_point.y());
}

QString CellIface::fallDiagonalColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->fallDiagonalColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::fallDiagonalStyle() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    Qt::PenStyle penStyle=cell->format()->fallDiagonalStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}


//GoUpDiagonal
void CellIface::setGoUpDiagonalStyle( const QString& _style )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if(_style=="DotLine")
      cell->format()->setGoUpDiagonalStyle(Qt::DotLine);
    else if(_style=="DashLine")
      cell->format()->setGoUpDiagonalStyle(Qt::DashLine);
    else if(_style=="DashDotLine")
      cell->format()->setGoUpDiagonalStyle(Qt::DashDotLine);
    else if(_style=="DashDotDotLine")
      cell->format()->setGoUpDiagonalStyle(Qt::DashDotDotLine);
    else if(_style=="SolidLine")
      cell->format()->setGoUpDiagonalStyle(Qt::SolidLine);
    else
      cell->format()->setGoUpDiagonalStyle(Qt::SolidLine);
    m_sheet->setRegionPaintDirty(cell->cellRect());

}

void CellIface::setGoUpDiagonalColor(const QString& _c)
{
    if( !m_sheet ) return;
    QColor c(_c);
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setGoUpDiagonalColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());

}
void CellIface::setGoUpDiagonalColor(int r,int g,int b)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    QColor c(r,g,b);
    cell->format()->setGoUpDiagonalColor(c );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

void CellIface::setGoUpDiagonalWidth( int _size )
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setGoUpDiagonalWidth( _size );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

int  CellIface::goUpDiagonalWidth() const
{
    if( !m_sheet ) return 0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->goUpDiagonalWidth(m_point.x(), m_point.y());
}

QString CellIface::goUpDiagonalColor() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    return cell->format()->goUpDiagonalColor( m_point.x(), m_point.y() ).name();
}

QString CellIface::goUpDiagonalStyle() const
{
    if( !m_sheet ) return QString();
    Cell* cell = m_Cell( sheet, m_point );
    Qt::PenStyle penStyle=cell->format()->goUpDiagonalStyle( m_point.x(), m_point.y() );
    QString tmp;
    if( penStyle==Qt::DotLine)
        tmp="DotLine";
    else if( penStyle==Qt::DashLine)
        tmp="DashLine";
    else if( penStyle==Qt::DashDotLine)
        tmp="DashDotLine";
    else if( penStyle==Qt::DashDotDotLine)
        tmp="DashDotDotLine";
    else if( penStyle==Qt::SolidLine)
        tmp="SolidLine";
    else
        tmp="SolidLine";
    return tmp;
}

void CellIface::setIndent(double indent)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if( indent >= 0.0 )
      cell->format()->setIndent( indent );
    else
      cell->format()->setIndent( 0.0 );
    m_sheet->setRegionPaintDirty(cell->cellRect());
}

double CellIface::getIndent() const
{
    if( !m_sheet ) return 0.0;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->getIndent( m_point.x(), m_point.y() );
}

void CellIface::setDontPrintText ( bool _print)
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->format()->setDontPrintText ( _print);
}

bool CellIface::getDontprintText() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    return cell->format()->getDontprintText(m_point.x(), m_point.y());
}

bool CellIface::hasValidation() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
        return true;
    else
        return false;
}

QString CellIface::validationTitle() const
{
    if( !m_sheet ) return "";
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->title;
    }
    else
        return "";
}

QString CellIface::validationMessage() const
{
    if( !m_sheet ) return "";
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->message;
    }
    else
        return "";
}

QStringList CellIface::listValidation() const
{
    if( !m_sheet ) return QStringList();
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->listValidity;
    }
    else
        return QStringList();
}


QString CellIface::validationTitleInfo() const
{
    if( !m_sheet ) return "";
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->titleInfo;
    }
    else
        return "";
}

QString CellIface::validationMessageInfo() const
{
    if( !m_sheet ) return "";
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->messageInfo;
    }
    else
        return "";
}


bool CellIface::displayValidationInformation() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->displayValidationInformation;
    }
    else
        return false;
}

bool CellIface::displayValidationMessage() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->displayMessage;
    }
    else
        return false;
}

bool CellIface::validationAllowEmptyCell() const
{
    if( !m_sheet ) return false;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    if ( cell->getValidity( 0  ) )
    {
        return cell->getValidity( 0  )->allowEmptyCell;
    }
    else
        return false;
}

void CellIface::removeValidity()
{
    if( !m_sheet ) return;
    Cell* cell = m_sheet->nonDefaultCell( m_point.x(), m_point.y() );
    cell->removeValidity();
}
