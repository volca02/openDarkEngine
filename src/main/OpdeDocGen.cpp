/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    $Id$
 *
 *****************************************************************************/

#include <iostream>
#include <OgreException.h>

#include "OpdeException.h"
#include "Root.h"
#include "StringTokenizer.h"
#include "Variant.h"
#include "config/ConfigService.h"
#include "format.h"
#include "link/LinkService.h"
#include "property/PropertyService.h"

// Simple and very dirty doc generator.
// outputs to latex so it can be converted to html using latex2html or rendered
// to pdf, etc.
namespace Opde {
/// Documentation generator for properties and links
class DocGenerator {
protected:
    typedef enum { DT_PROP, DT_LINK, DT_ENUM, DT_SPECIAL } DocumentStringType;

    int mArgc;
    char **mArgv;
    std::string mConfigFile;
    Opde::Root *mRoot;
    ConfigServicePtr mConfigSvc;

    typedef std::map<std::string, std::string> DocStrings;

    DocStrings mPropertyDocs;
    DocStrings mLinkDocs;
    DocStrings mEnumDocs;
    DocStrings mSpecialDocs;

    typedef std::map<std::string, Enumeration *> EnumMap;

    EnumMap mEncounteredEnums;

    void queueEnumForDesc(Enumeration *en) {
        assert(en);

        mEncounteredEnums[en->getName()] = en;
    }

    bool parseCommandline(void) {
        // look for config file name
        if (mArgc < 2)
            return false;

        mConfigFile = mArgv[1];
        return true;
    }

    void initialize(void) {
        // parse the config file. Load the dtype scripts
        // this is done automatically by adding the resource locations
        // then bootstrapping
        // initialize Opde::Root. No service types besides the base
        mRoot->loadConfigFile(mConfigFile);

        Variant p;

        if (!mConfigSvc->getParam("resources", p)) {
            // default
            p = "resources.cfg";
        }

        mRoot->loadResourceConfig(p.toString());
        mRoot->bootstrapFinished();
    }

    void dispatchAdditionalDocString(DocumentStringType type,
                                     const std::string &addr,
                                     const std::string &txt)
    {
        if (addr != "") {
            switch (type) {
            case DT_PROP:
                LOG_DEBUG("Dispatched prop desc for '%s': %s", addr.c_str(),
                          txt.c_str());
                mPropertyDocs[addr] = txt;
                break;
            case DT_LINK:
                LOG_DEBUG("Dispatched link desc for '%s': %s", addr.c_str(),
                          txt.c_str());
                mLinkDocs[addr] = txt;
                break;
            case DT_ENUM:
                LOG_DEBUG("Dispatched enum desc for '%s': %s", addr.c_str(),
                          txt.c_str());
                mEnumDocs[addr] = txt;
                break;
            case DT_SPECIAL:
                LOG_DEBUG("Dispatched special doc page '%s': %s", addr.c_str(),
                          txt.c_str());
                mSpecialDocs[addr] = txt;
                break;
            }
        } else {
            LOG_DEBUG("Empty address for accumulated doc %s", txt.c_str());
        }
    }

    void loadAdditionalDocStrings(void) {
        // see if we can open the specified file
        Variant fn = "docstrings.txt"; // default

        // override, if present
        mConfigSvc->getParam("docstrings", fn);

        std::fstream s;

        s.open(fn.toString().c_str(), std::ios::in);

        if (s.fail())
            return;

        std::string target = "";
        DocumentStringType doctype = DT_PROP;
        std::string text = "";

        // process line by line
        while (!s.eof()) {
            std::string line;

            getline(s, line, '\n');

            // process
            // strip comments
            size_t cpos = line.find_first_of("#");

            if (cpos != line.npos)
                line = line.substr(0, --cpos);

            // we now have the comment stripped. See if we have a target
            // specifier

            size_t cl = line.find_first_of("]");

            if (line[0] == '[' && cl != line.npos) {
                // yup. see the type
                std::string cont = line.substr(1, cl - 1);

                WhitespaceStringTokenizer tok(cont);

                std::string type = tok.next();
                std::string address = tok.next();

                DocumentStringType ntype = DT_PROP;

                if (type == "prop") {
                    ntype = DT_PROP;
                } else if (type == "link") {
                    ntype = DT_LINK;
                } else if (type == "enum") {
                    ntype = DT_ENUM;
                } else if (type == "page") {
                    ntype = DT_SPECIAL;
                } else {
                    OPDE_EXCEPT(format("Unknown section type :", type));
                }

                // dispatch the old text
                dispatchAdditionalDocString(doctype, target, text);

                doctype = ntype;
                text = "";
                target = address;
            } else {
                text += line;
                text += '\n'; // we want to preserve formatting
            }
        } // the cycle

        // last dispatch
        dispatchAdditionalDocString(doctype, target, text);

        s.close();
    }

