add_library(Util STATIC
    StringResources.cpp
    RecordFile.cpp
    DllObjectFactory.cpp
    FuzzyLogic.cpp
    Profiler.cpp
    StrRes.cpp
    WorldCoordinates.cpp
)

target_compile_definitions(Util PRIVATE
    _WINDLL
    _AFXDLL
    MR_UTIL
)

target_link_libraries(Util
)