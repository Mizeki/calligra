#include "kwframelayout.h"
#include "kwtextframeset.h"
#include "kwdoc.h"

//#define DEBUG_FRAMELAYOUT

#ifdef NDEBUG
#undef DEBUG_FRAMELAYOUT
#endif

KWFrameLayout::HeaderFooterFrameset::HeaderFooterFrameset( KWTextFrameSet* fs, int start, int end,
                                                           double spacing, OddEvenAll oea )
    : m_frameset(fs), m_startAtPage(start), m_endAtPage(end), m_oddEvenAll(oea),
      m_spacing(spacing)
{
    if ( fs->getNumFrames() > 0 )
        m_height = fs->frame(0)->height();
    else
        m_height = 20; // whatever. The text layout will resize it.
}

/*struct PageInfo {
    double top;
    double bottom;
};*/

void KWFrameLayout::layout( KWDocument* doc, KWFrameSet* mainTextFrameSet, int numColumns,
                            QPtrList<HeaderFooterFrameset>& info,
                            int fromPage, int toPage )
{
    // Just debug stuff
#ifdef DEBUG_FRAMELAYOUT
    kdDebug(32002) << "KWFrameLayout::layout " << fromPage << " to " << toPage << endl;
    Q_ASSERT( toPage >= fromPage );
    QPtrListIterator<HeaderFooterFrameset> itdbg( info );
    for ( ; itdbg.current() ; ++itdbg )
    {
        HeaderFooterFrameset* hff = itdbg.current();
        kdDebug(32002) << "  " << hff->m_frameset->getName()
                       << " pages: " << hff->m_startAtPage << "-" << (hff->m_endAtPage==-1?QString("(all)"):QString::number(hff->m_endAtPage))
                       << " page selection: " << (hff->m_oddEvenAll==HeaderFooterFrameset::Odd ? "odd" :
                                                  hff->m_oddEvenAll==HeaderFooterFrameset::Even ? "even" : "all")
                       << " height: " << hff->m_height << " spacing: " << hff->m_spacing << endl;
    }
#endif
    double ptColumnWidth = doc->ptColumnWidth();

    // Prepare page array
    //PageInfo* pagesArray = new PageInfo[ toPage - fromPage + 1 ];
    for ( int pageNum = fromPage ; pageNum <= toPage ; ++pageNum )
    {
        //pagesArray[pageNum].top = doc->ptTopBorder();
        //pagesArray[pageNum].bottom = doc->ptPaperHeight() - doc->ptBottomBorder();
        double top = pageNum * doc->ptPaperHeight() + doc->ptTopBorder();
        double bottom = doc->ptPaperHeight() - doc->ptBottomBorder();
        double left = doc->ptLeftBorder();
        double right = doc->ptPaperWidth() - doc->ptRightBorder();

        // For each frameset.... we have to assume they have been correctly sorted
        QPtrListIterator<HeaderFooterFrameset> it( info );
        for ( ; it.current() ; ++it )
        {
            int frameNum = it.current()->frameNumberForPage( pageNum );
            if ( frameNum != -1 )
            {
                KWTextFrameSet* fs = it.current()->m_frameset;
#ifdef DEBUG_FRAMELAYOUT
                kdDebug(32002) << " KWFrameLayout::layout page " << pageNum << " adding frame " << frameNum << " from " << fs->getName() << endl;
#endif
                KoRect rect;
                if ( fs->isAHeader() ) // add on top
                {
                    rect.setRect( left, top, right - left, it.current()->m_height );
                    top += it.current()->m_height + it.current()->m_spacing;
                } else { // footnote or footer, add at bottom
                    rect.setRect( left, bottom - it.current()->m_height, right - left, it.current()->m_height );
                    bottom -= it.current()->m_height + it.current()->m_spacing;
                }
#ifdef DEBUG_FRAMELAYOUT
                kdDebug(32002) << "     rect:" << rect << "   - new top:bottom: " << top << ":" << bottom << endl;
#endif
                resizeOrCreateHeaderFooter( fs, frameNum, rect );
            }
        }
        // All headers/footers/footnotes for this page have been done,
        // now resize the frame from the main textframeset (if any)
        if ( mainTextFrameSet )
        {
            for ( int col = 0; col < numColumns; col++ ) {
                // Calculate wanted rect for this frame
                KoRect rect( left + col * ( ptColumnWidth + doc->ptColumnSpacing() ),
                             top, ptColumnWidth, bottom - top );
                int frameNum = pageNum * numColumns + col;
#ifdef DEBUG_FRAMELAYOUT
                kdDebug(32002) << " KWFrameLayout::layout page " << pageNum << " resizing main text frame " << frameNum << " to " << rect << endl;
#endif
                if ( frameNum < mainTextFrameSet->getNumFrames() ) {
                    // Resize existing frame
                    mainTextFrameSet->frame( frameNum )->setRect( rect );
                } else {
                    // Create new frame
                    KWFrame * frame = new KWFrame( mainTextFrameSet, rect.x(), rect.y(), rect.width(), rect.height() );
                    mainTextFrameSet->addFrame( frame );
                    Q_ASSERT( frameNum == mainTextFrameSet->getNumFrames()-1 );
                }
            }
        }
    }
    // Final cleanup: delete all frames after m_currentFrame in each frameset
    QPtrListIterator<HeaderFooterFrameset> it( info );
    for ( ; it.current() ; ++it )
    {
        int lastFrame = it.current()->lastFrameNumber( doc->getPages() - 1 );
        KWTextFrameSet* fs = it.current()->m_frameset;
        while ( (int)fs->getNumFrames() - 1 > lastFrame ) {
#ifdef DEBUG_FRAMELAYOUT
            kdDebug(32002) << "KWFrameLayout::resizeOrCreateHeaderFooter deleting frame " << fs->getNumFrames() - 1 << " of " << fs->getName() << endl;
#endif
            fs->delFrame( fs->getNumFrames() - 1 );
        }
    }

    //delete [] pagesArray;
}

void KWFrameLayout::resizeOrCreateHeaderFooter( KWTextFrameSet* headerFooter, uint frameNumber, const KoRect& rect )
{
    if ( frameNumber < headerFooter->getNumFrames() )
        headerFooter->frame( frameNumber )->setRect( rect );
    else
    {
#ifdef DEBUG_FRAMELAYOUT
        kdDebug(32002) << "KWFrameLayout::resizeOrCreateHeaderFooter creating frame for " << headerFooter->getName() << endl;
#endif
        KWFrame *frame = new KWFrame( headerFooter, rect.x(), rect.y(), rect.width(), rect.height() );
        frame->setFrameBehavior( KWFrame::AutoExtendFrame );
        frame->setNewFrameBehavior( KWFrame::Copy );
        frame->setCopy( true );
        headerFooter->addFrame( frame );
    }
}