    void genPropDocs(std::fstream &fo) {
        PropertyServicePtr ps = GET_SERVICE(PropertyService);

        StringIteratorPtr pn = ps->getAllPropertyNames();

        fo << "\\chapter{Properties}" << std::endl;

        while (!pn->end()) {
            const std::string &propname = pn->next();

            std::cerr << "Prop " << propname << std::endl;

            fo << "\\section*{" << propname << "}" << std::endl;

            // for cross linking
            fo << "\\label{prop_" << propname << "}" << std::endl;

            // standard info header. Chunk versions, data size, label
            // Get the prop group
            Property *pg = ps->getProperty(propname);

            fo << "\\textbf{Property}: " << propname << std::endl
               << std::endl; // TODO: Path

            uint maj, min;
            maj = pg->getChunkVersionMajor();
            min = pg->getChunkVersionMinor();

            // get the info
            fo << "\\textbf{Chunk version:} " << maj << "." << min << std::endl;

            if (pg->getBuiltin())
                fo << "Built-In" << std::endl;

            DocStrings::iterator it = mPropertyDocs.find(propname);
            if (it != mPropertyDocs.end()) {
                fo << std::endl << "\\subsection*{Description}" << std::endl;

                // spit out the additional docs if present
                fo << it->second << std::endl;
            }

            fo << std::endl << "\\subsection*{Fields}" << std::endl;
            fo << "\\begin{tabular}{lcrrr}" << std::endl;
            // table contents, each line ending with \\, cells separated using &
            fo << "\\textbf{Field} & \\textbf{Type} & \\textbf{Size} & "
                  "\\textbf{Enum} & \\textbf{Description} \\\\"
               << std::endl;

            // TODO: spit all the property fields with descriptions
            for (const DataFieldDesc &df : pg->getFieldDesc()) {
                fo << df.name << " & " << Variant::typeToString(df.type)
                   << " & " << df.size << " & ";

                if (df.enumerator != NULL) {
                    fo << df.enumerator->getName() << " \\pageref{enum_"
                       << df.enumerator->getName() << "}";
                    queueEnumForDesc(df.enumerator);
                } else {
                    fo << "-";
                };

                fo << " &  \\\\" << std::endl;
            }

            fo << "\\end{tabular}" << std::endl;

            fo << std::endl;
        }
    }

    void genLinkDocs(std::fstream &fo) {
        LinkServicePtr ls = GET_SERVICE(LinkService);

        StringIteratorPtr ln = ls->getAllLinkNames();

        fo << "\\chapter{Links}" << std::endl;

        while (!ln->end()) {
            const std::string &linkname = ln->next();

            fo << "\\section*{" << linkname << "}" << std::endl;

            // for cross linking
            fo << "\\label{prop_" << linkname << "}" << std::endl;

            DocStrings::iterator it = mLinkDocs.find(linkname);
            if (it != mLinkDocs.end()) {
                fo << std::endl << "\\subsection*{Description}" << std::endl;

                // spit out the additional docs if present
                fo << it->second << std::endl;
            }

            fo << "\\end{tabular}" << std::endl;

            fo << std::endl;
        }
    }

