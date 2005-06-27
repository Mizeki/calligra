/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qwmatrix.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_transform_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"

double KisSimpleFilterStrategy::valueAt(double t) const {
        /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
        if(t < 0.0) t = -t;
        if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
        return(0.0);
}

double KisBoxFilterStrategy::valueAt(double t) const {
        if((t > -0.5) && (t <= 0.5)) return(1.0);
        return(0.0);
}

double KisTriangleFilterStrategy::valueAt(double t) const {
        if(t < 0.0) t = -t;
        if(t < 1.0) return(1.0 - t);
        return(0.0);
}

double KisBellFilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < .5) return(.75 - (t * t));
        if(t < 1.5) {
                t = (t - 1.5);
                return(.5 * (t * t));
        }
        return(0.0);
}

double KisBSplineFilterStrategy::valueAt(double t) const {
        double tt;

        if(t < 0) t = -t;
        if(t < 1) {
                tt = t * t;
                return((.5 * tt * t) - tt + (2.0 / 3.0));
        } else if(t < 2) {
                t = 2 - t;
                return((1.0 / 6.0) * (t * t * t));
        }
        return(0.0);
}

double KisLanczos3FilterStrategy::valueAt(double t) const {
        if(t < 0) t = -t;
        if(t < 3.0) return(sinc(t) * sinc(t/3.0));
        return(0.0);
}

double KisLanczos3FilterStrategy::sinc(double x) const {
        const double pi=3.1415926535897932385;
        x *= pi;
        if(x != 0) return(sin(x) / x);
        return(1.0);
}

double KisMitchellFilterStrategy::valueAt(double t) const {
        const double B=1.0/3.0;
        const double C=1.0/3.0;
        double tt;

        tt = t * t;
        if(t < 0) t = -t;
        if(t < 1.0) {
                t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt)) + ((-18.0 + 12.0 * B + 6.0 * C) * tt) + (6.0 - 2 * B));
                return(t / 6.0);
        } else if(t < 2.0) {
                t = (((-1.0 * B - 6.0 * C) * (t * tt)) + ((6.0 * B + 30.0 * C) * tt) + ((-12.0 * B - 48.0 * C) * t) + (8.0 * B + 24 * C));
                return(t / 6.0);
                }
        return(0.0);
}


void KisTransformVisitor::transformx(KisPaintDevice *src, KisPaintDevice *dst, Q_INT32 scale, Q_INT32 scaleDenom, Q_INT32  shear, Q_INT32 dx,   
				     KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy)
{
	Q_INT32 x,y,left,top,w,h;
        Q_INT32 center, begin, end;	/* filter calculation variables */
	Q_UINT8 *data;
	Q_UINT8 pixelSize = src->pixelSize();
	KisSelectionSP dstSelection;
	
	if(src->hasSelection())
	{
		QRect r = src->selection()->selectedExactRect();
		r.rect(&left, &top, &w, &h);
		dstSelection = dst->selection();
	}
	else
	{
		src->exactBounds(left, top, w, h);
		dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
	}
		
	double weight;
	double fscale;
	double width = filterStrategy->support();
	Q_INT32  targetW, targetL;
	
	if(scale < 0)
	{
		targetW = -w * scale / scaleDenom;
		fscale = -1.0 / scale;
	}
	else
	{
		targetW = w * scale / scaleDenom;
		fscale = 1.0 / scale;
	}
	
	// Calculate extra width needed due to shear
	Q_INT32 extrawidth =0;
	
	Q_UINT8 *tmpLine = new Q_UINT8[w * pixelSize];
	Q_CHECK_PTR(tmpLine);

	Q_UINT8 *tmpSel = new Q_UINT8[w];
	Q_CHECK_PTR(tmpSel);

	kdDebug(DBG_AREA_CORE) << "w=" << w << " tW" << targetW << " scale=" << scale << " sDenom=" <<scaleDenom << endl;
	kdDebug(DBG_AREA_CORE) << "left="<< left << ",dx=" << dx << endl;
	
	for(y = top; y < top+h; y++)
	{
		if(scale < 0)
			targetL = left * scale / scaleDenom - targetW + dx;
		else
			targetL = (left) * scale / scaleDenom + dx;
		
		// Build a temporary line
		KisHLineIteratorPixel srcIt = src->createHLineIterator(left, y, w, true);
		int i = 0;
		while(!srcIt.isDone())
		{
			Q_UINT8 *data;
			
			if(srcIt.isSelected())
			{
				data = srcIt.rawData();
				memcpy(&tmpLine[i*pixelSize], data, pixelSize);
				
				// XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
				// but for now this will do
				*(data+3) = 0;
				
				tmpSel[i] = 255;
			}
			else
				tmpSel[i] = 0;
			++srcIt;
			i++;
		}

		KisHLineIteratorPixel dstIt = dst->createHLineIterator(targetL, y, targetW, true);
		KisHLineIteratorPixel dstSelIt = dstSelection->createHLineIterator(targetL, y, targetW, true);
		
		i=0;
		while(!dstIt.isDone())
		{
			if(scale < 0)
				center = w + (i * scaleDenom) / scale;
			else
				center = (i * scaleDenom) / scale;
			
			begin = ceil(center - width);
			end = floor(center + width);
			
			for(int srcpos = (int)begin; srcpos <= end; srcpos++)
				weight = filterStrategy->valueAt((center - srcpos) / fscale) / fscale;

			Q_UINT8 selectedness = tmpSel[center];
			if(selectedness)
			{
				data = dstIt.rawData();
				memcpy(data, &tmpLine[center*pixelSize],  pixelSize);
				data = dstSelIt.rawData();
				*data = selectedness;
			}
			
			++dstSelIt;
			++dstIt;
			i++;
		}
		
		//progress info
		emit notifyProgress(this,((y-top) * 100) / h);
		if (m_cancelRequested) {
			break;
		}
	}
	delete [] tmpLine;
	delete [] tmpSel;
}

