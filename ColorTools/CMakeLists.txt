add_library(ColorTools STATIC
    ColorTools.cpp
)

target_compile_definitions(ColorTools PRIVATE
    MR_COLOR_TOOLS
)

target_link_libraries(ColorTools PRIVATE Util)