    void genEnumDocs(std::fstream &fo) {
        fo << "\\chapter{Enumerations}" << std::endl;

        EnumMap::iterator it = mEncounteredEnums.begin();

        for (; it != mEncounteredEnums.end(); ++it) {
            Enumeration *en = it->second;

            const std::string &enname = en->getName();

            fo << "\\section*{" << enname << "}" << std::endl;

            // for cross linking
            fo << "\\label{enum_" << enname << "}" << std::endl;

            fo << "\\textbf{Type:}";

            if (en->isBitfield())
                fo << "Bitfield" << std::endl << std::endl; // TODO: Path
            else
                fo << "Enumeration" << std::endl << std::endl; // TODO: Path

            DocStrings::iterator it = mEnumDocs.find(enname);

            if (it != mEnumDocs.end()) {
                fo << std::endl << "\\subsection*{Description}" << std::endl;

                // spit out the additional docs if present
                fo << it->second << std::endl;
            }

            fo << std::endl << "\\subsection*{Structure}" << std::endl;
            fo << "\\begin{tabular}{lr}" << std::endl;
            // table contents, each line ending with \\, cells separated using &
            fo << "\\textbf{Key} & \\textbf{Value} \\\\" << std::endl;

            Enumeration::EnumFieldList l = en->getFieldList(0);

            Enumeration::EnumFieldList::iterator kit = l.begin();

            // iterate
            for (; kit != l.end(); ++kit) {
                Enumeration::EnumField &f = *kit;

                fo << f.key << " & " << f.value.toUInt() << " \\\\"
                   << std::endl;
            }

            fo << "\\end{tabular}" << std::endl;

            fo << std::endl;
        }
    }

    void doGenerate(void) {
        Variant fn = "output.tex"; // default

        // override, if present
        mConfigSvc->getParam("outfile", fn);

        std::fstream s;

        s.open(fn.toString().c_str(), std::ios::out);

        // if we have specialdoc header, output it
        s << mSpecialDocs["header"];

        // first iterate over the properties
        genPropDocs(s);

        // links
        genLinkDocs(s);

        // enumerations
        genEnumDocs(s);

        s << "\\end{document}" << std::endl;
        s.close();
    }

public:
    DocGenerator(int argc, char **argv) : mArgc(argc), mArgv(argv) {
        mRoot = new Opde::Root(SERVICE_CORE, "opde-dg.log");
        // we want script autoload
        // mRoot->registerCustomScriptLoaders();
        // TODO: debug config param.
        mRoot->setLogLevel(5);
        mRoot->bootstrapFinished();
        mConfigSvc = GET_SERVICE(ConfigService);
    }

    ~DocGenerator(void) {
        mConfigSvc.reset();
        delete mRoot;
        mRoot = NULL;
    }

    void help(void) {
        std::cout << "Usage: opdeDocGen config_file" << std::endl;
    }

    bool generateDoc(void) {
        if (!parseCommandline()) {
            help();
            return false;
        }
        // initialize the system - load the dtype scripts according to the
        // config
        initialize();

        // read the additional docs file. This file contains docs for each
        // entity has a simple format [prop PropName] or [link LinkName] for doc
        // target change. Other lines (after stripping # comments) are
        // accumulated to the last specified doc target
        loadAdditionalDocStrings();

        // generate the doc
        doGenerate();

        return true;
    }
};
}; // namespace Opde

int main(int argc, char **argv) {
    int rv = 0;

    try {
        Opde::DocGenerator dg(argc, argv);

        if (!dg.generateDoc())
            rv = 3; // exit code 3 - parsing failed
    } catch (Ogre::Exception &e) {
        std::cerr << "An exception has occured: "
                  << e.getFullDescription().c_str() << std::endl;
        rv = 1; // exit code 2 - ogre error
    } catch (Opde::BasicException &e) {
        std::cerr << "An exception has occured: " << e.getDetails().c_str()
                  << std::endl;
        rv = 2; // exit code 1 - opde error
    }

    return rv;
}
