TEMPLATE = lib
!win32:VERSION = 0.7
DESTDIR = $${QXT_BUILD_TREE}/lib
win32:DLLDESTDIR = $${QXT_BUILD_TREE}/bin
target.path = $${QXT_INSTALL_LIBS}
win32:dlltarget.path = $${QXT_INSTALL_BINS}
headers.path = $${QXT_INSTALL_HEADERS}/$${CLEAN_TARGET}
contains(CONVENIENCE, $${CLEAN_TARGET}){
    for(header,HEADERS){
        headers.files += $$header
    }
    headers.files += $${QXT_SOURCE_TREE}/include/$${CLEAN_TARGET}/*
}
INSTALLS += target headers
win32:INSTALLS += dlltarget

defineReplace(qxtLibraryTarget) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   mac:!static:contains(QT_CONFIG, qt_framework) {
      QMAKE_FRAMEWORK_BUNDLE_NAME = $$LIBRARY_NAME
      export(QMAKE_FRAMEWORK_BUNDLE_NAME)
   }
   contains(TEMPLATE, .*lib):CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:RET = $$member(LIBRARY_NAME, 0)
          else:win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

TARGET = $$qxtLibraryTarget($$CLEAN_TARGET)

include(qxtlibs.pri)

macx {
    CONFIG(qt_no_framework): CONFIG += absolute_library_soname
    CONFIG(qt_framework, qt_framework|qt_no_framework): CONFIG += lib_bundle 
    FRAMEWORK_HEADERS.version = Versions
    FRAMEWORK_HEADERS.files   = $${HEADERS}
    FRAMEWORK_HEADERS.path    = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
    QMAKE_LFLAGS += -F$${QXT_BUILD_TREE}/lib
}
unix|win32-g++* {
    CONFIG += create_pc create_prl
    QMAKE_PKGCONFIG_REQUIRES += QtCore
    QMAKE_PKGCONFIG_DESTDIR = pkgconfig
    QMAKE_PKGCONFIG_PREFIX = $${QXT_BUILD_TREE}
#    QMAKE_PKGCONFIG_LIBDIR = $${QXT_INSTALL_LIBS}
    QMAKE_PKGCONFIG_INCDIR = $${QXT_BUILD_TREE}/include/$${CLEAN_TARGET}
    include_replace.match = $$re_escape($${QMAKE_INCDIR_QXT})
    include_replace.replace = $${QXT_INSTALL_HEADERS}/$${CLEAN_TARGET}
    lib_replace.match = $$re_escape($${QMAKE_LIBDIR_QXT})
    lib_replace.replace = $${QXT_INSTALL_LIBS}
    prefix_replace.match = $$re_escape($$QXT_BUILD_TREE)
    prefix_replace.replace = $${QXT_INSTALL_PREFIX}
    QMAKE_PKGCONFIG_INSTALL_REPLACE += include_replace lib_replace prefix_replace
}

win32-msvc|win32-msvc2005|win32-msvc2008: QMAKE_LFLAGS_DEBUG += /PDB:$$DESTDIR/$${TARGET}.pdb

symbian {
    TARGET.EPOCALLOWDLLDATA = 1
    MMP_RULES += EXPORTUNFROZEN
}

astyle.params += --pad=oper
astyle.params += --unpad=paren
astyle.params += --convert-tabs
astyle.params += --brackets=break
astyle.params += --indent-namespaces
astyle.commands = astyle $$astyle.params $$HEADERS $$SOURCES
QMAKE_EXTRA_TARGETS += astyle

# cannot use .moc/.obj with gcov ("cannot open graph file")
!symbian:!contains(CONFIG, coverage) {
    CONFIG(debug, debug|release) {
        MOC_DIR          = debug/.moc
        OBJECTS_DIR      = debug/.obj
        RCC_DIR          = debug/.rcc
    } else {
        MOC_DIR          = release/.moc
        OBJECTS_DIR      = release/.obj
        RCC_DIR          = release/.rcc
    }
}
QMAKE_CLEAN += *.gcda *.gcno

contains(CONFIG, coverage) {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LIBS += -lgcov

    zerocounters.commands = @lcov --directory \$(OBJECTS_DIR) --zerocounters
    QMAKE_EXTRA_UNIX_TARGETS *= zerocounters

    capture.commands = @mkdir -p ../../coverage
    capture.commands += && lcov --directory \$(OBJECTS_DIR) --capture --output-file ../../coverage/\$(QMAKE_TARGET).cov
    capture.filters = \"/usr/*\" \"moc_*.cpp\" \"qrc_*.cpp\"
    capture.commands += && lcov --remove ../../coverage/\$(QMAKE_TARGET).cov $$capture.filters --output-file ../../coverage/\$(QMAKE_TARGET).cov
    QMAKE_EXTRA_UNIX_TARGETS *= capture

    genhtml.commands = @genhtml --output-directory ../../coverage/\$(QMAKE_TARGET) ../../coverage/\$(QMAKE_TARGET).cov
    genhtml.commands += && xdg-open ../../coverage/\$(QMAKE_TARGET)/index.html
    QMAKE_EXTRA_UNIX_TARGETS *= genhtml
}
