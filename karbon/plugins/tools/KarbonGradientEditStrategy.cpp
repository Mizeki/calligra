/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonGradientEditStrategy.h"
#include <KarbonGlobal.h>

#include <KoShape.h>
#include <KoViewConverter.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeBorderCommand.h>

#include <QBrush>
#include <QGradient>
#include <QUndoCommand>
#include <QPainter>

#include <math.h>

#include <kdebug.h>

int GradientStrategy::m_handleRadius = 3;
const double stopDistance = 15.0;


GradientStrategy::GradientStrategy( KoShape *shape, const QGradient * gradient, Target target )
    : m_shape( shape ), m_editing( false )
    , m_target( target ), m_gradientLine( 0, 1 )
    , m_selection( None ), m_selectionIndex(0)
    , m_type( gradient->type() )
{
    if( m_target == Fill )
    {
        m_matrix = m_shape->background().matrix() * m_shape->absoluteTransformation( 0 );
    }
    else
    {
        KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( m_shape->border() );
        if( stroke )
            m_matrix = stroke->lineBrush().matrix() * m_shape->absoluteTransformation( 0 );
    }
    m_stops = gradient->stops();
}

void GradientStrategy::setEditing( bool on )
{
    m_editing = on;
    // if we are going into editing mode, save the old background
    // for use inside the command emitted when finished
    if( on )
    {
        if( m_target == Fill )
        {
            m_oldBrush = m_shape->background();
        }
        else
        {
            KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( m_shape->border() );
            if( stroke )
            {
                m_oldStroke = *stroke;
                m_oldBrush = stroke->lineBrush();
            }
        }
        m_newBrush = m_oldBrush;
    }
}

bool GradientStrategy::selectHandle( const QPointF &mousePos, const KoViewConverter &converter )
{
    QRectF hr = handleRect( converter );

    int handleIndex = 0;
    foreach( QPointF handle, m_handles )
    {
        hr.moveCenter( m_matrix.map( handle ) );
        if( hr.contains( mousePos ) )
        {
            setSelection( Handle, handleIndex );
            return true;
        }
        handleIndex++;
    }

    setSelection( None );

    return false;
}

bool GradientStrategy::selectLine( const QPointF &mousePos, const KoViewConverter &converter )
{
    double maxDistance = handleRect( converter ).size().width();
    if( mouseAtLineSegment( mousePos, maxDistance ) )
    {
        m_lastMousePos = mousePos;
        setSelection( Line );
        return true;
    }

    setSelection( None );

    return false;
}

bool GradientStrategy::selectStop( const QPointF &mousePos, const KoViewConverter &converter )
{
    QRectF hr = handleRect( converter );

    QList<StopHandle> handles = stopHandles( converter );

    int stopCount = m_stops.count();
    for( int i = 0; i < stopCount; ++i )
    {
        hr.moveCenter( handles[i].second );
        if( hr.contains( mousePos ) )
        {
            setSelection( Stop, i );
            m_lastMousePos = mousePos;
            return true;
        }
    }

    setSelection( None );

    return false;
}

void GradientStrategy::paintHandle( QPainter &painter, const KoViewConverter &converter, const QPointF &position )
{
    QRectF hr = handleRect( converter );
    hr.moveCenter( position );
    painter.drawRect( hr );
}

void GradientStrategy::paintStops( QPainter &painter, const KoViewConverter &converter )
{
    painter.save();

    QRectF hr = handleRect( converter );

    QPen defPen = painter.pen();
    QList<StopHandle> handles = stopHandles( converter );
    int stopCount = m_stops.count();
    for( int i = 0; i < stopCount; ++i )
    {
        hr.moveCenter( handles[i].second );

        painter.setPen( defPen );
        painter.drawLine( handles[i].first, handles[i].second );
        painter.setBrush( m_stops[i].second );
        painter.setPen( invertedColor( m_stops[i].second ) );
        painter.drawEllipse( hr );
    }

    painter.restore();
}

void GradientStrategy::paint( QPainter &painter, const KoViewConverter &converter, bool selected )
{
    m_shape->applyConversion( painter, converter );

    QPointF startPoint = m_matrix.map( m_handles[m_gradientLine.first] );
    QPointF stopPoint = m_matrix.map( m_handles[m_gradientLine.second] );

    // draw the gradient line
    painter.drawLine( startPoint, stopPoint );

    // draw the gradient stops
    if( selected )
        paintStops( painter, converter );

    // draw the gradient handles
    foreach( QPointF handle, m_handles )
        paintHandle( painter, converter, m_matrix.map( handle ) );
}

