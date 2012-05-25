/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#ifndef CHARACTERGENERAL_H
#define CHARACTERGENERAL_H

#include <ui_CharacterGeneral.h>

#include <QWidget>

class KoCharacterStyle;
class FontDecorations;
class CharacterHighlighting;
class LanguageTab;

class CharacterGeneral : public QWidget
{
    Q_OBJECT
public:
    explicit CharacterGeneral(QWidget *parent = 0);

    void setStyle(KoCharacterStyle *style);
    void hideStyleName(bool hide);
    bool isStyleChanged();
    QString styleName() const;
    void selectName();

public slots:
    void save(KoCharacterStyle *style = 0);

    void switchToGeneralTab();

signals:
    void nameChanged(const QString &name);
    void styleAltered(const KoCharacterStyle *style); // when saving
    void styleChanged(); /// when user modifying

private slots:
    void setPreviewCharacterStyle();

protected:
    Ui::CharacterGeneral widget;

private:
    bool m_nameHidden;

    FontDecorations *m_characterDecorations;
    CharacterHighlighting *m_characterHighlighting;
    LanguageTab *m_languageTab;

    KoCharacterStyle *m_style;
};

#endif
