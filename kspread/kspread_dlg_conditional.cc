/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999,2000,2001 Montel Laurent <lmontel@mandrakesoft.com>
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


#include "kspread_dlg_conditional.h"

#include "kspread_canvas.h"
#include "kspread_cell.h"
#include "kspread_doc.h"
#include "kspread_sheet.h"
#include "kspread_style.h"
#include "kspread_style_manager.h"
#include "kspread_view.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <koGlobal.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

KSpreadConditionalWidget::KSpreadConditionalWidget( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
  QGridLayout * Form1Layout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout");

  QGroupBox * groupBox1_3 = new QGroupBox( this, "groupBox1_3" );
  groupBox1_3->setColumnLayout(0, Qt::Vertical );
  groupBox1_3->layout()->setSpacing( 6 );
  groupBox1_3->layout()->setMargin( 11 );
  QGridLayout * groupBox1_3Layout = new QGridLayout( groupBox1_3->layout() );
  groupBox1_3Layout->setAlignment( Qt::AlignTop );

  QLabel * textLabel1_3 = new QLabel( groupBox1_3, "textLabel1_3" );
  groupBox1_3Layout->addWidget( textLabel1_3, 0, 0 );

  m_condition_3 = new QComboBox( false, groupBox1_3, "m_condition_3" );
  groupBox1_3Layout->addWidget( m_condition_3, 0, 1 );

  m_firstValue_3 = new KLineEdit( groupBox1_3, "m_firstValue_3" );
  m_firstValue_3->setEnabled( false );
  groupBox1_3Layout->addWidget( m_firstValue_3, 0, 2 );

  m_secondValue_3 = new KLineEdit( groupBox1_3, "m_secondValue_3" );
  m_secondValue_3->setEnabled( false );
  groupBox1_3Layout->addWidget( m_secondValue_3, 0, 3 );

  m_style_3 = new QComboBox( false, groupBox1_3, "m_style_3" );
  m_style_3->setEnabled( false );
  groupBox1_3Layout->addWidget( m_style_3, 1, 1 );

  QLabel * textLabel2_3 = new QLabel( groupBox1_3, "textLabel2_3" );
  groupBox1_3Layout->addWidget( textLabel2_3, 1, 0 );

  QSpacerItem * spacer = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  groupBox1_3Layout->addItem( spacer, 1, 2 );
  QSpacerItem * spacer_2 = new QSpacerItem( 61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  groupBox1_3Layout->addItem( spacer_2, 1, 3 );

  Form1Layout->addWidget( groupBox1_3, 2, 0 );

  QGroupBox * groupBox1_2 = new QGroupBox( this, "groupBox1_2" );
  groupBox1_2->setColumnLayout(0, Qt::Vertical );
  groupBox1_2->layout()->setSpacing( 6 );
  groupBox1_2->layout()->setMargin( 11 );

  QGridLayout * groupBox1_2Layout = new QGridLayout( groupBox1_2->layout() );
  groupBox1_2Layout->setAlignment( Qt::AlignTop );

  QLabel * textLabel1_2 = new QLabel( groupBox1_2, "textLabel1_2" );
  groupBox1_2Layout->addWidget( textLabel1_2, 0, 0 );

  QLabel * textLabel2_2 = new QLabel( groupBox1_2, "textLabel2_2" );
  groupBox1_2Layout->addWidget( textLabel2_2, 1, 0 );

  m_condition_2 = new QComboBox( false, groupBox1_2, "m_condition_2" );
  groupBox1_2Layout->addWidget( m_condition_2, 0, 1 );

  m_style_2 = new QComboBox( false, groupBox1_2, "m_style_2" );
  m_style_2->setEnabled( false );
  groupBox1_2Layout->addWidget( m_style_2, 1, 1 );

  m_firstValue_2 = new KLineEdit( groupBox1_2, "m_firstValue_2" );
  m_firstValue_2->setEnabled( false );
  groupBox1_2Layout->addWidget( m_firstValue_2, 0, 2 );

  m_secondValue_2 = new KLineEdit( groupBox1_2, "m_secondValue_2" );
  m_secondValue_2->setEnabled( false );

  groupBox1_2Layout->addWidget( m_secondValue_2, 0, 3 );

  QSpacerItem * spacer_3 = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  groupBox1_2Layout->addItem( spacer_3, 1, 2 );
  QSpacerItem * spacer_4 = new QSpacerItem( 61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  groupBox1_2Layout->addItem( spacer_4, 1, 3 );
  Form1Layout->addWidget( groupBox1_2, 1, 0 );

  QGroupBox * groupBox1_1 = new QGroupBox( this, "groupBox1_1" );
  groupBox1_1->setColumnLayout(0, Qt::Vertical );
  groupBox1_1->layout()->setSpacing( 6 );
  groupBox1_1->layout()->setMargin( 11 );

  QGridLayout * groupBox1_1Layout = new QGridLayout( groupBox1_1->layout() );
  groupBox1_1Layout->setAlignment( Qt::AlignTop );

  QLabel * textLabel1_1 = new QLabel( groupBox1_1, "textLabel1_2_2" );
  groupBox1_1Layout->addWidget( textLabel1_1, 0, 0 );

  QLabel * textLabel2_1 = new QLabel( groupBox1_1, "textLabel2_2_2" );
  groupBox1_1Layout->addWidget( textLabel2_1, 1, 0 );

  m_condition_1 = new QComboBox( false, groupBox1_1, "m_condition_1" );
  groupBox1_1Layout->addWidget( m_condition_1, 0, 1 );

  m_style_1 = new QComboBox( false, groupBox1_1, "m_style_1" );
  m_style_1->setEnabled( false );
  groupBox1_1Layout->addWidget( m_style_1, 1, 1 );

  m_firstValue_1 = new KLineEdit( groupBox1_1, "m_firstValue_1" );
  m_firstValue_1->setEnabled( false );
  groupBox1_1Layout->addWidget( m_firstValue_1, 0, 2 );

  m_secondValue_1 = new KLineEdit( groupBox1_1, "m_secondValue_1" );
  m_secondValue_1->setEnabled( false );
  groupBox1_1Layout->addWidget( m_secondValue_1, 0, 3 );

  QSpacerItem * spacer_5 = new QSpacerItem( 41, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  groupBox1_1Layout->addItem( spacer_5, 1, 2 );
  QSpacerItem * spacer_6 = new QSpacerItem( 61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  groupBox1_1Layout->addItem( spacer_6, 1, 3 );

  Form1Layout->addWidget( groupBox1_1, 0, 0 );
  resize( QSize(702, 380).expandedTo( minimumSizeHint() ) );
  clearWState( WState_Polished );

  QStringList list;
  list += i18n( "<none>" );
  list += i18n( "equal to" );
  list += i18n( "greater than" );
  list += i18n( "less than" );
  list += i18n( "equal to or greater than" );
  list += i18n( "equal to or less than" );
  list += i18n( "between" );
  list += i18n( "different from" );

  m_condition_1->clear();
  m_condition_2->clear();
  m_condition_3->clear();
  m_condition_1->insertStringList( list );
  m_condition_2->insertStringList( list );
  m_condition_3->insertStringList( list );

  groupBox1_1->setTitle( i18n( "First Condition" ) );
  groupBox1_2->setTitle( i18n( "Second Condition" ) );
  groupBox1_3->setTitle( i18n( "Third Condition" ) );
  textLabel1_1->setText( i18n( "Cell is" ) );
  textLabel1_2->setText( i18n( "Cell is" ) );
  textLabel1_3->setText( i18n( "Cell is" ) );
  textLabel2_1->setText( i18n( "Cell style" ) );
  textLabel2_2->setText( i18n( "Cell style" ) );
  textLabel2_3->setText( i18n( "Cell style" ) );

  kdDebug() << "Creating connections" << endl;
  connect( m_condition_1, SIGNAL( highlighted( const QString & ) ), this, SLOT( slotTextChanged1( const QString & ) ) );
  connect( m_condition_2, SIGNAL( highlighted( const QString & ) ), this, SLOT( slotTextChanged2( const QString & ) ) );
  connect( m_condition_3, SIGNAL( highlighted( const QString & ) ), this, SLOT( slotTextChanged3( const QString & ) ) );
}

KSpreadConditionalWidget::~KSpreadConditionalWidget()
{
}

void KSpreadConditionalWidget::slotTextChanged1( const QString & text )
{
  kdDebug() << "Text1: " << text << endl;
  if ( text == i18n( "<none>" ) )
  {
    m_firstValue_1->setEnabled( false );
    m_secondValue_1->setEnabled( false );
    m_style_1->setEnabled( false );
  }
  else
  {
    m_condition_2->setEnabled( true );
    m_style_1->setEnabled( true );
    if ( ( text == i18n( "between" ) ) || ( text == i18n( "different from" ) ) )
    {
      m_firstValue_1->setEnabled( true );
      m_secondValue_1->setEnabled( true );
    }
    else
    {
      m_firstValue_1->setEnabled( true );
      m_secondValue_1->setEnabled( false );
    }
  }
}

void KSpreadConditionalWidget::slotTextChanged2( const QString & text )
{
  kdDebug() << "Text2: " << text << endl;
  if ( text == i18n( "<none>" ) )
  {
    m_firstValue_2->setEnabled( false );
    m_secondValue_2->setEnabled( false );
    m_style_2->setEnabled( false );
  }
  else
  {
    m_condition_3->setEnabled( true );
    m_style_2->setEnabled( true );
    if ( ( text == i18n( "between" ) ) || ( text == i18n( "different from" ) ) )
    {
      m_firstValue_2->setEnabled( true );
      m_secondValue_2->setEnabled( true );
    }
    else
    {
      m_firstValue_2->setEnabled( true );
      m_secondValue_2->setEnabled( false );
    }
  }
}

void KSpreadConditionalWidget::slotTextChanged3( const QString & text )
{
  kdDebug() << "Text3: " << text << endl;
  if ( text == i18n( "<none>" ) )
  {
    m_firstValue_3->setEnabled( false );
    m_secondValue_3->setEnabled( false );
    m_style_3->setEnabled( false );
  }
  else
  {
    m_style_3->setEnabled( true );
    if ( ( text == i18n( "between" ) ) || ( text == i18n( "different from" ) ) )
    {
      m_firstValue_3->setEnabled( true );
      m_secondValue_3->setEnabled( true );
    }
    else
    {
      m_firstValue_3->setEnabled( true );
      m_secondValue_3->setEnabled( false );
    }
  }
}

/**
 * Dialog
 */


KSpreadConditionalDlg::KSpreadConditionalDlg( KSpreadView * parent, const char * name,
                                              const QRect & marker )
  : KDialogBase( parent, name, true, "", KDialogBase::Ok | KDialogBase::Cancel,
                 KDialogBase::Ok, false ),
    m_view( parent ),
    m_dlg( new KSpreadConditionalWidget( this ) ),
    m_marker( marker )
{
  QStringList list( m_view->doc()->styleManager()->styleNames() );

  m_dlg->m_style_1->insertStringList( list );
  m_dlg->m_style_2->insertStringList( list );
  m_dlg->m_style_3->insertStringList( list );

  setCaption( i18n( "Conditional Cell Attributes" ) );
  setButtonBoxOrientation( Vertical );
  setMainWidget( m_dlg );

  init();
}

void KSpreadConditionalDlg::init()
{
  QValueList<KSpreadConditional> conditionList;
  QValueList<KSpreadConditional> otherList;
  bool found;
  int numCondition;

  QValueList<KSpreadConditional>::iterator it1;
  QValueList<KSpreadConditional>::iterator it2;

  KSpreadCell * obj = m_view->activeTable()->cellAt( m_marker.left(),
                                                     m_marker.top() );

  conditionList = obj->conditionList();
  /* this is the list, but only display the conditions common to all selected
     cells*/

  for ( int x = m_marker.left(); x <= m_marker.right(); x++ )
  {
    for ( int y = m_marker.top(); y <= m_marker.bottom(); y++ )
    {
      KSpreadCell * obj2 = m_view->activeTable()->cellAt( x, y );
      otherList = obj2->conditionList();

      it1 = conditionList.begin();
      while ( it1 != conditionList.end() )
      {
	found = false;
	for ( it2 = otherList.begin(); !found && it2 != otherList.end(); ++it2 )
	{
	  found = ( (*it1).val1 == (*it2).val1 &&
                    (*it1).val2 == (*it2).val2 &&
                    *(*it1).strVal1 == *(*it2).strVal1 &&
                    *(*it1).strVal2 == *(*it2).strVal2 &&
                    *(*it1).colorcond == *(*it2).colorcond &&
                    *(*it1).fontcond == *(*it2).fontcond &&
                    *(*it1).styleName == *(*it2).styleName &&
                    (*it1).cond == (*it2).cond );
	}

	if ( !found )  /* if it's not here, don't display this condition */
	{
	  it1 = conditionList.remove( it1 );
	}
	else
	{
	  ++it1;
	}
      }
    }
  }

  m_dlg->m_condition_2->setEnabled( false );
  m_dlg->m_condition_3->setEnabled( false );

  m_dlg->m_style_1->setEnabled( false );
  m_dlg->m_style_2->setEnabled( false );
  m_dlg->m_style_3->setEnabled( false );

  numCondition = 0;
  for ( it1 = conditionList.begin();
        numCondition < 3 && it1 != conditionList.end();
        ++it1 )
  {
    init( *it1, numCondition );

    ++numCondition;
  }
}

void KSpreadConditionalDlg::init( KSpreadConditional const & tmp, int numCondition )
{
  QComboBox * cb  = 0;
  QComboBox * sb  = 0;
  KLineEdit * kl1 = 0;
  KLineEdit * kl2 = 0;
  QString value;

  switch( numCondition )
  {
   case 0:
    cb  = m_dlg->m_condition_1;
    sb  = m_dlg->m_style_1;
    kl1 = m_dlg->m_firstValue_1;
    kl2 = m_dlg->m_secondValue_1;
   case 1:
    cb  = m_dlg->m_condition_2;
    sb  = m_dlg->m_style_2;
    kl1 = m_dlg->m_firstValue_2;
    kl2 = m_dlg->m_secondValue_2;
   case 3:
    cb  = m_dlg->m_condition_3;
    sb  = m_dlg->m_style_3;
    kl1 = m_dlg->m_firstValue_3;
    kl2 = m_dlg->m_secondValue_3;
  }

  if ( tmp.styleName )
  {
    sb->setCurrentText( *tmp.styleName );
    sb->setEnabled( true );
  }

  switch( tmp.cond )
  {
   case None :
    break;

   case Equal :
    cb->setCurrentItem( 1 );
    break;

   case Superior :
    cb->setCurrentItem( 2 );
    break;

   case Inferior :
    cb->setCurrentItem( 3 );
    break;

   case SuperiorEqual :
    cb->setCurrentItem( 4 );
    break;

   case InferiorEqual :
    cb->setCurrentItem( 5 );
    break;

   case Between :
    cb->setCurrentItem(6);

    if ( tmp.strVal2 )
      kl2->setText( *tmp.strVal2 );
    else
    {
      value = value.setNum( tmp.val2 );
      kl2->setText( value );
    }
    break;

   case Different :
    cb->setCurrentItem(7);
    if ( tmp.strVal2 )
      kl2->setText( *tmp.strVal2 );
    else
    {
      value = value.setNum( tmp.val2 );
      kl2->setText( value );
    }
    break;
  }

  if ( tmp.cond != None )
  {
    kl1->setEnabled( true );

    if ( tmp.strVal1 )
      kl1->setText( *tmp.strVal1 );
    else
    {
      value = value.setNum( tmp.val1 );
      kl1->setText( value );
    }
  }
}

Conditional KSpreadConditionalDlg::typeOfCondition( QComboBox const * const cb ) const
{
  Conditional result = None;
  switch( cb->currentItem() )
  {
   case 0 :
    result = None;
    break;
   case 1 :
    result = Equal;
    break;
   case 2 :
    result = Superior;
    break;
   case 3 :
    result = Inferior;
    break;
   case 4 :
    result = SuperiorEqual;
    break;
   case 5 :
    result = InferiorEqual;
    break;
   case 6 :
    result = Between;
    break;
   case 7 :
    result = Different;
    break;
   default:
    kdDebug(36001) << "Error in list" << endl;
    break;
  }

  return result;
}

bool KSpreadConditionalDlg::checkInputData( KLineEdit const * const edit1,
                                            KLineEdit const * const edit2 )
{
  bool b1 = false;
  bool b2 = false;

  if ( !edit2->isEnabled() )
    return true;

  edit1->text().toDouble( &b1 );
  edit2->text().toDouble( &b2 );

  if ( b1 != b2 )
  {
    if ( b1 )
      KMessageBox::sorry( 0, i18n( "If the first value is a number the second value has to be a number, too." ) );
    else
      KMessageBox::sorry( 0, i18n( "If the first value is a string the second value has to be a string, too." ) );
    return false;
  }

  return true;
}

bool KSpreadConditionalDlg::checkInputData()
{
  if ( m_dlg->m_firstValue_1->isEnabled() && !checkInputData( m_dlg->m_firstValue_1, m_dlg->m_secondValue_1 ) )
    return false;
  if ( m_dlg->m_firstValue_2->isEnabled() && !checkInputData( m_dlg->m_firstValue_2, m_dlg->m_secondValue_2 ) )
    return false;
  if ( m_dlg->m_firstValue_3->isEnabled() && !checkInputData( m_dlg->m_firstValue_3, m_dlg->m_secondValue_3 ) )
    return false;

  return true;
}

bool KSpreadConditionalDlg::getCondition( KSpreadConditional & newCondition, QComboBox const * const cb,
                                          KLineEdit const * const edit1, KLineEdit const * const edit2,
                                          QComboBox const * const sb, KSpreadStyle * style )
{
  if ( !cb->isEnabled() )
    return false;

  newCondition.cond = typeOfCondition( cb );
  if ( newCondition.cond == None )
    return false;

  bool ok = false;
  double d1 = edit1->text().toDouble( &ok );
  double d2 = 0.0;
  QString * s1 = 0;
  QString * s2 = 0;

  if ( ok )
  {
    if ( edit2->isEnabled() )
      d2 = edit2->text().toDouble( &ok );
    // values are already checked...
  }
  else
  {
    d1 = 0.0;
    s1 = new QString( edit1->text() );

    if ( edit2->isEnabled() )
      s2 = new QString( edit2->text() );
  }

  newCondition.val1      = d1;
  newCondition.val2      = d2;
  newCondition.strVal1   = s1;
  newCondition.strVal2   = s2;
  newCondition.fontcond  = 0;
  newCondition.colorcond = 0;
  newCondition.styleName = new QString( sb->currentText() );
  newCondition.style     = style;

  return true;
}

void KSpreadConditionalDlg::slotOk()
{
  kdDebug() << "slotOk" << endl;

  if ( !checkInputData() )
    return;

  kdDebug() << "Input data is valid" << endl;

  m_view->doc()->emitBeginOperation( false );
  KSpreadStyleManager * manager = m_view->doc()->styleManager();

  QValueList<KSpreadConditional> newList;

  KSpreadConditional newCondition;

  if ( getCondition( newCondition, m_dlg->m_condition_1, m_dlg->m_firstValue_1,
                     m_dlg->m_secondValue_1, m_dlg->m_style_1, manager->style( m_dlg->m_style_1->currentText() ) ) )
    newList.append( newCondition );

  if ( getCondition( newCondition, m_dlg->m_condition_2, m_dlg->m_firstValue_2,
                     m_dlg->m_secondValue_2, m_dlg->m_style_2, manager->style( m_dlg->m_style_1->currentText() ) ) )
    newList.append( newCondition );

  if ( getCondition( newCondition, m_dlg->m_condition_3, m_dlg->m_firstValue_3,
                     m_dlg->m_secondValue_3, m_dlg->m_style_3, manager->style( m_dlg->m_style_1->currentText() ) ) )
    newList.append( newCondition );

  kdDebug() << "Setting conditional list" << endl;
  m_view->activeTable()->setConditional( m_view->selectionInfo(), newList );
  m_view->slotUpdateView( m_view->activeTable(), m_view->selectionInfo()->selection() );

  accept();
}

#include "kspread_dlg_conditional.moc"


