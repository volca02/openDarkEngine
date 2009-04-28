import opde

import epydoc.docbuilder
import epydoc.docwriter.html

# TODO: Write directory spec. Templatization with CMake, automatic doc. builds
index = epydoc.docbuilder.build_doc_index([opde, opde.services])
epydoc.docwriter.html.HTMLWriter(index).write("html")
