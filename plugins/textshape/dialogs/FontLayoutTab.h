/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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

#ifndef FONTLAYOUTTAB_H
#define FONTLAYOUTTAB_H

#include <ui_FontLayoutTab.h>

class QButtonGroup;
class KoCharacterStyle;

class FontLayoutTab : public QWidget
{
    Q_OBJECT

public:
    explicit FontLayoutTab(bool withSubSuperScript, bool uniqueFormat, QWidget* parent = 0);
    ~FontLayoutTab() {}

    void setDisplay(KoCharacterStyle *style);
    void save(KoCharacterStyle *style);

private slots:
    void textPositionChanged();
    void hyphenateStateChanged();

private:
    Ui::FontLayoutTab widget;
    QButtonGroup *m_buttonGroup;
    
    bool m_uniqueFormat;

    bool m_ignoreSignals;
    bool m_positionInherited;
    bool m_hyphenateInherited;

};

#endif
