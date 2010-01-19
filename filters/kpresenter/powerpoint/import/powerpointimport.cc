/* This file is part of the KDE project
   Copyright (C) 2005 Yolla Indria <yolla.indria@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <powerpointimport.h>
#include <powerpointimport.moc>

#include <QBuffer>
#include <QColor>
#include <QString>

#include <kdebug.h>
#include <KoFilterChain.h>
#include <KoGlobal.h>
#include <KoUnit.h>
#include <KoOdf.h>
#include <KoOdfWriteStore.h>
#include <kgenericfactory.h>

#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include "libppt.h"
#include "datetimeformat.h"
#include <iostream>
#include <set>
#include <math.h>
#include "pictures.h"

using namespace Libppt;

typedef KGenericFactory<PowerPointImport> PowerPointImportFactory;
K_EXPORT_COMPONENT_FACTORY(libpowerpointimport,
                           PowerPointImportFactory("kofficefilters"))

namespace Libppt
{

inline QString string(const Libppt::UString& str)
{
    return QString::fromRawData(reinterpret_cast<const QChar*>(str.data()), str.length());
}

}

class PowerPointImport::Private
{
public:
    QString inputFile;
    QString outputFile;
    QMap<QByteArray, QString> pictureNames;

    Presentation *presentation;
    DateTimeFormat *dateTime;
};


PowerPointImport::PowerPointImport(QObject*parent, const QStringList&)
        : KoFilter(parent)
{
    d = new Private;
}

PowerPointImport::~PowerPointImport()
{
    delete d;
}

QMap<QByteArray, QString>
createPictures(const char* filename, KoStore* store, KoXmlWriter* manifest)
{
    POLE::Storage storage(filename);
    QMap<QByteArray, QString> fileNames;
    if (!storage.open()) return fileNames;
    POLE::Stream* stream = new POLE::Stream(&storage, "/Pictures");
    while (!stream->eof() && !stream->fail()
            && stream->tell() < stream->size()) {

        PictureReference ref = savePicture(*stream, store);
        if (ref.name.length() == 0) break;
        manifest->addManifestEntry("Pictures/" + ref.name, ref.mimetype);
        fileNames[ref.uid] = ref.name;
    }
    storage.close();
    delete stream;
    return fileNames;
}

KoFilter::ConversionStatus PowerPointImport::convert(const QByteArray& from, const QByteArray& to)
{
    if (from != "application/vnd.ms-powerpoint")
        return KoFilter::NotImplemented;

    if (to != KoOdf::mimeType(KoOdf::Presentation))
        return KoFilter::NotImplemented;

    d->inputFile = m_chain->inputFile();
    d->outputFile = m_chain->outputFile();

    // open inputFile
    d->presentation = new Presentation;
    if (!d->presentation->load(d->inputFile.toLocal8Bit())) {
        delete d->presentation;
        d->presentation = 0;
        return KoFilter::StupidError;
    }

    // create output store
    KoStore* storeout = KoStore::createStore(d->outputFile, KoStore::Write,
                        KoOdf::mimeType(KoOdf::Presentation), KoStore::Zip);
    if (!storeout) {
        kWarning() << "Couldn't open the requested file.";
        return KoFilter::FileNotFound;
    }
    KoOdfWriteStore odfWriter(storeout);
    KoXmlWriter* manifest = odfWriter.manifestWriter(
                                KoOdf::mimeType(KoOdf::Presentation));

    // store the images from the 'Pictures' stream
    storeout->disallowNameExpansion();
    storeout->enterDirectory("Pictures");
    d->pictureNames = createPictures(d->inputFile.toLocal8Bit(),
                                     storeout, manifest);
    storeout->leaveDirectory();

    KoGenStyles styles;
    createMainStyles(styles);

    // store document content
    if (!storeout->open("content.xml")) {
        kWarning() << "Couldn't open the file 'content.xml'.";
        return KoFilter::CreationError;
    }
    storeout->write(createContent(styles));
    storeout->close();
    manifest->addManifestEntry("content.xml", "text/xml");

    // store document styles
    styles.saveOdfStylesDotXml(storeout, manifest);

    // we are done!
    delete d->presentation;

    odfWriter.closeManifestWriter();

    delete storeout;
    d->inputFile.clear();
    d->outputFile.clear();
    d->presentation = 0;

    return KoFilter::OK;
}

void
addElement(KoGenStyle& style, const char* name,
           const QMap<const char*, QString>& m,
           const QMap<const char*, QString>& mtext)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);
    elementWriter.startElement(name);
    QMapIterator<const char*, QString> i(m);
    while (i.hasNext()) {
        i.next();
        elementWriter.addAttribute(i.key(), i.value());
    }
    if (mtext.size()) {
        elementWriter.startElement("style:text-properties");
        QMapIterator<const char*, QString> j(mtext);
        while (j.hasNext()) {
            j.next();
            elementWriter.addAttribute(j.key(), j.value());
        }
        elementWriter.endElement();
    }
    elementWriter.endElement();
    style.addChildElement(name,
                          QString::fromUtf8(buffer.buffer(), buffer.buffer().size()));
}

void collectPropertyIntegerValues(const GroupObject* go,
        const std::string& property, std::set<int>& set)
{
    if (go == NULL) return;
    for (unsigned i = 0; i < go->objectCount(); i++) {
        const Object* object = go->object(i);
        if (object && object->hasProperty(property)) {
            set.insert(object->getIntProperty(property));
            const GroupObject* cgo = dynamic_cast<const GroupObject*>(object);
            if (cgo) {
                collectPropertyIntegerValues(cgo, property, set);
            }
        }
    }
}

void PowerPointImport::createFillImages(KoGenStyles& styles)
{
    // loop over all objects to find all "fillPib" numbers
    // the names are simply "fillImage" with the pib number concatenated
    for (unsigned c = 0; c < d->presentation->slideCount(); c++) {
        Slide* slide = d->presentation->slide(c);
        GroupObject* root = slide->rootObject();
        std::set<int> pibs;
        collectPropertyIntegerValues(root, "fillPib", pibs);
        for (std::set<int>::const_iterator i = pibs.begin(); i != pibs.end(); ++i) {
            KoGenStyle fillImage(KoGenStyle::StyleFillImage);
            fillImage.addAttribute("xlink:href", getPicturePath(*i));
            styles.lookup(fillImage, "fillImage" + QString::number(*i),
                KoGenStyles::DontForceNumbering);
        }
    }
}

void PowerPointImport::createMainStyles(KoGenStyles& styles)
{
    int x = 0;
    int y = 0;
    Slide* master = d->presentation->masterSlide();
    if(!master)
        return;
    QString pageWidth = QString("%1pt").arg((master) ? master->pageWidth() : 0);
    QString pageHeight = QString("%1pt").arg((master) ? master->pageHeight() : 0);
    

    KoGenStyle marker(KoGenStyle::StyleMarker);
    marker.addAttribute("draw:display-name", "msArrowEnd 5");
    marker.addAttribute("svg:viewBox", "0 0 210 210");
    marker.addAttribute("svg:d", "m105 0 105 210h-210z");
    styles.lookup(marker, "msArrowEnd_20_5");

    // add all the fill image definitions
    createFillImages(styles);

    KoGenStyle pl(KoGenStyle::StylePageLayout);
    pl.setAutoStyleInStylesDotXml(true);
    pl.addAttribute("style:page-usage", "all");
    pl.addProperty("fo:margin-bottom", "0pt");
    pl.addProperty("fo:margin-left", "0pt");
    pl.addProperty("fo:margin-right", "0pt");
    pl.addProperty("fo:margin-top", "0pt");
    pl.addProperty("fo:page-height", pageHeight);
    pl.addProperty("fo:page-width", pageWidth);
    pl.addProperty("style:print-orientation", "landscape");
    styles.lookup(pl, "pm");

    KoGenStyle dp(KoGenStyle::StyleDrawingPage, "drawing-page");
    dp.setAutoStyleInStylesDotXml(true);
    dp.addProperty("draw:background-size", "border");
    dp.addProperty("draw:fill", "solid");
    dp.addProperty("draw:fill-color", "#ffffff");
    styles.lookup(dp, "dp");

    KoGenStyle p(KoGenStyle::StyleAuto, "paragraph");
    p.setAutoStyleInStylesDotXml(true);
    p.addProperty("fo:margin-left", "0cm");
    p.addProperty("fo:margin-right", "0cm");
    p.addProperty("fo:text-indent", "0cm");
    p.addProperty("fo:font-size", "14pt", KoGenStyle::TextType);
    p.addProperty("style:font-size-asian", "14pt", KoGenStyle::TextType);
    p.addProperty("style:font-size-complex", "14pt", KoGenStyle::TextType);
    styles.lookup(p, "P");

    KoGenStyle l(KoGenStyle::StyleListAuto);
    l.setAutoStyleInStylesDotXml(true);
    QMap<const char*, QString> lmap;
    lmap["text:level"] = "1";
    const char bullet[4] = {0xe2, 0x97, 0x8f, 0};
    lmap["text:bullet-char"] = QString::fromUtf8(bullet);//  "●";
    QMap<const char*, QString> ltextmap;
    ltextmap["fo:font-family"] = "StarSymbol";
    ltextmap["style:font-pitch"] = "variable";
    ltextmap["fo:color"] = "#000000";
    ltextmap["fo:font-size"] = "45%";
    addElement(l, "text:list-level-style-bullet", lmap, ltextmap);
    styles.lookup(l, "L");

    //Creating dateTime class object
    d->dateTime = new DateTimeFormat(master);
    if(d->dateTime)
        d->dateTime->addDateTimeAutoStyles(styles);

    KoGenStyle text(KoGenStyle::StyleTextAuto, "text");
    text.setAutoStyleInStylesDotXml(true);
    text.addProperty("fo:font-size", "12pt");
    text.addProperty("fo:language", "en");
    text.addProperty("fo:country", "US");
    text.addProperty("style:font-size-asian", "12pt");
    text.addProperty("style:font-size-complex", "12pt");


    KoGenStyle Mpr(KoGenStyle::StylePresentationAuto, "presentation");
    Mpr.setAutoStyleInStylesDotXml(true);
    Mpr.addProperty("draw:stroke", "none");
    Mpr.addProperty("draw:fill", "none");
    Mpr.addProperty("draw:fill-color", "#bbe0e3");
    Mpr.addProperty("draw:textarea-horizontal-align", "justify");
    Mpr.addProperty("draw:textarea-vertical-align", "top");
    Mpr.addProperty("fo:wrap-option", "wrap");
    styles.lookup(Mpr, "Mpr");

    KoGenStyle s(KoGenStyle::StyleMaster);
    s.addAttribute("style:page-layout-name", styles.lookup(pl));
    s.addAttribute("draw:style-name", styles.lookup(dp));

    if (master) {
        x = master->pageWidth() - 50;
        y = master->pageHeight() - 50;
    }

    addFrame(s, "page-number", "20pt", "20pt",

             QString("%1pt").arg(x), QString("%1pt").arg(y),
             styles.lookup(p), styles.lookup(text));
    addFrame(s, "date-time", "200pt", "20pt", "20pt",
             QString("%1pt").arg(y), styles.lookup(p), styles.lookup(text));
    styles.lookup(s, "Standard", KoGenStyles::DontForceNumbering);

    //Deleting Datetime.
    delete d->dateTime;
    d->dateTime = NULL;

}

void PowerPointImport::addFrame(KoGenStyle& style, const char* presentationClass,
                                QString width, QString height,
                                QString x, QString y, QString pStyle, QString tStyle)
{
    QBuffer buffer;
    int  headerFooterAtomFlags = 0;
    Slide *master = d->presentation->masterSlide();
    if(master)
        headerFooterAtomFlags = master->headerFooterFlags();
    //QString datetime;

    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter xmlWriter(&buffer);

    xmlWriter.startElement("draw:frame");
    xmlWriter.addAttribute("presentation:style-name", "Mpr1");
    xmlWriter.addAttribute("draw:layer", "layout");
    xmlWriter.addAttribute("svg:width", width);
    xmlWriter.addAttribute("svg:height", height);
    xmlWriter.addAttribute("svg:x", x);
    xmlWriter.addAttribute("svg:y", y);
    xmlWriter.addAttribute("presentation:class", presentationClass);
    xmlWriter.startElement("draw:text-box");
    xmlWriter.startElement("text:p");
    xmlWriter.addAttribute("text:style-name", pStyle);

    if (strcmp(presentationClass, "page-number") == 0) {
        xmlWriter.startElement("text:span");
        xmlWriter.addAttribute("text:style-name", tStyle);
        xmlWriter.startElement("text:page-number");
        xmlWriter.addTextNode("<number>");
        xmlWriter.endElement();//text:page-number
        xmlWriter.endElement(); // text:span
    }

    if (strcmp(presentationClass, "date-time") == 0) {
        //Same DateTime object so no need to pass style name for date time
        if (headerFooterAtomFlags && DateTimeFormat::fHasTodayDate) {
            d->dateTime->addMasterDateTimeSection(xmlWriter, tStyle);
        } else if (headerFooterAtomFlags && DateTimeFormat::fHasUserDate) {
            //Future FixedDate format
        }
    }

    xmlWriter.endElement(); // text:p

    xmlWriter.endElement(); // draw:text-box
    xmlWriter.endElement(); // draw:frame
    style.addChildElement("draw:frame",
                          QString::fromUtf8(buffer.buffer(), buffer.buffer().size()));
}

QByteArray PowerPointImport::createContent(KoGenStyles& styles)
{
    KoXmlWriter* contentWriter;
    QByteArray contentData;
    int  headerFooterAtomFlags = 0;
    QBuffer contentBuffer(&contentData);

    contentBuffer.open(QIODevice::WriteOnly);
    contentWriter = new KoXmlWriter(&contentBuffer);

    contentWriter->startDocument("office:document-content");
    contentWriter->startElement("office:document-content");
    contentWriter->addAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    contentWriter->addAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    contentWriter->addAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    contentWriter->addAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    contentWriter->addAttribute("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
    contentWriter->addAttribute("xmlns:presentation", "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0");
    contentWriter->addAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    contentWriter->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    contentWriter->addAttribute("office:version", "1.0");

    // office:automatic-styles

    Slide *master = d->presentation->masterSlide();
    processDocStyles(master , styles);

    for (unsigned c = 0; c < d->presentation->slideCount(); c++) {
        Slide* slide = d->presentation->slide(c);
        processSlideForStyle(c, slide, styles);
    }

    styles.saveOdfAutomaticStyles(contentWriter, false);

    // office:body

    contentWriter->startElement("office:body");
    contentWriter->startElement("office:presentation");

    if(master)
        headerFooterAtomFlags = master->headerFooterFlags();

    if (headerFooterAtomFlags && DateTimeFormat::fHasTodayDate) {
        contentWriter->startElement("presentation:date-time-decl");
        contentWriter->addAttribute("presentation:name", "dtd1");
        contentWriter->addAttribute("presentation:source", "current-date");
        //contentWriter->addAttribute("style:data-style-name", "Dt1");
        contentWriter->endElement();  // presentation:date-time-decl
    } else if (headerFooterAtomFlags && DateTimeFormat::fHasUserDate) {
        contentWriter->startElement("presentation:date-time-decl");
        contentWriter->addAttribute("presentation:name", "dtd1");
        contentWriter->addAttribute("presentation:source", "fixed");
        //Future - Add Fixed date data here
        contentWriter->endElement();  //presentation:date-time-decl
    }

    for (unsigned c = 0; c < d->presentation->slideCount(); c++) {
        processSlideForBody(c, contentWriter);
    }

    contentWriter->endElement();  // office:presentation

    contentWriter->endElement();  // office:body

    contentWriter->endElement();  // office:document-content
    contentWriter->endDocument();
    delete contentWriter;
    return contentData;
}

void PowerPointImport::processDocStyles(Slide *master, KoGenStyles &styles)
{
    int  headerFooterAtomFlags = 0;
    if(master)
        headerFooterAtomFlags = master->headerFooterFlags();

    KoGenStyle dp(KoGenStyle::StyleDrawingPage, "drawing-page");
    dp.addProperty("presentation:background-objects-visible", "true");

    if (headerFooterAtomFlags && DateTimeFormat::fHasSlideNumber)
        dp.addProperty("presentation:display-page-number", "true");
    else
        dp.addProperty("presentation:display-page-number", "false");

    if (headerFooterAtomFlags && DateTimeFormat::fHasDate)
        dp.addProperty("presentation:display-date-time" , "true");
    else
        dp.addProperty("presentation:display-date-time" , "false");

    styles.lookup(dp, "dp");

    if(master)
        master->setStyleName(styles.lookup(dp));
}

void PowerPointImport::processEllipse(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:ellipse");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->endElement(); // draw:ellipse
}

void PowerPointImport::processRectangle(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject) return;
    if (!xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:rect");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    if (drawObject->hasProperty("libppt:rotation")) {

        double rotAngle = drawObject->getDoubleProperty("libppt:rotation");
        double xMid = (drawObject->left() + 0.5 * drawObject->width());
        double yMid = (drawObject->top() + 0.5 * drawObject->height());
        double xVec = drawObject->left() - xMid;
        double yVec = yMid - drawObject->top();

        double xNew = xVec * cos(rotAngle) - yVec * sin(rotAngle);
        double yNew = xVec * sin(rotAngle) + yVec * cos(rotAngle);
        QString rot = QString("rotate (%1) translate (%2mm %3mm)").arg(rotAngle).arg(xNew + xMid).arg(yMid - yNew);
        xmlWriter->addAttribute("draw:transform", rot);
    } else {
        xmlWriter->addAttribute("svg:x", xStr);
        xmlWriter->addAttribute("svg:y", yStr);
    }
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->endElement(); // draw:rect
}

void PowerPointImport::processRoundRectangle(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());


    if (drawObject->hasProperty("libppt:rotation")) {
        double rotAngle = drawObject->getDoubleProperty("libppt:rotation");



        if (rotAngle > 0.785399) { // > 45 deg
            xmlWriter->addAttribute("svg:width", heightStr);
            xmlWriter->addAttribute("svg:height", widthStr);
            double xMid = (drawObject->left() - 0.5 * drawObject->height());
            double yMid = (drawObject->top() + 0.5 * drawObject->width());
            double xVec = drawObject->left() - xMid;
            double yVec =  drawObject->top() - yMid;

            double xNew = xVec * cos(rotAngle) - yVec * sin(rotAngle);
            double yNew = xVec * sin(rotAngle) + yVec * cos(rotAngle);
            QString rot = QString("rotate (%1) translate (%2mm %3mm)").arg(rotAngle).arg(xNew + xMid).arg(yMid + yNew);
            xmlWriter->addAttribute("draw:transform", rot);
        } else {
            xmlWriter->addAttribute("svg:width", widthStr);
            xmlWriter->addAttribute("svg:height", heightStr);
            double xMid = (drawObject->left() + 0.5 * drawObject->width());
            double yMid = (drawObject->top() + 0.5 * drawObject->height());
            double xVec = drawObject->left() - xMid;
            double yVec = yMid - drawObject->top();

            double xNew = xVec * cos(rotAngle) - yVec * sin(rotAngle);
            double yNew = xVec * sin(rotAngle) + yVec * cos(rotAngle);
            QString rot = QString("rotate (%1) translate (%2mm %3mm)").arg(rotAngle).arg(xNew + xMid).arg(yMid - yNew);
            xmlWriter->addAttribute("draw:transform", rot);
        }


    } else {
        xmlWriter->addAttribute("svg:width", widthStr);
        xmlWriter->addAttribute("svg:height", heightStr);
        xmlWriter->addAttribute("svg:x", xStr);
        xmlWriter->addAttribute("svg:y", yStr);
    }
// xmlWriter->addAttribute( "svg:x", xStr );
// xmlWriter->addAttribute( "svg:y", yStr );

    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:enhanced-geometry");
    xmlWriter->addAttribute("draw:type", "round-rectangle");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 /3");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "right-?f0 ");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "bottom-?f0 ");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "left+?f0 ");
    xmlWriter->addAttribute("draw:name", "f3");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "top+?f0 ");
    xmlWriter->addAttribute("draw:name", "f4");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->endElement(); // draw:enhanced-geometry
    xmlWriter->endElement(); // draw:custom-shape
}

void PowerPointImport::processDiamond(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 0);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 10);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    xmlWriter->addAttribute("draw:type", "diamond");
    xmlWriter->endElement();
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->endElement();
}

void PowerPointImport::processTriangle(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    /* draw IsocelesTriangle or RightTriangle */
    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 2.5);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 0);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 10);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 7.5);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();

    xmlWriter->startElement("draw:enhanced-geometry");

    if (drawObject->hasProperty("draw:mirror-vertical")) {
        xmlWriter->addAttribute("draw:mirror-vertical", "true");
    }
    if (drawObject->hasProperty("draw:mirror-horizontal")) {
        xmlWriter->addAttribute("draw:mirror-horizontal", "true");
    }
    if (drawObject->hasProperty("libppt:rotation")) { // draw:transform="rotate (1.5707963267946) translate (6.985cm 14.181cm)"

        double rotAngle = drawObject->getDoubleProperty("libppt:rotation");
        double xMid = (drawObject->left() + 0.5 * drawObject->width());
        double yMid = (drawObject->top() + 0.5 * drawObject->height());
        QString rot = QString("rotate (%1) translate (%2cm %3cm)").arg(rotAngle).arg(xMid).arg(yMid);
        xmlWriter->addAttribute("draw:transform", rot);
    }
    if (drawObject->shape() == DrawObject::RightTriangle) {
        xmlWriter->addAttribute("draw:type", "right-triangle");
    } else if (drawObject->shape() == DrawObject::IsoscelesTriangle) {
        xmlWriter->addAttribute("draw:type", "isosceles-triangle");
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "$0 ");
        xmlWriter->addAttribute("draw:name", "f0");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "$0 /2");
        xmlWriter->addAttribute("draw:name", "f1");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "?f1 +10800");
        xmlWriter->addAttribute("draw:name", "f2");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "$0 *2/3");
        xmlWriter->addAttribute("draw:name", "f3");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "?f3 +7200");
        xmlWriter->addAttribute("draw:name", "f4");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "21600-?f0 ");
        xmlWriter->addAttribute("draw:name", "f5");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "?f5 /2");
        xmlWriter->addAttribute("draw:name", "f6");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:equation");
        xmlWriter->addAttribute("draw:formula", "21600-?f6 ");
        xmlWriter->addAttribute("draw:name", "f7");
        xmlWriter->endElement();
        xmlWriter->startElement("draw:handle");
        xmlWriter->addAttribute("draw:handle-range-x-maximum", 21600);
        xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
        xmlWriter->addAttribute("draw:handle-position", "$0 top");
        xmlWriter->endElement();
    }

    xmlWriter->endElement();    // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processTrapezoid(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");

    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 2.5);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 0);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    if (drawObject->hasProperty("draw:mirror-vertical")) {
        xmlWriter->addAttribute("draw:mirror-vertical", "true");
    }
    if (drawObject->hasProperty("draw:mirror-horizontal")) {
        xmlWriter->addAttribute("draw:mirror-horizontal", "true");
    }
    xmlWriter->addAttribute("draw:type", "trapezoid");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-$0 ");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 *10/18");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f2 +1750");
    xmlWriter->addAttribute("draw:name", "f3");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f3");
    xmlWriter->addAttribute("draw:name", "f4");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 /2");
    xmlWriter->addAttribute("draw:name", "f5");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f5");
    xmlWriter->addAttribute("draw:name", "f6");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:handle");
    xmlWriter->addAttribute("draw:handle-range-x-maximum", 10800);
    xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
    xmlWriter->addAttribute("draw:handle-position", "$0 bottom");
    xmlWriter->endElement();
    xmlWriter->endElement(); // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processParallelogram(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 6.25);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 4.5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 8.75);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 3.75);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 1.25);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    if (drawObject->hasProperty("draw:mirror-vertical")) {
        xmlWriter->addAttribute("draw:mirror-vertical", "true");
    }
    if (drawObject->hasProperty("draw:mirror-horizontal")) {
        xmlWriter->addAttribute("draw:mirror-horizontal", "true");
    }
    xmlWriter->addAttribute("draw:type", "parallelogram");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 ");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-$0");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 *10/24");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f2 +1750");
    xmlWriter->addAttribute("draw:name", "f3");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f3");
    xmlWriter->addAttribute("draw:name", "f4");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f0 /2");
    xmlWriter->addAttribute("draw:name", "f5");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "10800+?f5");
    xmlWriter->addAttribute("draw:name", "f6");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f0-10800 ");
    xmlWriter->addAttribute("draw:name", "f7");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "if(?f7,?f12,0");
    xmlWriter->addAttribute("draw:name", "f8");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "10800-?f5");
    xmlWriter->addAttribute("draw:name", "f9");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "if(?f7, ?f12, 21600");
    xmlWriter->addAttribute("draw:name", "f10");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f5");
    xmlWriter->addAttribute("draw:name", "f11");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600*10800/?f0");
    xmlWriter->addAttribute("draw:name", "f12");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f12");
    xmlWriter->addAttribute("draw:name", "f13");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:handle");
    xmlWriter->addAttribute("draw:handle-range-x-maximum", 21600);
    xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
    xmlWriter->addAttribute("draw:handle-position", "$0 top");
    xmlWriter->endElement();
    xmlWriter->endElement(); // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processHexagon(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 0);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 10);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    xmlWriter->addAttribute("draw:type", "hexagon");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 ");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-$0");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 *100/234");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f2 +1700");
    xmlWriter->addAttribute("draw:name", "f3");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f3");
    xmlWriter->addAttribute("draw:name", "f4");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:handle");
    xmlWriter->addAttribute("draw:handle-range-x-maximum", 10800);
    xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
    xmlWriter->addAttribute("draw:handle-position", "$0 top");
    xmlWriter->endElement();
    xmlWriter->endElement(); // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processOctagon(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 0);
    xmlWriter->addAttribute("svg:y", 4.782);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 10);
    xmlWriter->addAttribute("svg:y", 4.782);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    xmlWriter->addAttribute("draw:type", "octagon");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "left+$0 ");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "top+$0 ");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "right-$0 ");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "bottom-$0 ");
    xmlWriter->addAttribute("draw:name", "f3");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0 /2");
    xmlWriter->addAttribute("draw:name", "f4");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "left+?f4 ");
    xmlWriter->addAttribute("draw:name", "f5");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "top+?f4 ");
    xmlWriter->addAttribute("draw:name", "f6");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "right-?f4 ");
    xmlWriter->addAttribute("draw:name", "f7");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "bottom-?f4 ");
    xmlWriter->addAttribute("draw:name", "f8");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:handle");
    xmlWriter->addAttribute("draw:handle-range-x-maximum", 10800);
    xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
    xmlWriter->addAttribute("draw:handle-position", "$0 top");
    xmlWriter->endElement();
    xmlWriter->endElement(); // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processArrow(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:enhanced-geometry");

    if (drawObject->shape() == DrawObject::RightArrow)
        xmlWriter->addAttribute("draw:type", "right-arrow");
    else if (drawObject->shape() == DrawObject::LeftArrow)
        xmlWriter->addAttribute("draw:type", "left-arrow");
    else if (drawObject->shape() == DrawObject::UpArrow)
        xmlWriter->addAttribute("draw:type", "up-arrow");
    else if (drawObject->shape() == DrawObject::DownArrow)
        xmlWriter->addAttribute("draw:type", "down-arrow");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$1");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-$1");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "21600-?f1");
    xmlWriter->addAttribute("draw:name", "f3");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f3 *?f0 /10800");
    xmlWriter->addAttribute("draw:name", "f4");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f1 +?f4 ");
    xmlWriter->addAttribute("draw:name", "f5");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f1 *?f0 /10800");
    xmlWriter->addAttribute("draw:name", "f6");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "?f1 -?f6 ");
    xmlWriter->addAttribute("draw:name", "f7");
    xmlWriter->endElement(); // draw:equation
    xmlWriter->startElement("draw:handle");
    if (drawObject->shape() == DrawObject::RightArrow || drawObject->shape() == DrawObject::LeftArrow) {
        xmlWriter->addAttribute("draw:handle-range-x-maximum", 21600);
        xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
        xmlWriter->addAttribute("draw:handle-position", "$0 $1");
        xmlWriter->addAttribute("draw:handle-range-y-maximum", 10800);
        xmlWriter->addAttribute("draw:handle-range-y-minimum", 0);
    } else if (drawObject->shape() == DrawObject::UpArrow || drawObject->shape() == DrawObject::DownArrow) {
        xmlWriter->addAttribute("draw:handle-range-x-maximum", 10800);
        xmlWriter->addAttribute("draw:handle-range-x-minimum", 0);
        xmlWriter->addAttribute("draw:handle-position", "$1 $0");
        xmlWriter->addAttribute("draw:handle-range-y-maximum", 21600);
        xmlWriter->addAttribute("draw:handle-range-y-minimum", 0);
    }
    xmlWriter->endElement(); // draw:handle
    xmlWriter->endElement(); // draw:enhanced-geometry
    xmlWriter->endElement(); // draw:custom-shape
}

