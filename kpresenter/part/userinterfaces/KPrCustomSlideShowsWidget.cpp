/* This file is part of the KDE project
   Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>

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
 * Boston, MA 02110-1301, USA.
*/

#include"CustomSlideShowsWidget.h"
#include <QStringList>
#include <QListWidgetItem>
#include <QIcon>

KPrCustomSlideShowsWidget::KPrCustomSlideShowsWidget( QWidget *parent, 
                                                      KPrCustomSlideShows *slideShows,
                                                      QList<KoPAPageBase*> *allPages )
: QDialog(parent), m_slideShows(slideShows)
{
    m_uiWidget.customSlideshowNamesList->addItems( QStringList( m_slideShows->customSlideShowsNames() ));
    const unsigned int pagesCount( allPages->count() );
    for( int i=0; i<=pagesCount; ++i ) {
        m_uiWidget.avaliableSlidesList.insertItem( 
            QListWidgetItem(
                m_slideShows->customSlideShowsNames().at( i ),
                QIcon() );
    }
//         const unsigned int slideShowsCount( allPages->count() );
//     for( int i=0; i<=slideShowsCount; ++i ) {
//         m_uiWidget.currentSlideshowSlidesList.insertItem( 
//             QListWidgetItem(
//                 m_slideShows->customSlideShowsNames().at( i ),
//                 QIcon() );
//     }
}

KPrCustomSlideShowsWidget::~KPrCustomSlideShowsWidget()
{
    delete m_slideShows;
}

#include"KPrCustomSlideShowsWidget.moc"