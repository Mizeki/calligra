/*
  Copyright 2011 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EMFPLUSPARSER_H
#define EMFPLUSPARSER_H

#include "emf_export.h"

#include "EmfAbstractBackend.h"

#include <QString>
#include <QRect> // also provides QSize

/**
   \file

   Primary definitions for EMFPLUS parser
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{

class EmfplusDeviceContext;


/**
    %Parser for EMFPLUS records inside an EMF file.
 */
class EMF_EXPORT EmfplusParser
{
public:
    EmfplusParser();
    ~EmfplusParser();

    /**
     * Parse EMFPLUS records from a stream
     *
     * \param stream the stream to read from
     *
     * \return true on successful parsing, or false on failure
     */
    bool parse(QDataStream &stream);

    /**
       Set the backend for the parser.

       \param backend pointer to a backend implementation
    */
    void setBackend(EmfAbstractBackend *backend);

private:
    // read a single EMF record
    bool parseRecord(QDataStream &stream, EmfplusDeviceContext &context);

    // Pointer to the backend.
    EmfAbstractBackend *m_backend;
};

}

#endif