double GradientStrategy::projectToGradientLine( const QPointF &point )
{
    QPointF startPoint = m_matrix.map( m_handles[m_gradientLine.first] );
    QPointF stopPoint = m_matrix.map( m_handles[m_gradientLine.second] );
    QPointF diff = stopPoint - startPoint;
    double diffLength = sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
    if( diffLength == 0.0f )
        return 0.0f;
    // project mouse position relative to stop position on gradient line
    double scalar = KarbonGlobal::scalarProduct( point-startPoint, diff / diffLength );
    return scalar /= diffLength;
}

bool GradientStrategy::mouseAtLineSegment( const QPointF &mousePos, double maxDistance )
{
    double scalar = projectToGradientLine( mousePos );
    if( scalar < 0.0 || scalar > 1.0 )
        return false;
    // calculate vector between relative mouse position and projected mouse position
    QPointF startPoint = m_matrix.map( m_handles[m_gradientLine.first] );
    QPointF stopPoint = m_matrix.map( m_handles[m_gradientLine.second] );
    QPointF distVec = startPoint + scalar * (stopPoint-startPoint) - mousePos;
    double dist = distVec.x()*distVec.x() + distVec.y()*distVec.y();
    if( dist > maxDistance*maxDistance )
        return false;

    return true;
}

void GradientStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED( modifiers )

    QMatrix invMatrix = m_matrix.inverted();
    switch( m_selection )
    {
        case Line:
        {
            uint handleCount = m_handles.count();
            QPointF delta = invMatrix.map( mouseLocation ) - invMatrix.map( m_lastMousePos );
            for( uint i = 0; i < handleCount; ++i )
                m_handles[i] += delta;
            m_lastMousePos = mouseLocation;
            break;
        }
        case Handle:
            m_handles[m_selectionIndex] = invMatrix.map( mouseLocation );
            break;
        case Stop:
        {
            double scalar = projectToGradientLine( mouseLocation ); 
            scalar = qMax( 0.0, scalar );
            scalar = qMin( scalar, 1.0 );
            m_stops[m_selectionIndex].first = scalar;
            m_lastMousePos = mouseLocation;
            break;
        }
        default:
            return;
    }

    applyChanges();
}

bool GradientStrategy::handleDoubleClick( const QPointF &mouseLocation )
{
    if( m_selection == Line )
    {
        // double click on gradient line inserts a new gradient stop

        double scalar = projectToGradientLine( mouseLocation );
        // calculate distance to gradient line
        QPointF startPoint = m_matrix.map( m_handles[m_gradientLine.first] );
        QPointF stopPoint = m_matrix.map( m_handles[m_gradientLine.second] );
        QPointF diff = stopPoint - startPoint;
        QPointF diffToLine = startPoint + scalar * diff - mouseLocation;
        if( diffToLine.x()*diffToLine.x() + diffToLine.y()*diffToLine.y() > m_handleRadius*m_handleRadius )
            return false;

        QGradientStop prevStop( -1.0, QColor() );
        QGradientStop nextStop( 2.0, QColor() );
        // find framing gradient stops
        foreach( QGradientStop stop, m_stops )
        {
            if( stop.first > prevStop.first && stop.first < scalar )
                prevStop = stop;
            if( stop.first < nextStop.first && stop.first > scalar )
                nextStop = stop;
        }
        QColor newColor;

        if( prevStop.first < 0.0 )
        {
            // new stop is before the first stop
            newColor = nextStop.second;
        }
        else if( nextStop.first > 1.0 )
        {
            // new stop is after the last stop
            newColor = prevStop.second;
        }
        else
        {
            // linear interpolate colors between framing stops
            QColor prevColor = prevStop.second, nextColor = nextStop.second;
            double colorScale = (scalar - prevStop.first) / (nextStop.first - prevStop.first);
            newColor.setRedF( prevColor.redF() + colorScale * (nextColor.redF()-prevColor.redF()) );
            newColor.setGreenF( prevColor.greenF() + colorScale * (nextColor.greenF()-prevColor.greenF()) );
            newColor.setBlueF( prevColor.blueF() + colorScale * (nextColor.blueF()-prevColor.blueF()) );
            newColor.setAlphaF( prevColor.alphaF() + colorScale * (nextColor.alphaF()-prevColor.alphaF()) );
        }
        m_stops.append( QGradientStop( scalar, newColor ) );
    }
    else if( m_selection == Stop )
    {
        // double click on stop handle removes gradient stop

        // do not allow removing one of the last two stops
        if( m_stops.count() <= 2 )
            return false;
        m_stops.remove( m_selectionIndex );
        setSelection( None );
    }
    else
        return false;

    applyChanges();

    return true;
}

