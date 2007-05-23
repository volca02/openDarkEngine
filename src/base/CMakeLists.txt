add_subdirectory(logger)
add_subdirectory(console)
add_subdirectory(servicemanager)
# add_subdirectory(gui)
add_subdirectory(file)
add_subdirectory(dyntype)

add_library ( OpdeBase OpdeException.cpp RefCounted.cpp )
