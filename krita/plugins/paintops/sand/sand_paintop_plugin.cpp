/*
 *  Copyright 2012 (c) Francisco Fernandes <francisco.fernandes.j@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "sand_paintop_plugin.h"


#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_paintop_registry.h>


#include "kis_sand_paintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_sand_paintop_settings.h"
#include "kis_sand_paintop_settings_widget.h"

#include "kis_global.h"

K_PLUGIN_FACTORY(SandPaintOpPluginFactory, registerPlugin<SandPaintOpPlugin>();)
K_EXPORT_PLUGIN(SandPaintOpPluginFactory("krita"))


/**
 * Construct the Sand plugin and register in the plugin factory
 */

SandPaintOpPlugin::SandPaintOpPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    qDebug() << "Loading Sand Paintop...";

    //Gets the instance of the Paintop Register
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();

    //Creates the paintop plugin
    r->add( new KisSimplePaintOpFactory<KisSandPaintOp,
                KisSandPaintOpSettings,
                KisSandPaintOpSettingsWidget>("sandbrush",
                                              i18n("Sand brush"),
                                              KisPaintOpFactory::categoryExperimental(),
                                              "krita-sand.png"
                                             )
          );
    
    qDebug() << "Sand Paintop loaded!";
}

SandPaintOpPlugin::~SandPaintOpPlugin()
{
    
}

#include "sand_paintop_plugin.moc"
