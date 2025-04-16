add_library(ObjFac1 STATIC
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
    TestElement.cpp
    WoodSurface.cpp           
)

target_compile_definitions(ObjFac1 PRIVATE
)

target_link_libraries(ObjFac1 PRIVATE Model Util MainCharacter ObjFacTools
)