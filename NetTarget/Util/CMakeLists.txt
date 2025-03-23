add_library(Util SHARED
    DllObjectFactory.cpp
    FuzzyLogic.cpp
    Profiler.cpp
    RecordFile.cpp
    stdafx.cpp
    StrRes.cpp
    WorldCoordinates.cpp
)

target_compile_definitions(Util PRIVATE
    _WINDLL
    _AFXDLL
    MR_UTIL
)

target_compile_options(Util PRIVATE
    /permissive-         # Enable strict standards compliance
    /Zc:__cplusplus       # Correctly defines the `__cplusplus` macro for Clang
)
target_link_libraries(Util
    winmm.lib
)