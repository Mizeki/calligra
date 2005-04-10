/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KIS_CIMG_FILTER_H_
#define _KIS_CIMG_FILTER_H_

#include "kis_filter.h"
#include "kis_view.h"
#include <kdebug.h>

#include "CImg.h"

class KisCImgFilterConfiguration : public KisFilterConfiguration
{

public:

	KisCImgFilterConfiguration() {};

public:

	Q_INT32 nb_iter;    // Number of smoothing iterations
	float   dt;         // Time step
	float   dlength;    // Integration step
	float   dtheta;     // Angular step (in degrees)
	float   sigma;      // Structure tensor blurring
	float   power1;     // Diffusion limiter along isophote
	float   power2;     // Diffusion limiter along gradient
	float   gauss_prec; //  Precision of the gaussian function
	bool    onormalize; // Output image normalization (in [0,255])
	bool    linear;     // Use linear interpolation for integration ?
};


class KisCImgFilter : public KisFilter
{
public:
	KisCImgFilter(KisView * view);
public:
	virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
	static inline KisID id() { return KisID("cimg", i18n("Image Restauration (cimg-based)")); };
	virtual bool supportsPainting() { return true; }
public:
	virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
	virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);

private:

	bool process();

       // Compute smoothed structure tensor field G
        void compute_smoothed_tensor();

        // Compute normalized tensor field sqrt(T) in G
        void compute_normalized_tensor();

        // Compute LIC's along different angle projections a_\alpha
        void compute_LIC(int &counter);
        void compute_LIC_back_forward(int x, int y);
        void compute_W(float cost, float sint);

        // Average all the LIC's
        void compute_average_LIC();

        // Prepare datas
        bool prepare();
        bool prepare_restore();
        bool prepare_inpaint();
        bool prepare_resize();
        bool prepare_visuflow();

        // Check arguments
        bool check_args();

        // Clean up memory (CImg datas) to save memory
        void cleanup();

private:
	KisProgressDisplayInterface *m_progress;

        unsigned int nb_iter; // Number of smoothing iterations
        float dt;       // Time step
        float dlength; // Integration step
        float dtheta; // Angular step (in degrees)
        float sigma;  // Structure tensor blurring
        float power1; // Diffusion limiter along isophote
        float power2; // Diffusion limiter along gradient
        float gauss_prec; //  Precision of the gaussian function
        bool onormalize; // Output image normalization (in [0,255])
        bool linear; // Use linear interpolation for integration


        // internal use
        bool restore;
        bool inpaint;
        bool resize;
        const char* visuflow;
        cimg_library::CImg<> dest, sum, W;
        cimg_library::CImg<> img, img0, flow,G;
        cimg_library::CImgl<> eigen;
        cimg_library::CImg<unsigned char> mask;

	

};

#endif