void GradientStrategy::applyChanges()
{
    m_newBrush = brush();
    if( m_target == Fill )
    {
        m_shape->setBackground( m_newBrush );
    }
    else
    {
        KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( m_shape->border() );
        if( stroke )
            stroke->setLineBrush( m_newBrush );
    }
}

QUndoCommand * GradientStrategy::createCommand( QUndoCommand * parent )
{
    if( m_newBrush == m_oldBrush )
        return 0;

    if( m_target == Fill )
    {
        m_shape->setBackground( m_oldBrush );
        return new KoShapeBackgroundCommand( m_shape, m_newBrush, parent );
    }
    else
    {
        KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( m_shape->border() );
        if( stroke )
        {
            *stroke = m_oldStroke;
            KoLineBorder * newStroke = new KoLineBorder( *stroke );
            newStroke->setLineBrush( m_newBrush );
            return new KoShapeBorderCommand( m_shape, newStroke, parent );
        }
    }

    return 0;
}

QRectF GradientStrategy::boundingRect( const KoViewConverter &converter ) const
{
    // calculate the bounding rect of the handles
    QRectF bbox( m_matrix.map( m_handles[0] ), QSize(0,0) );
    for( int i = 1; i < m_handles.count(); ++i )
    {
        QPointF handle = m_matrix.map( m_handles[i] );
        bbox.setLeft( qMin( handle.x(), bbox.left() ) );
        bbox.setRight( qMax( handle.x(), bbox.right() ) );
        bbox.setTop( qMin( handle.y(), bbox.top() ) );
        bbox.setBottom( qMax( handle.y(), bbox.bottom() ) );
    }
    QList<StopHandle> handles = stopHandles( converter );
    foreach( StopHandle stopHandle, handles )
    {
        QPointF handle = stopHandle.second;
        bbox.setLeft( qMin( handle.x(), bbox.left() ) );
        bbox.setRight( qMax( handle.x(), bbox.right() ) );
        bbox.setTop( qMin( handle.y(), bbox.top() ) );
        bbox.setBottom( qMax( handle.y(), bbox.bottom() ) );
    }
    // quick hack for gradient stops
    //bbox.adjust( -stopDistance, -stopDistance, stopDistance, stopDistance );
    return bbox.adjusted( -m_handleRadius, -m_handleRadius, m_handleRadius, m_handleRadius );
}

void GradientStrategy::repaint() const
{
    m_shape->update();
}

const QGradient * GradientStrategy::gradient()
{
    if( m_target == Fill )
    {
        return m_shape->background().gradient();
    }
    else
    {
        KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( m_shape->border() );
        if( ! stroke )
            return 0;
        return stroke->lineBrush().gradient();
    }
}

GradientStrategy::Target GradientStrategy::target() const
{
    return m_target;
}

void GradientStrategy::startDrawing( const QPointF &mousePos )
{
    QMatrix invMatrix = m_matrix.inverted();

    int handleCount = m_handles.count();
    for( int handleId = 0; handleId < handleCount; ++handleId )
        m_handles[handleId] = invMatrix.map( mousePos );

    setSelection( Handle, handleCount-1 );
    setEditing( true );
}

bool GradientStrategy::hasSelection() const
{
    return m_selection != None;
}

KoShape * GradientStrategy::shape()
{
    return m_shape;
}

QGradient::Type GradientStrategy::type() const
{
    return m_type;
}

void GradientStrategy::updateStops()
{
    QBrush brush;
    if( m_target == Fill )
    {
        brush = m_shape->background();
    }
    else
    {
        KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( m_shape->border() );
        if( stroke )
        {
            brush = stroke->lineBrush();
        }
    }
    if( brush.gradient() )
        m_stops = brush.gradient()->stops();
}

