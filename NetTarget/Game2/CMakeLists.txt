add_executable(Game2
    Banner.cpp
    ClientSession.cpp
    CommonDialog.cpp
    GameApp.cpp
    InternetRoom.cpp
    License.cpp
    main.cpp
    MatchReport.cpp
    NetInterface.cpp
    NetworkSession.cpp
    Observer.cpp
    Sponsor.cpp
    StdAfx.cpp
    TrackSelect.cpp
)

target_compile_definitions(Game2 PRIVATE
    WIN32
    _WINDOWS
    _MBNET_
    _AFXDLL
)

target_link_libraries(Game2 PRIVATE MainCharacter Model ObjFacTools Util VideoServices
    vfw32.lib
    wsock32.lib
    winmm.lib
)

target_link_options(Game2 PRIVATE "/subsystem:windows")
