add_library(MainCharacter STATIC
    MainCharacter.cpp
    MainCharacterRenderer.cpp
)

target_compile_definitions(MainCharacter PRIVATE
    MR_MAIN_CHARACTER
)

target_link_libraries(MainCharacter PRIVATE Model Util
)