void GradientStrategy::setGradientLine( int start, int stop )
{
    m_gradientLine = QPair<int,int>( start, stop );
}

QRectF GradientStrategy::handleRect( const KoViewConverter &converter ) const
{
    return converter.viewToDocument( QRectF( 0, 0, 2*m_handleRadius, 2*m_handleRadius ) );
}

void GradientStrategy::setSelection( SelectionType selection, int index )
{
    m_selection = selection;
    m_selectionIndex = index;
}

QColor GradientStrategy::invertedColor( const QColor &color )
{
    return QColor( 255-color.red(), 255-color.green(), 255-color.blue() );
}

QList<GradientStrategy::StopHandle> GradientStrategy::stopHandles( const KoViewConverter &converter ) const
{
    // get the gradient line start and end point in document coordinates
    QPointF start = m_matrix.map( m_handles[m_gradientLine.first] );
    QPointF stop = m_matrix.map( m_handles[m_gradientLine.second] );

    // calculate orthogonal vector to the gradient line
    // using the cross product of the line vector and the negative z-axis
    QPointF diff = stop-start;
    QPointF ortho( -diff.y(), diff.x() );
    double orthoLength = sqrt( ortho.x()*ortho.x() + ortho.y()*ortho.y() );
    if( orthoLength == 0.0 )
        ortho = QPointF( stopDistance, 0.0f );
    else
        ortho *= stopDistance / orthoLength;

    // make handles have always the same distance to the gradient line
    // independent of acual zooming
    ortho = converter.viewToDocument( ortho );

    QList<StopHandle> handles;
    foreach( QGradientStop stop, m_stops )
    {
        QPointF base = start + stop.first * diff;
        handles.append( StopHandle( base, base + ortho ) );
    }

    return handles;
}

/////////////////////////////////////////////////////////////////
// strategy implementations
/////////////////////////////////////////////////////////////////

LinearGradientStrategy::LinearGradientStrategy( KoShape *shape, const QLinearGradient *gradient, Target target )
: GradientStrategy( shape, gradient, target )
{
    m_handles.append( gradient->start() );
    m_handles.append( gradient->finalStop() );
}

QBrush LinearGradientStrategy::brush()
{
    QLinearGradient gradient( m_handles[start], m_handles[stop] );
    gradient.setStops( m_stops );
    gradient.setSpread( m_oldBrush.gradient()->spread() );
    QBrush brush = QBrush( gradient );
    brush.setMatrix( m_oldBrush.matrix() );
    return brush;
}

RadialGradientStrategy::RadialGradientStrategy( KoShape *shape, const QRadialGradient *gradient, Target target )
: GradientStrategy( shape, gradient, target )
{
    m_handles.append( gradient->center() );
    m_handles.append( gradient->focalPoint() );
    m_handles.append( gradient->center() + QPointF( gradient->radius(), 0 ) );
    setGradientLine( 0, 2 );
}

QBrush RadialGradientStrategy::brush()
{
    QPointF d = m_handles[radius]-m_handles[center];
    double r = sqrt( d.x()*d.x() + d.y()*d.y() );
    QRadialGradient gradient( m_handles[center], r, m_handles[focal] );
    gradient.setStops( m_stops );
    gradient.setSpread( m_oldBrush.gradient()->spread() );
    QBrush brush = QBrush( gradient );
    brush.setMatrix( m_oldBrush.matrix() );
    return brush;
}

ConicalGradientStrategy::ConicalGradientStrategy( KoShape *shape, const QConicalGradient *gradient, Target target )
: GradientStrategy( shape, gradient, target )
{
    double angle = gradient->angle() * M_PI / 180.0;
    double scale = 0.25 * ( shape->size().height() + shape->size().width() );
    m_handles.append( gradient->center() );
    m_handles.append( gradient->center() + scale * QPointF( cos( angle ), -sin( angle ) ) );
}

QBrush ConicalGradientStrategy::brush()
{
    QPointF d = m_handles[direction]-m_handles[center];
    double angle = atan2( -d.y(), d.x() ) / M_PI * 180.0;
    if( angle < 0.0 )
        angle += 360;
    QConicalGradient gradient( m_handles[center], angle );
    gradient.setStops( m_stops );
    gradient.setSpread( m_oldBrush.gradient()->spread() );
    QBrush brush = QBrush( gradient );
    brush.setMatrix( m_oldBrush.matrix() );
    return brush;
}
