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

target_link_libraries(Util
    winmm.lib
)