void KisTransformVisitor::transformy(KisPaintDevice *src, KisPaintDevice *dst, Q_INT32 scale, Q_INT32 scaleDenom, Q_INT32  shear, Q_INT32 dx,   KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy)
{
	Q_INT32 x,y,left,top,w,h;
        Q_INT32 center, begin, end;	/* filter calculation variables */
	Q_UINT8 *data;
	Q_UINT8 pixelSize = src->pixelSize();
	KisSelectionSP dstSelection;
	
	if(src->hasSelection())
	{
		QRect r = src->selection()->selectedExactRect();
		r.rect(&left, &top, &w, &h);
		dstSelection = dst->selection();
	}
	else
	{
		src->exactBounds(left, top, w, h);
		dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
	}
		
	double weight;
	double fscale;
	double width = filterStrategy->support();
	Q_INT32  targetW, targetT;
	
	if(scale < 0)
	{
		// mirroring
		targetW = -h * scale / scaleDenom;
		fscale = -1.0 / scale;
	}
	else
	{
		targetW = h * scale / scaleDenom;
		fscale = 1.0 / scale;
	}
	
	// Calculate extra width needed due to shear
	Q_INT32 extrawidth =0;
	
	Q_UINT8 *tmpLine = new Q_UINT8[h*pixelSize];
	Q_UINT8 *tmpSel = new Q_UINT8[h];
	kdDebug(DBG_AREA_CORE) << "h=" << h << ", tH=" << targetW << endl;
	
	for(x = left; x < left+w; x++)
	{
		if(scale < 0)
			targetT = top * scale / scaleDenom - targetW + dx;
		else
			targetT = (top) * scale / scaleDenom + dx;
		
		// Build a temporary line
		KisVLineIteratorPixel srcIt = src->createVLineIterator(x, top, h, true);
		int i = 0;
		while(!srcIt.isDone())
		{
			Q_UINT8 *data;
			
			if(srcIt.isSelected())
			{
				data = srcIt.rawData();
				memcpy(&tmpLine[i*pixelSize], data, pixelSize);
				
				// XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
				// but for now this will do
				*(data+3) = 0;
				
				tmpSel[i] = 255;
			}
			else
				tmpSel[i] = 0;
			++srcIt;
			i++;
		}

		KisVLineIteratorPixel dstIt = dst->createVLineIterator(x, targetT, targetW, true);
		KisVLineIteratorPixel dstSelIt = dstSelection->createVLineIterator(x, targetT, targetW, true);
		
		i=0;
		while(!dstIt.isDone())
		{
			if(scale < 0)
				center = h + (i * scaleDenom) / scale;
			else
				center = (i * scaleDenom) / scale;
				
			begin = ceil(center - width);
			end = floor(center + width);
			
			for(int srcpos = (int)begin; srcpos <= end; srcpos++)
				weight = filterStrategy->valueAt((center - srcpos) / fscale) / fscale;

			Q_UINT8 selectedness = tmpSel[center];
			if(selectedness)
			{
				data = dstIt.rawData();
				memcpy(data, &tmpLine[center*pixelSize], pixelSize);
				data = dstSelIt.rawData();
				*data = selectedness;
			}
			
			++dstSelIt;
			++dstIt;
			i++;
		}
		
		//progress info
		emit notifyProgress(this,((x-left) * 100) / w);
		if (m_cancelRequested) {
			break;
		}
	}
	delete [] tmpLine;
	delete [] tmpSel;
}

void KisTransformVisitor::transform(double  xscale, double  yscale, 
				    Q_INT32  xshear, Q_INT32  yshear,
				    Q_INT32  xtranslate, Q_INT32  ytranslate,
				    KisProgressDisplayInterface *m_progress, enumFilterType filterType)
{
        double fwidth;

        KisFilterStrategy *filterStrategy = 0;

        switch(filterType){
                case BOX_FILTER:
                        filterStrategy = new KisBoxFilterStrategy();
                        break;
                case TRIANGLE_FILTER:
                        filterStrategy = new KisTriangleFilterStrategy();
                        break;
                case BELL_FILTER:
                        filterStrategy = new KisBellFilterStrategy();
                        break;
                case B_SPLINE_FILTER:
                        filterStrategy = new KisBSplineFilterStrategy();
                        break;
                case FILTER:
                        filterStrategy = new KisSimpleFilterStrategy();
                        break;
                case LANCZOS3_FILTER:
                        filterStrategy = new KisLanczos3FilterStrategy();
                        break;
                case MITCHELL_FILTER:
                        filterStrategy = new KisMitchellFilterStrategy();
                        break;
        }
	
	fwidth = filterStrategy->support();

	Q_INT32 width = m_dev->image()->width();
	Q_INT32 height = m_dev->image()->height();
        
	m_cancelRequested = false;
	
	KisPaintDeviceSP tmpdev = new KisPaintDevice(m_dev->colorStrategy(),"temporary");
	transformx(m_dev, tmpdev, xscale*width, width, xshear, xtranslate, m_progress, filterStrategy);
	if(m_dev->hasSelection())
		m_dev->selection()->clear();
	transformy(tmpdev, m_dev, yscale*height, height, yshear, ytranslate, m_progress, filterStrategy);
	
	//progress info
        emit notifyProgressDone(this);

        //return nRet;
        return;
}
