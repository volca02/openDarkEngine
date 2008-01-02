# binary service, handling data definitions
add_subdirectory(binary)

# WorldRep service, Handling world geometry loading
add_subdirectory(worldrep)

# Game service, handling the high game states
add_subdirectory(game)

# Config service, handling game options
add_subdirectory(config)

#### Object system services ####
# Link service, handling links
add_subdirectory(link)
# Inheritance service, handling inheritance of goodies (Properties, really) across the object tree 
add_subdirectory(inherit)
# Property service, handling game object properties (value sets)
add_subdirectory(property)

# Object service,  handling game object creation and slay
# TODO: add_subdirectory(object)

# Renderer service
add_subdirectory(render)

# Dark Database handling service
add_subdirectory(database)

# Input management service
add_subdirectory(input)

# Loop service. The main loop code run by the main.cpp
add_subdirectory(loop)

# Script service. Script execution and object scripts
add_subdirectory(script)