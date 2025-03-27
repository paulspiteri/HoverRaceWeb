add_library(ObjFac1 SHARED
    BallElement.cpp
    BumperGate.cpp
    DefaultSurface.cpp
    DoubleSpeedSource.cpp
    FinishLine.cpp
    FuelSource.cpp
    GenericRenderer.cpp
    HoverRender.cpp
    main.cpp
    Mine.cpp
    Missile.cpp
    PowerUp.cpp
    stdafx.cpp
    TestElement.cpp
    WoodSurface.cpp           
)

target_compile_definitions(ObjFac1 PRIVATE
    _WINDLL
    _AFXDLL
)

target_link_libraries(ObjFac1 PRIVATE MainCharacter Model ObjFacTools Util VideoServices
)