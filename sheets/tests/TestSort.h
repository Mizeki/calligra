/* This file is part of the KDE project
   Copyright 2011 Juan Aquino <utcl95@gmail.com>

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
#ifndef CALLIGRA_SHEETS_TEST_SORT
#define CALLIGRA_SHEETS_TEST_SORT

#include <QtWidgets>
#include <QtTest>

namespace Calligra
{
namespace Sheets
{

class TestSort : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void AscendingOrder();
    void DescendingOrder();

};

} // namespace Sheets
} // namespace Calligra

#endif // CALLIGRA_SHEETS_TEST_SORT
