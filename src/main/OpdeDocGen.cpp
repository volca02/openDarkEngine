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

#include "config.h"

#include "OpdeException.h"
#include "Root.h"
#include "StringTokenizer.h"
#include "ConfigService.h"

#include <OgreException.h>

// Simple and very dirty doc generator.
// outputs to latex so it can be converted to html using latex2html or rendered to pdf, etc.
using namespace Opde;
using namespace std;

namespace Opde {
	/// Documentation generator for properties and links
    class DocGenerator {
		protected:
			typedef enum {
				DT_PROP,
				DT_LINK,
				DT_ENUM,
				DT_SPECIAL
			} DocumentStringType;

			int	mArgc;
			char** mArgv;
			std::string mConfigFile;
			Opde::Root* mRoot;
			ConfigServicePtr mConfigSvc;

			typedef std::map<std::string, std::string> DocStrings;

			DocStrings mPropertyDocs;
			DocStrings mLinkDocs;
			DocStrings mEnumDocs;
			DocStrings mSpecialDocs;

			typedef std::map<std::string, DEnum*> EnumMap;

			EnumMap mEncounteredEnums;

			void queueEnumForDesc(DEnum* en) {
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

				DVariant p;

				if (!mConfigSvc->getParam("resources", p)) {
					// default
					p = "resources.cfg";
				}

				mRoot->loadResourceConfig(p.toString());
				mRoot->bootstrapFinished();
			}

			void dispatchAdditionalDocString(DocumentStringType type, const string& addr, const string& txt) {
				if (addr != "") {
					switch (type) {
						case DT_PROP:
							LOG_DEBUG("Dispatched prop desc for '%s': %s", addr.c_str(), txt.c_str());
							mPropertyDocs[addr] = txt;
							break;
						case DT_LINK:
							LOG_DEBUG("Dispatched link desc for '%s': %s", addr.c_str(), txt.c_str());
							mLinkDocs[addr] = txt;
							break;
                        case DT_ENUM:
							LOG_DEBUG("Dispatched enum desc for '%s': %s", addr.c_str(), txt.c_str());
							mEnumDocs[addr] = txt;
							break;
						case DT_SPECIAL:
							LOG_DEBUG("Dispatched special doc page '%s': %s", addr.c_str(), txt.c_str());
							mSpecialDocs[addr] = txt;
							break;
					}
				} else {
					LOG_DEBUG("Empty address for accumulated doc %s", txt.c_str());
				}
			}

