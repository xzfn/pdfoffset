cmake_minimum_required(VERSION 3.13)

project(pdfoffset)

# packages
find_package(FLTK REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)

# libqpdf without CMake
set(QPDF_INCLUDE_DIR "QPDF_INCLUDE_DIR_NOTFOUND" CACHE PATH "QPDF include directory")
set(QPDF_LINK_LIBRARIES "QPDF_LIBRARY_FILE_NOTFOUND" CACHE FILEPATH "QPDF qpdf.lib file")

# fltk ui, requires fluid
fltk_wrap_ui(pdfoffset src/ui.fl)

# core functionality
set(pdfoffsetcore_SOURCES
    src/pdfoffsetcore.h
    src/pdfoffsetcore.cpp
)

# win32 resource
set(pdfoffset_RESOURCES
    src/pdfoffset.rc
)

# app sources
set(pdfoffset_SOURCES
    ${pdfoffsetcore_SOURCES}
    ${pdfoffset_FLTK_UI_SRCS}
    src/pdfoffset.cpp
    ${pdfoffset_RESOURCES}
)

# app with ui
add_executable(pdfoffset
    ${pdfoffset_SOURCES}
)

# use fltk library
target_include_directories(pdfoffset PUBLIC ${FLTK_INCLUDE_DIR})
target_link_libraries(pdfoffset ${FLTK_LIBRARIES})

# use qpdf library
target_include_directories(pdfoffset PUBLIC ${QPDF_INCLUDE_DIR})
target_link_libraries(pdfoffset ${QPDF_LINK_LIBRARIES})

# use zlib static library
target_include_directories(pdfoffset PUBLIC ${ZLIB_INCLUDE_DIRS})
target_link_libraries(pdfoffset ${ZLIB_INCLUDE_DIRS}/../lib/zlibstatic.lib)

# use jpeg static library
target_include_directories(pdfoffset PUBLIC ${JPEG_INCLUDE_DIRS})
target_link_libraries(pdfoffset ${JPEG_INCLUDE_DIRS}/../lib/jpeg-static.lib)
