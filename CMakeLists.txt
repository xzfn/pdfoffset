cmake_minimum_required(VERSION 3.13)

project(pdfoffset)

find_package(FLTK REQUIRED)

fltk_wrap_ui(pdfoffset src/ui.fl)

set(pdfoffsetcore_SOURCES
    src/pdfoffsetcore.h
    src/pdfoffsetcore.cpp
)

set(pdfoffset_SOURCES
    ${pdfoffsetcore_SOURCES}
    ${pdfoffset_FLTK_UI_SRCS}
    src/pdfoffset.cpp
)

add_executable(pdfoffset
    ${pdfoffset_SOURCES}
)

target_include_directories(pdfoffset PUBLIC ${FLTK_INCLUDE_DIR})
target_link_libraries(pdfoffset LINK_PUBLIC ${FLTK_LIBRARIES})
