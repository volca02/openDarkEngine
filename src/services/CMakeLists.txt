###### Basic Services - background for others ######
# binary service, handling data definitions
add_subdirectory(binary)

# Config service, handling game options
add_subdirectory(config)



###### Object system services ######
# Link service, handling links
add_subdirectory(link)

# Inheritance service, handling inheritance of goodies (Properties, really) across the object tree 
add_subdirectory(inherit)

# Property service, handling game object properties (value sets)
add_subdirectory(property)

# Object service. Handles objects (creation, deletion, MP addition and removal, MP queries)
add_subdirectory(object)



###### Property realization services ######
# Renderer service
add_subdirectory(render)



###### Miscelenaous services ######
# WorldRep service, Handling world geometry loading
add_subdirectory(worldrep)

# Game service, handling the high game states
add_subdirectory(game)

# Dark Database handling service
add_subdirectory(database)

# Input management service
add_subdirectory(input)

# Loop service. The main loop code run by the main.cpp
add_subdirectory(loop)

# GUI service. Handles gui and overlays
add_subdirectory(gui)

# Script service. Script execution and object scripts
add_subdirectory(script)
