/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef VDOCUMENT_H
#define VDOCUMENT_H

#include "vlayer.h"

class QDomDocument;
class QDomElement;

// All non-visual, static doc info is in here.
// The karbon part uses this class.
// Filters can use this class as well instead of
// the visually oriented karbon part.
class VDocument
{
public: 
	VDocument();
	~VDocument();

	const QString& mime() { return m_mime; }
	void setMime( const QString& mime ) { m_mime = mime; }

	const QString& version() { return m_version; }
	void setVersion( const QString& version ) { m_version = version; }

	const QString& editor() { return m_editor; }
	void setEditor( const QString& editor ) { m_editor = editor; }

	const QString& syntaxVersion() { return m_syntaxVersion; }
	void setSyntaxVersion( const QString& syntaxVersion ) { m_syntaxVersion = syntaxVersion; }

	void insertLayer( VLayer* layer );

	const VLayerList& layers() const { return m_layers; }

	void save( QDomDocument& doc ) const;
	bool load( const QDomElement& element );

	// manipulate selection:
	const VObjectList& selection() const { return m_selection; }
	void selectAllObjects();    // select all vobjects period.
	void deselectAllObjects();  // unselect all vobjects from all vlayers.
	void selectObject( VObject& object, bool exclusive = false );
	void deselectObject( VObject& object );
	void selectObjectsWithinRect( const KoRect& rect,
								  const double zoomFactor, bool exclusive = false );

	// move up/down within layer
	void moveSelectionToTop();
	void moveSelectionToBottom();
	void moveSelectionDown();
	void moveSelectionUp();

	void insertObject( VObject* object ); // insert a new vobject

	VLayer* activeLayer() const { return m_activeLayer; }   // active layer.

private:
	VLayerList m_layers;			// all layers in this document
	VObjectList m_selection;        // a list of selected objects.
	VLayer* m_activeLayer;			// the active/current layer.

	QString m_mime;
	QString m_version;
	QString m_editor;
	QString m_syntaxVersion;
};

#endif

