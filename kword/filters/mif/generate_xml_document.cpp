/* $Id$
 *
 * This file is part of MIFParse, a MIF parser for Unix.
 *
 * Copyright (C) 1998 by Matthias Kalle Dalheimer <kalle@dalheimer.de>
 */

#include "generate_xml_document.h"
#include "treebuild_document.h"
#include <algorithm>

extern DocumentElementList documentelements;

static bool findPageSize( DocumentElement* );

double generate_xml_document::paperWidth()
{
	// PENDING(kalle) Replace with hash table.
	QListIterator<DocumentElement> dei( documentelements );
	DocumentElement* de = dei.current();
	while( de ) {
		++dei;
		if( findPageSize( de ) )
			break;
		de = dei.current();
	}
	DocumentPageSize* pagesize = de->pageSize();
	// PENDING(kalle) Throw error if 0.
	return pagesize->width();
}

double generate_xml_document::paperHeight()
{
	// PENDING(kalle) Replace with hash table.
	QListIterator<DocumentElement> dei( documentelements );
	DocumentElement* de = dei.current();
	while( de ) {
		++dei;
		if( findPageSize( de ) )
			break;
		de = dei.current();
	}
	DocumentPageSize* pagesize = de->pageSize();
	// PENDING(kalle) Throw error if 0.
	return pagesize->height();
}


bool findPageSize( DocumentElement* el )
{
	if( el->type() == DocumentElement::T_DocumentPageSize )
		return true;
	else
		return false;
}
