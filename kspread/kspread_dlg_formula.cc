/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999 Montel Laurent <montell@club-internet.fr>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/



#include "kspread_dlg_formula.h"
#include "kspread_dlg_create.h"
#include "kspread_view.h"
#include "kspread_canvas.h"
#include <kapp.h>
#include <klocale.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <kbuttonbox.h>



KSpreaddlgformula::KSpreaddlgformula( KSpreadView* parent, const char* name )
	: QDialog( parent, name )
{
  m_pView = parent;


  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setMargin( 5 );
  lay1->setSpacing( 10 );
  QHBoxLayout *lay2 = new QHBoxLayout( lay1 );
  lay2->setSpacing( 5 );

  type_formula=new QListBox(this);
  lay2->addWidget( type_formula );
  formula=new QListBox(this);
  lay2->addWidget( formula );

  setCaption( i18n("Formula") );

  exp=new QLabel(this);
  exp->layout();
  lay1->addWidget(exp);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  m_pOk = bb->addButton( i18n("OK") );
  m_pOk->setDefault( TRUE );
  m_pClose = bb->addButton( i18n( "Close" ) );
  bb->layout();
  lay1->addWidget( bb );

  type_formula->insertItem(i18n("All"));
  type_formula->insertItem(i18n("Statistic"));
  type_formula->insertItem(i18n("Trigonometric"));
  type_formula->insertItem(i18n("Analytic"));
  type_formula->insertItem(i18n("Logic"));
  type_formula->insertItem(i18n("Text"));
  connect( m_pOk, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
  connect( m_pClose, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
  QObject::connect( type_formula, SIGNAL( highlighted(const QString &) ), this, SLOT( slotselected(const QString &) ) );
  QObject::connect( formula, SIGNAL( highlighted(const QString &) ), this, SLOT( slotselected_formula(const QString &) ) );
  resize( 350, 300 );

}




void KSpreaddlgformula::slotOk()
{

  QString math;
  math=formula->text(formula->currentItem());

  if ( m_pView->activeTable() != 0L )
    {
    KSpreadcreate* dlg = new KSpreadcreate( m_pView, math );
    dlg->show();
    }

  accept();
}

void KSpreaddlgformula::slotClose()
{
    reject();
}

void KSpreaddlgformula::slotselected_formula(const QString & string)
{
/*if(string=="IF")
	{
	exp->clear();
	exp->setText("IF(exp logic,if true so,if false so..)");
	}
else if(string=="true")
	{
	exp->clear();
	exp->setText("TRUE return value TRUE");
	}
else if(string=="false")
	{
	exp->clear();
	exp->setText("FALSE return value FALSE");
	}
else if(string=="NO")
	{
	exp->clear();
	exp->setText("NO(exp logic) :\nif exp==TRUE return FALSE,\nif exp==FALSE return TRUE");
	}
else
	exp->clear();	
*/	
}
void KSpreaddlgformula::slotselected(const QString & string)
{
QStringList list_stat;
list_stat+="average";
list_stat+="variance";
list_stat+="stddev";

QStringList list_anal;
list_anal+="sum";
list_anal+="sqrt";
list_anal+="ln";
list_anal+="log";
list_anal+="exp";
list_anal+="fabs";
list_anal+="floor";
list_anal+="ceil";
list_anal+="max";
list_anal+="min";
list_anal+="multiply";
list_anal+="ENT";
list_anal+="PI";

QStringList list_trig;
list_trig+="cos";
list_trig+="sin";
list_trig+="tan";
list_trig+="acos";
list_trig+="asin";
list_trig+="atan";
list_trig+="cosh";
list_trig+="sinh";
list_trig+="tanh";
list_trig+="acosh";
list_trig+="asinh";
list_trig+="atanh";
list_trig+="degree";
list_trig+="radian";

QStringList list_logic;
list_logic+="if";
list_logic+="not";

QStringList list_text;
list_text+="join";
list_text+="right";
list_text+="left";
list_text+="len";
list_text+="EXACT";
list_text+="STXT";
list_text+="REPT";

if(string== "Statistic" )
	{
	formula->clear();
	formula->insertStringList(list_stat);
	}
if (string =="Trigonometric")
	{
	formula->clear();
	formula->insertStringList(list_trig);
	}
if (string =="Analytic")
	{
	formula->clear();
	formula->insertStringList(list_anal);
	}
if(string== "Logic" )
	{
	formula->clear();
	formula->insertStringList(list_logic);
	}
if(string== "Text" )
	{
	formula->clear();
	formula->insertStringList(list_text);
	}	
if(string == "All")
	{
	formula->clear();
	formula->insertStringList(list_stat);
	formula->insertStringList(list_trig);
	formula->insertStringList(list_anal);
	formula->insertStringList(list_text);
	formula->insertStringList(list_logic);
	}		
}

#include "kspread_dlg_formula.moc"