void PowerPointImport::processLine(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString x1Str = QString("%1mm").arg(drawObject->left());
    QString y1Str = QString("%1mm").arg(drawObject->top());
    QString x2Str = QString("%1mm").arg(drawObject->left() + drawObject->width());
    QString y2Str = QString("%1mm").arg(drawObject->top() + drawObject->height());

    if (drawObject->hasProperty("draw:mirror-vertical")) {
        QString temp = y1Str;
        y1Str = y2Str;
        y2Str = temp;
    }
    if (drawObject->hasProperty("draw:mirror-horizontal")) {
        QString temp = x1Str;
        x1Str = x2Str;
        x2Str = temp;
    }

    xmlWriter->startElement("draw:line");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:y1", y1Str);
    xmlWriter->addAttribute("svg:y2", y2Str);
    xmlWriter->addAttribute("svg:x1", x1Str);
    xmlWriter->addAttribute("svg:x2", x2Str);
    xmlWriter->addAttribute("draw:layer", "layout");

    xmlWriter->endElement();
}

void PowerPointImport::processSmiley(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 0);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 1.461);
    xmlWriter->addAttribute("svg:y", 1.461);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 0);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 1.461);
    xmlWriter->addAttribute("svg:y", 8.536);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 10);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 8.536);
    xmlWriter->addAttribute("svg:y", 1.461);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    xmlWriter->addAttribute("draw:type", "smiley");
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "$0-15510 ");
    xmlWriter->addAttribute("draw:name", "f0");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "17520-?f0");
    xmlWriter->addAttribute("draw:name", "f1");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:equation");
    xmlWriter->addAttribute("draw:formula", "15510+?f0");
    xmlWriter->addAttribute("draw:name", "f2");
    xmlWriter->endElement();
    xmlWriter->startElement("draw:handle");
    xmlWriter->addAttribute("draw:position", 10800);
    xmlWriter->addAttribute("draw:handle-range-y-maximum", 17520);
    xmlWriter->addAttribute("draw:handle-range-y-minimum", 15510);
    xmlWriter->addAttribute("draw:handle-position", "$0 top");
    xmlWriter->endElement();
    xmlWriter->endElement(); // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processHeart(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:custom-shape");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 1);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 1.43);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 5);
    xmlWriter->addAttribute("svg:y", 10);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:glue-point");
    xmlWriter->addAttribute("svg:x", 8.553);
    xmlWriter->addAttribute("svg:y", 5);
    xmlWriter->endElement();
    xmlWriter->startElement("draw:enhanced-geometry");
    xmlWriter->addAttribute("draw:type", "heart");

    xmlWriter->endElement(); // enhanced-geometry
    xmlWriter->endElement(); // custom-shape
}

