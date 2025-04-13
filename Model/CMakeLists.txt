add_library(Model STATIC
    ConcreteShape.cpp          
    ContactEffect.cpp          
    FreeElementMovingHelper.cpp
    GameSession.cpp            
    Level.cpp                  
    MazeElement.cpp            
    PhysicalCollision.cpp      
    ShapeCollisions.cpp        
    Shapes.cpp                 
)

target_compile_definitions(Model PRIVATE
    MR_MODEL
)

target_link_libraries(Model PRIVATE Util
)