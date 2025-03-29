add_library(ColorTools STATIC
    ColorTools.cpp
    stdafx.cpp
)

target_compile_definitions(ColorTools PRIVATE
    _WINDLL
    _AFXDLL
    MR_COLOR_TOOLS
)

target_link_libraries(ColorTools PRIVATE Util)