void PowerPointImport::processFreeLine(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:path");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->endElement(); // path
}

QString PowerPointImport::getPicturePath(int pib) const
{
    int picturePosition = pib - 1;
    const char* rgbUid = d->presentation->getRgbUid(picturePosition);
    return rgbUid ?"Pictures/" + d->pictureNames[QByteArray(rgbUid, 16)] :"";
}

void PowerPointImport::processPictureFrame(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{
    if (!drawObject || !xmlWriter) return;

    QString url = getPicturePath(drawObject->getIntProperty("pib"));
    QString widthStr = QString("%1mm").arg(drawObject->width());
    QString heightStr = QString("%1mm").arg(drawObject->height());
    QString xStr = QString("%1mm").arg(drawObject->left());
    QString yStr = QString("%1mm").arg(drawObject->top());

    xmlWriter->startElement("draw:frame");
    xmlWriter->addAttribute("draw:style-name", drawObject->graphicStyleName());
    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("draw:layer", "layout");
    xmlWriter->startElement("draw:image");
    xmlWriter->addAttribute("xlink:href", url);
    xmlWriter->addAttribute("xlink:type", "simple");
    xmlWriter->addAttribute("xlink:show", "embed");
    xmlWriter->addAttribute("xlink:actuate", "onLoad");
    xmlWriter->endElement(); // image
    xmlWriter->endElement(); // frame
}

void PowerPointImport::processDrawingObjectForBody(DrawObject* drawObject, KoXmlWriter* xmlWriter)
{

    if (!drawObject || !xmlWriter) return;
    if (drawObject->shape() == DrawObject::Ellipse) {
        processEllipse(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Rectangle) {
        processRectangle(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::RoundRectangle) {
        processRoundRectangle(drawObject, xmlWriter);
    } else  if (drawObject->shape() == DrawObject::Diamond) {
        processDiamond(drawObject, xmlWriter);
    } else  if (drawObject->shape() == DrawObject::IsoscelesTriangle ||
                drawObject->shape() == DrawObject::RightTriangle) {
        processTriangle(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Trapezoid) {
        processTrapezoid(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Parallelogram) {
        processParallelogram(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Hexagon) {
        processHexagon(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Octagon) {
        processOctagon(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::RightArrow ||
               drawObject->shape() == DrawObject::LeftArrow ||
               drawObject->shape() == DrawObject::UpArrow ||
               drawObject->shape() == DrawObject::DownArrow) {
        processArrow(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Line) {
        processLine(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Smiley) {
        processSmiley(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::Heart) {
        processHeart(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::FreeLine) {
        processFreeLine(drawObject, xmlWriter);
    } else if (drawObject->shape() == DrawObject::PictureFrame) {
        processPictureFrame(drawObject, xmlWriter);
    }
}

void PowerPointImport::processGroupObjectForBody(GroupObject* groupObject,
        KoXmlWriter* xmlWriter)
{
    if (!groupObject || !xmlWriter) return;
    if (!groupObject->objectCount()) return;

    xmlWriter->startElement("draw:g");

    for (unsigned i = 0; i < groupObject->objectCount(); i++) {
        Object* object = groupObject->object(i);
        if (object)
            processObjectForBody(object, xmlWriter);
    }

    xmlWriter->endElement(); // draw:g
}

void PowerPointImport::writeTextCFException(KoXmlWriter* xmlWriter,
        TextCFException *cf,
        TextPFException *pf,
        TextObject *textObject,
        const QString &text)
{
    xmlWriter->startElement("text:span");
    xmlWriter->addAttribute("text:style-name", textObject->textStyleName(cf, pf));

    QString copy = text;
    copy.remove(QChar(11)); //Remove vertical tabs which appear in some ppt files
    xmlWriter->addTextSpan(copy);

    xmlWriter->endElement(); // text:span
}

void PowerPointImport::writeTextLine(KoXmlWriter* xmlWriter,
                                     TextPFException *pf,
                                     TextObject *textObject,
                                     const QString &text,
                                     const unsigned int linePosition)
{
    StyleTextPropAtom *atom = textObject->styleTextProperty();
    QString part = "";
    TextCFRun *cf = atom->findTextCFRun(linePosition);

    if (!cf) {
        return;
    }

    if (text.isEmpty()) {
        writeTextCFException(xmlWriter, cf->textCFException(), pf, textObject, text);
        return;
    }

    //Iterate through all the characters in text
    for (int i = 0;i < text.length();i++) {
        TextCFRun *nextCFRun = atom->findTextCFRun(linePosition + i);

        //While character exception stays the same
        if (cf == nextCFRun) {
            //Catenate strings to our substring
            part += text[i];
        } else {
            /*
            When exception changes we write the text to xmlwriter unless the
            text style name stays the same, then we'll reuse the same
            stylename for the next character exception
            */
            if (nextCFRun &&
                    textObject->textStyleName(cf->textCFException(), pf) != textObject->textStyleName(nextCFRun->textCFException(), pf)) {
                writeTextCFException(xmlWriter, cf->textCFException(), pf, textObject, part);
                part = text[i];
            } else {
                part += text[i];
            }

            cf = nextCFRun;
        }
    }

    //If at the end we still have some text left, write it out
    if (!part.isEmpty()) {
        writeTextCFException(xmlWriter, cf->textCFException(), pf, textObject, part);
    }
}

void PowerPointImport::writeTextObjectDeIndent(KoXmlWriter* xmlWriter,
        const unsigned int count, QStack<QString>& levels)
{
    while ((unsigned int)levels.size() > count) {
        // if the style name at the lowest level is empty, there is no
        // list there
        if (levels.size() > 1
                || (levels.size() == 1 && !levels.top().isNull())) {
            xmlWriter->endElement(); // text:list
        }
        levels.pop();
        if (levels.size() > 1
                || (levels.size() == 1 && !levels.top().isNull())) {
            xmlWriter->endElement(); // text:list-item
        }
    }
}
namespace {
    void addListElement(KoXmlWriter* xmlWriter, QStack<QString>& levels,
            const QString& listStyle)
    {
        if (!listStyle.isNull()) {
            // if the context is a text:list, a text:list-item is needed
            if (levels.size() > 1
                    || (levels.size() == 1 && !levels.top().isNull())) {
                xmlWriter->startElement("text:list-item");
            }
            xmlWriter->startElement("text:list");
            if (!listStyle.isEmpty()) {
                xmlWriter->addAttribute("text:style-name", listStyle);
            }
        }
        levels.push(listStyle);
    }
}

void PowerPointImport::writeTextPFException(KoXmlWriter* xmlWriter,
        TextPFRun *pf,
        TextObject *textObject,
        const unsigned int textPos,
        QStack<QString>& levels)
{
    StyleTextPropAtom *atom = textObject->styleTextProperty();
    if (!atom || !pf || !textObject) {
        return;
    }

    QString text = textObject->text().mid(textPos, pf->count());
    //Text lines are separated with carriage return
    //There seems to be an extra carriage return at the end. We'll remove the last
    // carriage return so we don't end up with a single empty line in the end
    if (text.endsWith("\r")) {
        text = text.left(text.length() - 1);
    }

    //Then split the text into lines
    QStringList lines = text.split("\r");
    unsigned int linePos = textPos;

    //Indentation level paragraph wants
    unsigned int paragraphIndent = pf->indentLevel();
    //[MS-PPT].pdf says the itendation level can be 4 at most
    if (paragraphIndent > 4) paragraphIndent = 4;
    bool bullet = false;
    //Check if this paragraph has a bullet
    if (pf->textPFException()->hasBullet()) {
        bullet = pf->textPFException()->bullet();
    } else {
        //If text paragraph exception doesn't have a definition on bullet
        //then we'll have to check master style with our indentation level
        TextPFException *masterPF = masterTextPFException(textObject->type(),
                                    pf->indentLevel());
        if (masterPF && masterPF->hasBullet()) {
            bullet = masterPF->bullet();
        }
    }
    TextCFRun *cf = atom->findTextCFRun(linePos);
    // find the name of the list style, if the current style is a list
    // if the style is null, there is no, list, if the style is an empty string
    // there is a list but it has no style
    QString listStyle;
    if (cf && (bullet || paragraphIndent > 0)) {
        listStyle = textObject->listStyleName(cf->textCFException(),
            pf->textPFException());
    }
    if (listStyle.isNull() && (bullet || paragraphIndent > 0)) {
        listStyle = "";
    }
    // remove levels until the top level is the right indentation
    if ((unsigned int)levels.size() > paragraphIndent
            && levels[paragraphIndent] == listStyle) {
         writeTextObjectDeIndent(xmlWriter, paragraphIndent + 2, levels);
    } else {
         writeTextObjectDeIndent(xmlWriter, paragraphIndent + 1, levels);
    }
    // add styleless levels up to the current level of indentation
    while ((unsigned int)levels.size() < paragraphIndent) {
        addListElement(xmlWriter, levels, "");
    }
    // at this point, levels.size() == paragraphIndent
    if (levels.size() == 0 || listStyle != levels.top()) {
        addListElement(xmlWriter, levels, listStyle);
    }
    bool listItem = levels.size() > 1 || !levels.top().isNull();
    if (listItem) {
        xmlWriter->startElement("text:list-item");
    }
    QString pstyle;
    if (cf) {
        pstyle = textObject->paragraphStyleName(cf->textCFException(),
                pf->textPFException());
    }
    xmlWriter->startElement("text:p");
    if (pstyle.size() > 0) {
        xmlWriter->addAttribute("text:style-name", pstyle);
    }

    //Handle all lines
    for (int i = 0;i < lines.size();i++) {
        cf = atom->findTextCFRun(linePos);
        if (cf) {
            writeTextCFException(xmlWriter, cf->textCFException(),
                pf->textPFException(),
                textObject, lines[i]);
        }

        //Add +1 to line position to compensate for carriage return that is
        //missing from line due to the split method
        linePos += lines[i].size() + 1;
    }
    xmlWriter->endElement(); // text:p
    if (listItem) {
        xmlWriter->endElement(); // text:list-item
    }
}


void PowerPointImport::processTextObjectForBody(TextObject* textObject, KoXmlWriter* xmlWriter)
{
    if (!textObject || !xmlWriter) return;

    QString classStr = "subtitle";

    if (textObject->type() == TextObject::Title)
        classStr = "title";

    if (textObject->type() == TextObject::Body)
        classStr = "outline";

    QString widthStr = QString("%1mm").arg(textObject->width());

    QString heightStr = QString("%1mm").arg(textObject->height());

    QString xStr = QString("%1mm").arg(textObject->left());

    QString yStr = QString("%1mm").arg(textObject->top());

    xmlWriter->startElement("draw:frame");

    if (!textObject->graphicStyleName().isEmpty()) {
        xmlWriter->addAttribute("presentation:style-name",
                textObject->graphicStyleName());
    }

    xmlWriter->addAttribute("draw:layer", "layout");

    xmlWriter->addAttribute("svg:width", widthStr);
    xmlWriter->addAttribute("svg:height", heightStr);
    xmlWriter->addAttribute("svg:x", xStr);
    xmlWriter->addAttribute("svg:y", yStr);
    xmlWriter->addAttribute("presentation:class", classStr);
    xmlWriter->startElement("draw:text-box");

    StyleTextPropAtom *atom = textObject->styleTextProperty();
    if (atom) {
        //Paragraph formatting that applies to substring
        TextPFRun *pf = 0;

        QStack<QString> levels;
        levels.reserve(5);
        pf = atom->findTextPFRun(0);
        unsigned int index = 0;
        int pos = 0;

        while (pos < textObject->text().length()) {
            if (!pf) {
                kWarning() << "Failed to get text paragraph exception!";
                return;
            }

            writeTextPFException(xmlWriter,
                                 pf,
                                 textObject,
                                 pos,
                                 levels);

            pos += pf->count();
            index++;
            pf = atom->textPFRun(index);
        }

        writeTextObjectDeIndent(xmlWriter, 0, levels);
    } else {
        xmlWriter->startElement("text:p");

        if (!textObject->paragraphStyleName(0, 0).isEmpty()) {
            xmlWriter->addAttribute("text:style-name", textObject->paragraphStyleName(0, 0));
        }

        xmlWriter->startElement("text:span");
        if (!textObject->textStyleName(0, 0).isEmpty()) {
            xmlWriter->addAttribute("text:style-name", textObject->textStyleName(0, 0));
        }

        xmlWriter->addTextSpan(textObject->text());
        xmlWriter->endElement(); // text:span
        xmlWriter->endElement(); // text:p
    }

    xmlWriter->endElement(); // draw:text-box
    xmlWriter->endElement(); // draw:frame
}

void PowerPointImport::processObjectForBody(Object* object, KoXmlWriter* xmlWriter)
{
    if (!object ||  !xmlWriter) return;
    if (object->isText())
        processTextObjectForBody(static_cast<TextObject*>(object), xmlWriter);
    else if (object->isGroup())
        processGroupObjectForBody(static_cast<GroupObject*>(object), xmlWriter);
    else if (object->isDrawing())
        processDrawingObjectForBody(static_cast<DrawObject*>(object), xmlWriter);
}



void PowerPointImport::processSlideForBody(unsigned slideNo, KoXmlWriter* xmlWriter)
{
    Slide* slide = d->presentation->slide(slideNo);
    Slide* master = d->presentation->masterSlide();

    if (!slide || !xmlWriter || !master) return;

    QString nameStr = slide->title();

    if (nameStr.isEmpty())
        nameStr = QString("page%1").arg(slideNo + 1);

    //QString styleNameStr = QString("dp%1").arg(slideNo + 1);

    xmlWriter->startElement("draw:page");

    xmlWriter->addAttribute("draw:master-page-name", "Default");

    xmlWriter->addAttribute("draw:name", nameStr);

    xmlWriter->addAttribute("draw:style-name", master->styleName());

    xmlWriter->addAttribute("presentation:presentation-page-layout-name", "AL1T0");

    int  headerFooterAtomFlags = d->presentation->masterSlide()->headerFooterFlags();

    if (headerFooterAtomFlags && DateTimeFormat::fHasTodayDate) {
        xmlWriter->addAttribute("presentation:use-date-time-name", "dtd1");
    }

    GroupObject* root = slide->rootObject();

    if (root)
        for (unsigned i = 0; i < root->objectCount(); i++) {
            Object* object = root->object(i);

            if (object)
                processObjectForBody(object, xmlWriter);
        }

    xmlWriter->endElement(); // draw:page
}

void PowerPointImport::processSlideForStyle(unsigned , Slide* slide, KoGenStyles &styles)
{
    if (!slide) return;

    GroupObject* root = slide->rootObject();
    if (root)
        for (unsigned int i = 0; i < root->objectCount(); i++) {
            Object* object = root->object(i);
            if (object)
                processObjectForStyle(object, styles); 
        }
}

void PowerPointImport::processObjectForStyle(Object* object, KoGenStyles &styles)
{
    if (!object) return;

    if (object->isText())
        processTextObjectForStyle(static_cast<TextObject*>(object), styles);
    else if (object->isGroup())
        processGroupObjectForStyle(static_cast<GroupObject*>(object), styles);
    else if (object->isDrawing())
        processDrawingObjectForStyle(static_cast<DrawObject*>(object), styles);
}

QString PowerPointImport::paraSpacingToCm(int value) const
{
    if (value < 0) {
        unsigned int temp = -value;
        return pptMasterUnitToCm(temp);
    }

    return pptMasterUnitToCm(value);
}

QString PowerPointImport::pptMasterUnitToCm(unsigned int value) const
{
    qreal result = value;
    result *= 2.54;
    result /= 576;
    return QString("%1cm").arg(result);
}

QString PowerPointImport::textAlignmentToString(unsigned int value) const
{
    switch (value) {
        /**
        Tx_ALIGNLeft            0x0000 For horizontal text, left aligned.
                                   For vertical text, top aligned.
        */
    case 0:
        return "left";
        /**
        Tx_ALIGNCenter          0x0001 For horizontal text, centered.
                                   For vertical text, middle aligned.
        */
    case 1:
        return "center";
        /**
        Tx_ALIGNRight           0x0002 For horizontal text, right aligned.
                                   For vertical text, bottom aligned.
        */
    case 2:
        return "right";

        /**
        Tx_ALIGNJustify         0x0003 For horizontal text, flush left and right.
                                   For vertical text, flush top and bottom.
        */
        return "justify";

        //TODO these were missing from ODF specification v1.1, but are
        //in [MS-PPT].pdf

        /**
        Tx_ALIGNDistributed     0x0004 Distribute space between characters.
        */
    case 4:

        /**
        Tx_ALIGNThaiDistributed 0x0005 Thai distribution justification.
        */
    case 5:

        /**
        Tx_ALIGNJustifyLow      0x0006 Kashida justify low.
        */
    case 6:
        return "";

        //TODO these two are in ODF specification v1.1 but are missing from
        //[MS-PPT].pdf
        //return "end";
        //return "start";
    }

    return QString();
}


QColor PowerPointImport::colorIndexStructToQColor(const ColorIndexStruct &color)
{
    if (color.index() == 0xFE) {
        return QColor(color.red(), color.green(), color.blue());
    }

    MainMasterContainer *mainMaster = d->presentation->getMainMasterContainer();
    if (mainMaster) {
        return mainMaster->getSlideSchemeColorSchemeAtom()->getColor(color.index());
    }

    kError() << "failed to get main master!";

    return QColor();
}

TextPFException *PowerPointImport::masterTextPFException(int type, unsigned int level)
{
    MainMasterContainer *mainMaster = d->presentation->getMainMasterContainer();
    if (!mainMaster) {
        return 0;
    }

    TextMasterStyleAtom *style = mainMaster->textMasterStyleAtomForTextType(type);

    if (!style) {
        return 0;
    }

    TextMasterStyleLevel *styleLevel = style->level(level);
    if (!styleLevel) {
        return 0;
    }

    return styleLevel->pf();
}

TextCFException *PowerPointImport::masterTextCFException(int type, unsigned int level)
{
    MainMasterContainer *mainMaster = d->presentation->getMainMasterContainer();
    if (!mainMaster) {
        return 0;
    }

    TextMasterStyleAtom *style = mainMaster->textMasterStyleAtomForTextType(type);

    if (!style) {
        return 0;
    }

    TextMasterStyleLevel *styleLevel = style->level(level);
    if (!styleLevel) {
        return 0;
    }

    return styleLevel->cf();
}


void PowerPointImport::processTextExceptionsForStyle(TextCFRun *cf,
        TextPFRun *pf,
        KoGenStyles &styles,
        TextObject* textObject)
{
    if (!textObject) {
        return;
    }

    int indentLevel = 0;
    int indent = 0;
    if (pf) {
        indentLevel = pf->indentLevel();
        indent = pf->textPFException()->indent();
    }

    int type = textObject->type();

    /**
    TODO I had some ppt files where the text headers of slide body's text
    where defined as Tx_TYPE_CENTERBODY and title as Tx_TYPE_CENTERTITLE
    but their true style definitions (when compared to MS Powerpoint)
    where in Tx_TYPE_BODY and Tx_TYPE_TITLE TextMasterStyleAtoms.
    Either the text type is loaded incorrectly or there is some logic behind
    this that needs to be figured out.
    */
    if (type == 5) {
        //Replace Tx_TYPE_CENTERBODY with Tx_TYPE_BODY
        type = 1;
    } else if (type == 6) {
        //and Tx_TYPE_CENTERTITLE with Tx_TYPE_TITLE
        type = 0;
    }

    //Master character/paragraph styles are fetched for the textobjects type
    //using paragraph's indentation level
    TextCFException *masterCF = masterTextCFException(type,
                                indentLevel);

    TextPFException *masterPF = masterTextPFException(type,
                                indentLevel);

    //As mentioned in previous TODO we'll check if the text types
    //were placeholder types (2.13.33 TextTypeEnum in [MS-PPT].pdf
    //and use them for some styles
    TextCFException *placeholderCF = 0;
    TextPFException *placeholderPF = 0;

    if (textObject->type() == 5 || textObject->type() == 6) {
        placeholderPF = masterTextPFException(textObject->type(),
                                              indentLevel);

        placeholderCF = masterTextCFException(textObject->type(),
                                              indentLevel);
    }



    KoGenStyle styleParagraph(KoGenStyle::StyleAuto, "paragraph");
    if (pf && pf->textPFException()->hasLeftMargin()) {
        styleParagraph.addProperty("fo:margin-left",
                paraSpacingToCm(pf->textPFException()->leftMargin()),
                KoGenStyle::ParagraphType);
    } else {
        if (masterPF && masterPF->hasLeftMargin()) {
            styleParagraph.addProperty("fo:margin-left",
                    paraSpacingToCm(masterPF->leftMargin()),
                    KoGenStyle::ParagraphType);
        }
    }

    if (pf && pf->textPFException()->hasSpaceBefore()) {
        styleParagraph.addProperty("fo:margin-top",
                paraSpacingToCm(pf->textPFException()->spaceBefore()),
                KoGenStyle::ParagraphType);
    } else {
        if (masterPF && masterPF->hasSpaceBefore()) {
            styleParagraph.addProperty("fo:margin-top",
                    paraSpacingToCm(masterPF->spaceBefore()),
                    KoGenStyle::ParagraphType);
        }
    }

    if (pf && pf->textPFException()->hasSpaceAfter()) {
        styleParagraph.addProperty("fo:margin-bottom",
                paraSpacingToCm(pf->textPFException()->spaceAfter()),
                KoGenStyle::ParagraphType);
    } else {
        if (masterPF && masterPF->hasSpaceAfter()) {
            styleParagraph.addProperty("fo:margin-bottom",
                    paraSpacingToCm(masterPF->spaceAfter()),
                    KoGenStyle::ParagraphType);
        }
    }

    if (pf && pf->textPFException()->hasIndent()) {
        styleParagraph.addProperty("fo:text-indent",
                pptMasterUnitToCm(pf->textPFException()->indent()),
                KoGenStyle::ParagraphType);
    } else {
        if (masterPF && masterPF->hasIndent()) {
            styleParagraph.addProperty("fo:text-indent",
                    pptMasterUnitToCm(masterPF->indent()),
                    KoGenStyle::ParagraphType);
        }
    }


    /**
    TODO When previous TODO about Tx_TYPE_CENTERBODY and Tx_TYPE_CENTERTITLE
    is fixed, correct the logic here. Here is added an extra if to use the
    placeholderPF which in turn contained the correct value.
    */
    if (pf && pf->textPFException()->hasAlign()) {
        styleParagraph.addProperty("fo:text-align",
                                   textAlignmentToString(pf->textPFException()->textAlignment()),
                                   KoGenStyle::ParagraphType);
    } else if (placeholderPF && placeholderPF->hasAlign()) {
        styleParagraph.addProperty("fo:text-align",
                                   textAlignmentToString(placeholderPF->textAlignment()),
                                   KoGenStyle::ParagraphType);
    } else {
        if (masterPF && masterPF->hasAlign()) {
            styleParagraph.addProperty("fo:text-align",
                                       textAlignmentToString(masterPF->textAlignment()),
                                       KoGenStyle::ParagraphType);
        }
    }

    //Text style
    KoGenStyle styleText(KoGenStyle::StyleTextAuto, "text");
    if (cf && cf->textCFException()->hasColor()) {
        styleText.addProperty("fo:color",
                              colorIndexStructToQColor(cf->textCFException()->color()).name(),
                              KoGenStyle::TextType);
    } else {
        //Make sure that character formatting has color aswell
        if (masterCF && masterCF->hasColor()) {
            styleText.addProperty("fo:color",
                                  colorIndexStructToQColor(masterCF->color()).name(),
                                  KoGenStyle::TextType);
        }
    }

    if (cf && cf->textCFException()->hasFontSize()) {
        styleText.addProperty("fo:font-size",
                              QString("%1pt").arg(cf->textCFException()->fontSize()),
                              KoGenStyle::TextType);
    } else {
        if (masterCF && masterCF->hasFontSize()) {
            styleText.addProperty("fo:font-size",
                                  QString("%1pt").arg(masterCF->fontSize()),
                                  KoGenStyle::TextType);
        }
    }

    if (cf && cf->textCFException()->hasItalic()) {
        if (cf->textCFException()->italic()) {
            styleText.addProperty("fo:font-style",
                                  "italic",
                                  KoGenStyle::TextType);
        }
    } else {
        if (masterCF && masterCF->hasItalic() && masterCF->italic()) {
            styleText.addProperty("fo:font-style",
                                  "italic",
                                  KoGenStyle::TextType);
        }
    }

    if (cf && cf->textCFException()->hasBold()) {
        if (cf->textCFException()->bold()) {
            styleText.addProperty("fo:font-weight",
                                  "bold",
                                  KoGenStyle::TextType);
        }
    } else {
        if (masterCF && masterCF->hasBold() && masterCF->bold()) {
            styleText.addProperty("fo:font-weight",
                                  "bold",
                                  KoGenStyle::TextType);
        }
    }

    TextFont *font = 0;
    if (cf && cf->textCFException()->hasFont()) {
        font = d->presentation->getFont(cf->textCFException()->fontRef());
    } else {
        if (masterCF && masterCF->hasFont()) {
            font = d->presentation->getFont(masterCF->fontRef());
        }
    }

    if (font) {
        styleText.addProperty("fo:font-family", font->name());
        font = 0;
    }

    if (cf && cf->textCFException()->hasFontSize()) {
        styleText.addProperty("fo:font-size",
                              QString("%1pt").arg(cf->textCFException()->fontSize()),
                              KoGenStyle::TextType);

    } else {
        if (masterCF && masterCF->hasFontSize()) {
            styleText.addProperty("fo:font-size",
                                  QString("%1pt").arg(masterCF->fontSize()),
                                  KoGenStyle::TextType);
        }
    }

    if (cf && cf->textCFException()->hasPosition()) {
        styleText.addProperty("style:text-position",
                              QString("%1%").arg(cf->textCFException()->position()),
                              KoGenStyle::TextType);

    } else {
        if (masterCF && masterCF->hasPosition()) {
            styleText.addProperty("style:text-position",
                                  QString("%1%").arg(masterCF->position()),
                                  KoGenStyle::TextType);
        }
    }

    bool underline = false;
    if (cf && cf->textCFException()->hasUnderline()) {
        underline = cf->textCFException()->underline();
    } else {
        if (masterCF && masterCF->hasUnderline()) {
            underline = masterCF->underline();
        }
    }

    if (underline) {
        styleText.addProperty("style:text-underline-style",
                              "solid",
                              KoGenStyle::TextType);

        styleText.addProperty("style:text-underline-width",
                              "auto",
                              KoGenStyle::TextType);

        styleText.addProperty("style:text-underline-color",
                              "font-color",
                              KoGenStyle::TextType);
    }

    bool emboss = false;
    if (cf && cf->textCFException()->hasEmboss()) {
        emboss = cf->textCFException()->emboss();
    } else {
        if (masterCF && masterCF->hasEmboss()) {
            emboss = masterCF->emboss();
        }
    }

    if (emboss) {
        styleText.addProperty("style:font-relief",
                              "embossed",
                              KoGenStyle::TextType);
    }

    bool shadow = false;
    if (cf && cf->textCFException()->hasShadow()) {
        shadow = cf->textCFException()->shadow();
    } else {
        if (masterCF && masterCF->hasShadow()) {
            shadow = masterCF->shadow();
        }
    }

    if (shadow) {
        styleText.addProperty("fo:text-shadow", 0, KoGenStyle::TextType);
    }

    KoGenStyle styleList(KoGenStyle::StyleListAuto, 0);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level


    TextPFException9 *pf9 = 0;
    StyleTextProp9 *prop9 = 0;

    if (cf) {
        prop9 = textObject->findStyleTextProp9(cf->textCFException());
    }

    for (int i = 0;i < indent + 1;i++) {
        //TextCFException *levelCF = masterTextCFException(type, i);
        TextPFException *levelPF = masterTextPFException(type, i);

        if (prop9) {
            pf9 = prop9->pf9();
            std::cout << "\npf9 :" << pf9;
        }

        bool isListLevelStyleNumber =
                (levelPF && (levelPF->hasBulletFont() || levelPF->bulletFont()))
                || (pf9 && pf9->bulletHasScheme() && pf9->bulletScheme());
        if (isListLevelStyleNumber) {
            elementWriter.startElement("text:list-level-style-number");
        } else {
            elementWriter.startElement("text:list-level-style-bullet");
            // every text:list-level-style-bullet must have a text:bullet-char
            const char bullet[4] = {0xe2, 0x97, 0x8f, 0};
            QString bulletChar(QString::fromUtf8(bullet)); //  "●";
            if (pf && i == pf->textPFException()->indent() &&
                    pf->textPFException()->hasBulletChar()) {
                bulletChar = pf->textPFException()->bulletChar();
            } else if (!isListLevelStyleNumber
                    && levelPF && levelPF->hasBulletChar()) {
                bulletChar = levelPF->bulletChar();
            } else {
                QString::fromUtf8(bullet);//  "●";
            }
            elementWriter.addAttribute("text:bullet-char", bulletChar);
        }

        elementWriter.addAttribute("text:level", i + 1);

        if (pf9 && pf9->bulletHasScheme() && pf9->bulletScheme()) {
            QString numFormat = 0;
            QString numSuffix = 0;
            QString numPrefix = 0;
            processTextAutoNumberScheme(pf9->scheme(), numFormat, numSuffix,
                numPrefix);

            if (numPrefix != 0) {
                elementWriter.addAttribute("style:num-prefix", numPrefix);
            }

            if (numSuffix != 0) {
                elementWriter.addAttribute("style:num-suffix", numSuffix);
            }

            elementWriter.addAttribute("style:num-format", numFormat);
        }

        elementWriter.startElement("style:list-level-properties");

        if (pf && i == pf->textPFException()->indent() &&
                pf->textPFException()->hasSpaceBefore()) {
            elementWriter.addAttribute("text:space-before",
                    paraSpacingToCm(pf->textPFException()->spaceBefore()));
        } else if (levelPF && levelPF->hasSpaceBefore()) {
            elementWriter.addAttribute("text:space-before",
                    paraSpacingToCm(levelPF->spaceBefore()));
        }
        elementWriter.endElement(); // style:list-level-properties

        elementWriter.startElement("style:text-properties");

        if (pf && i == pf->textPFException()->indent() &&
                pf->textPFException()->hasBulletFont()) {
            font = d->presentation->getFont(pf->textPFException()->bulletFontRef());
        } else if (levelPF && levelPF->hasBulletFont()) {
            font = d->presentation->getFont(levelPF->bulletFontRef());
        }

        if (font) {
            elementWriter.addAttribute("fo:font-family", font->name());
        }

        if (pf && i == pf->textPFException()->indent() &&
                pf->textPFException()->hasBulletColor()) {
            elementWriter.addAttribute("fo:color",
                                       colorIndexStructToQColor(pf->textPFException()->bulletColor()).name());
        } else {
            if (levelPF && levelPF->hasBulletColor()) {
                elementWriter.addAttribute("fo:color",
                                           colorIndexStructToQColor(levelPF->bulletColor()).name());
            }
        }

        if (pf && i == pf->textPFException()->indent() &&
                pf->textPFException()->hasBulletSize()) {
            elementWriter.addAttribute("fo:font-size",
                                       QString("%1%").arg(pf->textPFException()->bulletSize()));

        } else {
            if (levelPF && levelPF->hasBulletSize()) {
                elementWriter.addAttribute("fo:font-size",
                                           QString("%1%").arg(levelPF->bulletSize()));
            } else {
                elementWriter.addAttribute("fo:font-size", "100%");
            }
        }

        elementWriter.endElement(); // style:text-properties

        elementWriter.endElement();  // text:list-level-style-bullet


        styleList.addChildElement("text:list-level-style-bullet",
                                  QString::fromUtf8(buffer.buffer(),
                                                    buffer.buffer().size()));
    }

    if (pf && cf) {
        textObject->addStylenames(cf->textCFException(),
                                  pf->textPFException(),
                                  styles.lookup(styleText),
                                  styles.lookup(styleParagraph),
                                  styles.lookup(styleList));
    } else {
        textObject->addStylenames(0,
                                  0,
                                  styles.lookup(styleText),
                                  styles.lookup(styleParagraph),
                                  styles.lookup(styleList));
    }

}

void PowerPointImport::processTextAutoNumberScheme(int val, QString& numFormat, QString& numSuffix, QString& numPrefix)
{
    switch (val) {

    case ANM_AlphaLcPeriod:         //Example: a., b., c., ...Lowercase Latin character followed by a period.
        numFormat = "a";
        numSuffix = ".";
        break;

    case ANM_AlphaUcPeriod:        //Example: A., B., C., ...Uppercase Latin character followed by a period.
        numFormat = "A";
        numSuffix = ".";
        break;

    case ANM_ArabicParenRight:     //Example: 1), 2), 3), ...Arabic numeral followed by a closing parenthesis.
        numFormat = "1";
        numSuffix = ")";
        break;

    case ANM_ArabicPeriod :        //Example: 1., 2., 3., ...Arabic numeral followed by a period.
        numFormat = "1";
        numSuffix = ".";
        break;

    case ANM_RomanLcParenBoth:     //Example: (i), (ii), (iii), ...Lowercase Roman numeral enclosed in parentheses.
        numPrefix = "(";
        numFormat = "i";
        numSuffix = ")";
        break;

    case ANM_RomanLcParenRight:    //Example: i), ii), iii), ... Lowercase Roman numeral followed by a closing parenthesis.
        numFormat = "i";
        numSuffix = ")";
        break;

    case ANM_RomanLcPeriod :        //Example: i., ii., iii., ...Lowercase Roman numeral followed by a period.
        numFormat = "i";
        numSuffix = ".";
        break;

    case ANM_RomanUcPeriod:         //Example: I., II., III., ...Uppercase Roman numeral followed by a period.
        numFormat = "I";
        numSuffix = ".";
        break;

    case ANM_AlphaLcParenBoth:      //Example: (a), (b), (c), ...Lowercase alphabetic character enclosed in parentheses.
        numPrefix = "(";
        numFormat = "a";
        numSuffix = ")";
        break;

    case ANM_AlphaLcParenRight:     //Example: a), b), c), ...Lowercase alphabetic character followed by a closing
        numFormat = "a";
        numSuffix = ")";
        break;

    case ANM_AlphaUcParenBoth:      //Example: (A), (B), (C), ...Uppercase alphabetic character enclosed in parentheses.
        numPrefix = "(";
        numFormat = "A";
        numSuffix = ")";
        break;

    case ANM_AlphaUcParenRight:     //Example: A), B), C), ...Uppercase alphabetic character followed by a closing
        numFormat = "A";
        numSuffix = ")";
        break;

    case ANM_ArabicParenBoth:       //Example: (1), (2), (3), ...Arabic numeral enclosed in parentheses.
        numPrefix = "(";
        numFormat = "1";
        numSuffix = ")";
        break;

    case ANM_ArabicPlain:           //Example: 1, 2, 3, ...Arabic numeral.
        numFormat = "1";
        break;

    case ANM_RomanUcParenBoth:      //Example: (I), (II), (III), ...Uppercase Roman numeral enclosed in parentheses.
        numPrefix = "(";
        numFormat = "I";
        numSuffix = ")";
        break;

    case ANM_RomanUcParenRight:     //Example: I), II), III), ...Uppercase Roman numeral followed by a closing parenthesis.
        numFormat = "I";
        numSuffix = ")";
        break;

    default:
        numFormat = "i";
        numSuffix = ".";
        break;
    }
}

QString hexname(const Color &c)
{
    QColor qc(c.red, c.green, c.blue);
    return qc.name();
}

void processGraphicStyles(KoGenStyle& style, Object* object)
{
    if (object->hasProperty("fillPib")) {
        style.addProperty("draw:fill", "bitmap");
        int pib = object->getIntProperty("fillPib");
        style.addProperty("draw:fill-image-name",
                "fillImage" + QString::number(pib));
    } else if (object->hasProperty("draw:fill")) {
        std::string s = object->getStrProperty("draw:fill");
        QString ss(s.c_str());
        style.addProperty("draw:fill", ss, KoGenStyle::GraphicType);
    }

    if (object->hasProperty("draw:fill-color")) {
        Color fillColor = object->getColorProperty("draw:fill-color");
        style.addProperty("draw:fill-color", hexname(fillColor),
                KoGenStyle::GraphicType);
    } else {
        style.addProperty("draw:fill-color", "#99ccff",
                KoGenStyle::GraphicType);
    }
}

void PowerPointImport::processTextObjectForStyle(TextObject* textObject,
        KoGenStyles &styles)
{
    if (!textObject) {
        return;
    }

    StyleTextPropAtom* atom  = textObject->styleTextProperty();
    if (!atom) {
        processTextExceptionsForStyle(0, 0, styles, textObject);
        return;
    }

    //What paragraph/character exceptions were used last
    TextPFRun *pf = 0;
    TextCFRun *cf = 0;

    //TODO this can be easily optimized by calculating proper increments to i
    //from both exception's character count
    for (int i = 0;i < textObject->text().length();i++) {
        if (cf == atom->findTextCFRun(i) && pf == atom->findTextPFRun(i) && i > 0) {
            continue;
        }

        pf = atom->findTextPFRun(i);
        cf = atom->findTextCFRun(i);

        processTextExceptionsForStyle(cf, pf, styles, textObject);
    }

    // set the presentation style
    QString styleName;
    KoGenStyle style(KoGenStyle::StyleGraphicAuto, "presentation");
    processGraphicStyles(style, textObject);
    styleName = styles.lookup(style);
    textObject->setGraphicStyleName(styleName);
}

void PowerPointImport::processGroupObjectForStyle(GroupObject* groupObject,
        KoGenStyles &styles)
{
    if (!groupObject) return;

    for (unsigned int i = 0; i < groupObject->objectCount(); ++i) {
        processObjectForStyle(groupObject->object(i), styles);
    }
}

void PowerPointImport::processDrawingObjectForStyle(DrawObject* drawObject, KoGenStyles &styles)
{
    if (!drawObject) return;

    KoGenStyle style(KoGenStyle::StyleGraphicAuto, "graphic");
    style.setParentName("standard");

    if (drawObject->hasProperty("libppt:invisibleLine")) {
        if (drawObject->getBoolProperty("libppt:invisibleLine") == true)
            style.addProperty("draw:stroke", "none", KoGenStyle::GraphicType);
    } else if (drawObject->hasProperty("draw:stroke")) {
        if (drawObject->getStrProperty("draw:stroke") == "dash") {
            style.addProperty("draw:stroke", "dash", KoGenStyle::GraphicType);
            std::string s = drawObject->getStrProperty("draw:stroke-dash");
            QString ss(s.c_str());
            style.addProperty("draw:stroke-dash", ss, KoGenStyle::GraphicType);
        } else if (drawObject->getStrProperty("draw:stroke") == "solid") {
            style.addProperty("draw:stroke", "solid", KoGenStyle::GraphicType);
        }
    } else {
        // default style is a solid line, this must be set explicitly as long
        // as kpresenter takes draw:stroke="none" as default
        style.addProperty("draw:stroke", "solid", KoGenStyle::GraphicType);
    }

    if (drawObject->hasProperty("svg:stroke-width")) {
        double strokeWidth = drawObject->getDoubleProperty("svg:stroke-width");
        style.addProperty("svg:stroke-width", QString("%1mm").arg(strokeWidth), KoGenStyle::GraphicType);
    }

    if (drawObject->hasProperty("svg:stroke-color")) {
        Color strokeColor = drawObject->getColorProperty("svg:stroke-color");
        style.addProperty("svg:stroke-color", hexname(strokeColor), KoGenStyle::GraphicType);
    }

    if (drawObject->hasProperty("draw:marker-start")) {
        std::string s = drawObject->getStrProperty("draw:marker-start");
        QString ss(s.c_str());
        style.addProperty("draw:marker-start", ss, KoGenStyle::GraphicType);
    }
    if (drawObject->hasProperty("draw:marker-end")) {
        std::string s = drawObject->getStrProperty("draw:marker-end");
        QString ss(s.c_str());
        style.addProperty("draw:marker-end", ss, KoGenStyle::GraphicType);
    }
    if (drawObject->hasProperty("draw:marker-start-width")) {
        double strokeWidth = drawObject->getDoubleProperty("svg:stroke-width");
        double arrowWidth = (drawObject->getDoubleProperty("draw:marker-start-width") * strokeWidth);
        style.addProperty("draw:marker-start-width", QString("%1cm").arg(arrowWidth), KoGenStyle::GraphicType);
    }

    if (drawObject->hasProperty("draw:marker-end-width")) {
        double strokeWidth = drawObject->getDoubleProperty("svg:stroke-width");
        double arrowWidth = (drawObject->getDoubleProperty("draw:marker-end-width") * strokeWidth);
        style.addProperty("draw:marker-end-width", QString("%1cm").arg(arrowWidth), KoGenStyle::GraphicType);
    }

    processGraphicStyles(style, drawObject);

#if 0
    if (drawObject->hasProperty("draw:shadow-color")) {
        elementWriter.addAttribute("draw:shadow", "visible");
        Color shadowColor = drawObject->getColorProperty("draw:shadow-color");
        style.addProperty("draw:shadow-color", hexname(shadowColor), KoGenStyle::GraphicType);
    } else {
        style.addProperty("draw:shadow", "hidden", KoGenStyle::GraphicType);
    }
#endif

    if (drawObject->hasProperty("draw:shadow-opacity")) {
        double opacity = drawObject->getDoubleProperty("draw:shadow-opacity") ;
        style.addProperty("draw:shadow-opacity", QString("%1%").arg(opacity), KoGenStyle::GraphicType);
    }

    if (drawObject->hasProperty("draw:shadow-offset-x")) {
        double offset = drawObject->getDoubleProperty("draw:shadow-offset-x") ;
        style.addProperty("draw:shadow-offset-x", QString("%1cm").arg(offset), KoGenStyle::GraphicType);
    }

    if (drawObject->hasProperty("draw:shadow-offset-y")) {
        double offset = drawObject->getDoubleProperty("draw:shadow-offset-y");
        style.addProperty("draw:shadow-offset-y", QString("%1cm").arg(offset), KoGenStyle::GraphicType);
    }

    drawObject->setGraphicStyleName(styles.lookup(style));
}   
