/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __ko_main_window_h__
#define __ko_main_window_h__

#include <shell.h>
#include <kurl.h>

class KoDocument;
class KToolBar;

/**
 * This class is used to represent a main window
 * of a KOffice component. Each main window contains
 * a menubar and some toolbars.
 *
 * If you are going to implement a new KOffice component, then
 * you must implement a subclass of this class.
 */
class KoMainWindow : public Shell
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     *
     *  Initializes a window with a file toolbar.
     */
    KoMainWindow( QWidget* parent = 0, const char *_name = 0 );

    /**
     *  Destructor.
     */
    ~KoMainWindow();

    /**
     *  Retrieves the document that is displayed in the mainwindow.
     */
    KoDocument* document() { return (KoDocument*)rootPart(); }

    /**
     * MimeType of the native file format of the document.
     * For example "application/x-kspread".
     */
    virtual QString nativeFormatMimeType() const = 0;

    /**
     * The pattern of the native file format, for example "*.ksp".
     */
    virtual QString nativeFormatPattern() const = 0;

    /**
     * The name of the native file format. Usually the name of
     * your component, for example "KSpread" or "KWord".
     */
    virtual QString nativeFormatName() const = 0;

    /**
     *  Retrieves the first MainWindow.
     *
     *  @ref nextMainWindow
     */
    static KoMainWindow* firstMainWindow();

    /**
     *  Retrieves the next window.
     *
     *  @ref firstMeinWindow
     */
    static KoMainWindow* nextMainWindow();

    /**
     *  Retrieves a pointer to the file toolbar. Each mainwindow
     *  has this toolbar per default.
     */
    KToolBar *fileToolBar() { return fileTools; }

public slots:

    /**
     *  Slot for opening a new document.
     *
     *  If the current document is empty, the new document replaces the it.
     *  If not a new shell will be opened for showing the document.
     */
    virtual void slotFileNew();

    /**
     *  Slot for opening a saved file.
     *
     *  If the current document is empty, the opened document replaces the it.
     *  If not a new shell will be opened for showing the opened file.
     */
    virtual void slotFileOpen();

    /**
     *  Saves the current document with the current name.
     */
    virtual void slotFileSave();

    /**
     *  Saves the current document with a new name.
     */
    virtual void slotFileSaveAs();

    /**
     *  Prints the actual document.
     */
    virtual void slotFilePrint();

    /**
     *  Closes the actual shell.
     */
    virtual void slotFileClose();

    /**
     *  Quits the Application.
     */
    virtual void slotFileQuit();

    /**
     *  Shows an about dialog for the app.
     */
    virtual void slotHelpAbout();

protected:

    /**
     *  Closes all documents.
     */
    virtual bool closeAllDocuments();

    /**
     *  Closes the actual document.
     */
    virtual bool closeDocument();

    /**
     * Create a new empty document.
     */
    virtual KoDocument* createDoc() = 0;

    /**
     * Load the desired document and show it.
     * @param url the URL to open
     * @param isTempFile true if the file is the result of an import filter
     *
     * @return TRUE on success.
     */
    virtual bool openDocument( const KURL & url, bool isTempFile );

    /**
     * Saves the document, asking for a filename if necessary.
     * Reset the URL of the document to "" in slotFileSaveAs
     *
     * @param _native_format the standard mimetype for your document
     *                       Will allow to use filters if saving to another format
     * @param _native_pattern *.kwd for KWord
     * @param _native_name optional. KWord for KWord :)
     * @param _saveas if set to TRUE the user is always prompted for a filename
     *
     * @return TRUE on success or on cancel, false on error
     *         (don't display anything in this case, the error dialog box is also implemented here
     *         but restore the original URL in slotFileSaveAs)
     */
    virtual bool saveDocument( const char* _native_format, const char* _native_pattern,
			       const char* _native_name = 0L, bool _saveas = FALSE );

private:

    static QList<KoMainWindow>* s_lstMainWindows;
    KToolBar* fileTools;

};

#endif