			void loadAdditionalDocStrings(void) {
				// see if we can open the specified file
				DVariant fn = "docstrings.txt"; // default

				// override, if present
				mConfigSvc->getParam("docstrings", fn);

				std::fstream s;

				s.open(fn.toString().c_str(), ios::in);

				if (s.fail())
					return;

				string target = "";
				DocumentStringType doctype = DT_PROP;
				string text = "";

				// process line by line
				while (!s.eof()) {
					string line;

					getline(s, line, '\n');

					// process
					// strip comments
					size_t cpos = line.find_first_of("#");

					if (cpos != line.npos)
						line = line.substr(0, --cpos);

					// we now have the comment stripped. See if we have a target specifier

					size_t cl = line.find_first_of("]");

					if (line[0] == '[' && cl != line.npos) {
						// yup. see the type
						string cont = line.substr(1, cl - 1);

						WhitespaceStringTokenizer tok(cont);

						string type = tok.next();
						string address = tok.next();

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
							OPDE_EXCEPT("Unknown section type :" + type, "DocGenerator::loadAdditionalDocStrings");
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

			void genPropDocs(fstream& fo) {
				PropertyServicePtr ps = GET_SERVICE(PropertyService);

				StringIteratorPtr pn = ps->getAllPropertyNames();

				fo << "\\chapter{Properties}" << endl;

				while (!pn->end()) {
					const string& propname = pn->next();

					std::cerr << "Prop " << propname << std::endl;

					fo << "\\section*{" << propname << "}" << endl;

					// for cross linking
					fo << "\\label{prop_" << propname << "}" << endl;

					// standard info header. Chunk versions, data size, label
					// Get the prop group
					PropertyGroup* pg = ps->getPropertyGroup(propname);

					fo << "\\textbf{Property}: " << propname << endl << endl; // TODO: Path

					uint maj, min;
					maj = pg->getChunkVersionMajor();
					min = pg->getChunkVersionMinor();

					// get the info
					fo << "\\textbf{Chunk version:} " << maj << "." << min << endl;

					if (pg->getBuiltin())
						fo << "Built-In" << endl;


					DocStrings::iterator it = mPropertyDocs.find(propname);
					if (it != mPropertyDocs.end()) {
						fo << endl << "\\subsection*{Description}" << endl;

						// spit out the additional docs if present
						fo << it->second << endl;
					}

					fo << endl << "\\subsection*{Fields}" << endl;
					fo << "\\begin{tabular}{lcrrr}" << endl;
					// table contents, each line ending with \\, cells separated using &
					fo << "\\textbf{Field} & \\textbf{Type} & \\textbf{Size} & \\textbf{Enum} & \\textbf{Description} \\\\" << endl;

					// TODO: spit all the property fields with descriptions
					DataFieldDescIteratorPtr dfi = pg->getFieldDescIterator();

					// iterate
					while(!dfi->end()) {
					    const DataFieldDesc& df = dfi->next();

					    fo << df.name << " & " <<
                              DVariant::typeToString(df.type) << " & " <<
                              df.size << " & ";

                        if (df.enumerator != NULL) {
                            fo << df.enumerator->getName() << " \\pageref{enum_"<< df.enumerator->getName() <<"}";
                            queueEnumForDesc(df.enumerator);
                        } else {
                            fo << "-";
                        };

                        fo << " &  \\\\" << std::endl;
					}

					fo << "\\end{tabular}" << endl;

					fo << endl;
				}
			}

			void genLinkDocs(fstream& fo) {
				LinkServicePtr ls = GET_SERVICE(LinkService);

				StringIteratorPtr ln = ls->getAllLinkNames();

				fo << "\\chapter{Links}" << endl;

				while (!ln->end()) {
					const string& linkname = ln->next();

					fo << "\\section*{" << linkname << "}" << endl;

					// for cross linking
					fo << "\\label{prop_" << linkname << "}" << endl;

					DocStrings::iterator it = mLinkDocs.find(linkname);
					if (it != mLinkDocs.end()) {
						fo << endl << "\\subsection*{Description}" << endl;

						// spit out the additional docs if present
						fo << it->second << endl;
					}

					fo << "\\end{tabular}" << endl;

					fo << endl;
				}
			}

			void genEnumDocs(fstream& fo) {
				fo << "\\chapter{Enumerations}" << endl;

				EnumMap::iterator it = mEncounteredEnums.begin();

				for (; it != mEncounteredEnums.end(); ++it) {
					DEnum* en = it->second;

					const string& enname = en->getName();

					fo << "\\section*{" << enname << "}" << endl;

					// for cross linking
					fo << "\\label{enum_" << enname << "}" << endl;

					fo << "\\textbf{Type:}";

					if (en->isBitfield())
                        fo << "Bitfield" << endl << endl; // TODO: Path
					else
                        fo << "Enumeration" <<  endl << endl; // TODO: Path

					DocStrings::iterator it = mEnumDocs.find(enname);

					if (it != mEnumDocs.end()) {
						fo << endl << "\\subsection*{Description}" << endl;

						// spit out the additional docs if present
						fo << it->second << endl;
					}

					fo << endl << "\\subsection*{Structure}" << endl;
					fo << "\\begin{tabular}{lr}" << endl;
					// table contents, each line ending with \\, cells separated using &
					fo << "\\textbf{Key} & \\textbf{Value} \\\\" << endl;

					DEnum::EnumFieldList l = en->getFieldList(0);

					DEnum::EnumFieldList::iterator kit = l.begin();

					// iterate
					for (; kit != l.end(); ++kit) {
					    DEnum::EnumField& f = *kit;

                        fo << f.key << " & " <<
                            f.value.toUInt() << " \\\\" << std::endl;
					}

					fo << "\\end{tabular}" << endl;

					fo << endl;
				}
			}

			void doGenerate(void) {
				DVariant fn = "output.tex"; // default

				// override, if present
				mConfigSvc->getParam("outfile", fn);

				std::fstream s;

				s.open(fn.toString().c_str(), ios::out);

				// if we have specialdoc header, output it
				s << mSpecialDocs["header"];

				// first iterate over the properties
				genPropDocs(s);

				// links
				genLinkDocs(s);

				// enumerations
				genEnumDocs(s);

				s<<"\\end{document}" << endl;
				s.close();
			}

		public:
			DocGenerator(int argc, char**argv) : mArgc(argc), mArgv(argv) {
				mRoot = new Opde::Root(SERVICE_CORE);
				// we want script autoload
				mRoot->registerCustomScriptLoaders();
				mConfigSvc = GET_SERVICE(ConfigService);
			}

			~DocGenerator(void) {
				mConfigSvc.setNull();
				delete mRoot;
			}

			void help(void) {
				cout << "Usage: opdeDocGen config_file" << endl;
			}


			bool generateDoc(void) {
				if (!parseCommandline()) {
					help();
					return false;
				}

				// TODO: debug config param.
				mRoot->setLogLevel(5);
				// TODO: logfile config param.
				mRoot->logToFile("opdeDocGen.log");

				// initialize the system - load the dtype scripts according to the config
				initialize();

				// read the additional docs file. This file contains docs for each entity
				// has a simple format [prop PropName] or [link LinkName] for doc target change. Other lines (after stripping # comments)
				// are accumulated to the last specified doc target
				loadAdditionalDocStrings();

				// generate the doc
				doGenerate();

				return true;
			}
    };
};

int main(int argc, char**argv) {
    int rv = 0;

    try {
		DocGenerator dg(argc, argv);

		if (!dg.generateDoc())
			rv = 3; // exit code 3 - parsing failed
    } catch( Ogre::Exception& e ) {
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
	rv = 1; // exit code 2 - ogre error
    } catch( BasicException& e ) {
        std::cerr << "An exception has occured: " <<
            e.getDetails().c_str() << std::endl;
		rv = 2; // exit code 1 - opde error
    }

    return rv;
}
