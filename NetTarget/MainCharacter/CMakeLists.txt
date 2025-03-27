add_library(MainCharacter SHARED
    MainCharacter.cpp
    MainCharacterRenderer.cpp
    stdafx.cpp                  
)

target_compile_definitions(MainCharacter PRIVATE
    _WINDLL
    _AFXDLL
    MR_MAIN_CHARACTER
)

target_link_libraries(MainCharacter PRIVATE Model Util VideoServices
)