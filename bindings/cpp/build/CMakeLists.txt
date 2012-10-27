INCLUDE_DIRECTORIES( ${APR_INCLUDE}
                     ../../../include
                     ../lucenecpp
                   )

MESSAGE( STATUS ${APR_DIR} )
LINK_DIRECTORIES( ${APR_DIR}/LibR )

IF( WIN32 )
  ADD_DEFINITIONS( -DAPR_DECLARE_STATIC )
ELSE( WIN32 )
  ADD_DEFINITIONS( -D_LARGEFILE64_SOURCE -D_LCNTRACE -DWall )
ENDIF( WIN32 )

#CMake Warnung "Libraries muessen gesucht werden" verhindern
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

ADD_LIBRARY( lucenecpp
             ../lucenecpp/analyzer
             ../lucenecpp/analyzermap
             ../lucenecpp/autoreleasepool
             ../lucenecpp/booleanquery
             ../lucenecpp/directory
             ../lucenecpp/document
             ../lucenecpp/documentdumpiterator
             ../lucenecpp/exception
             ../lucenecpp/field
             ../lucenecpp/hits
             ../lucenecpp/indexreader
             ../lucenecpp/indexsearcher
             ../lucenecpp/lucenecpp
             ../lucenecpp/prefixquery
             ../lucenecpp/query
             ../lucenecpp/searcher
             ../lucenecpp/string
             ../lucenecpp/term
             ../lucenecpp/termquery
              )

IF( NOT WIN32 )
  TARGET_LINK_LIBRARIES( lucenecpp lucene apr-1 m z )
ELSE( NOT WIN32 )
ENDIF( NOT WIN32 )

#ADD_EXECUTABLE( testexe
#                blah.cpp )

#TARGET_LINK_LIBRARIES( testexe lucenecpp lucene apr-1 m z )
#
#ADD_CUSTOM_COMMAND( TARGET textexe
#                    POSTBUILD
#                    COMMAND ./testexe )

