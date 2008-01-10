from Opde import *

# Service globals
lsrv = Services.LoopService()
isrv = Services.InputService()

# ------------- State management -------------
# To sim menu switcher
# To be called from a key input handler
def switchToSimMenu():
    log_debug("Switching to sim menu")
    lsrv.requestLoopMode("GUIOnlyLoopMode")
    # TODO:
    # guisrv.setActiveSheet(SimMenuSheet)
    # guisrv.setActive(true)
    # guisrv.setVisible(true)
    # render service -> hide world geometry
    # simservice.stop or thing like that to avoid step problems (but this can be auto via loopModeEnded)
    
# The reverse (to game switch from gui)
def switchToGameMode():
    log_debug("Switching to game mode")
    # TODO: Render service -> set world visible
    lsrv.requestLoopMode("AllClientsLoopMode")
    # guisrv.setActiveSheet(GameModeSheet)
    # guisrv.setVisible(true)
    # guisrv.setActive(false)
    # just to be sure
    isrv.setBindContext("game")


# ----- Input handlers -----
# Example: Handler to switch to sim menu (mapped on esc f.e.)
def simMenuRequest(msg):
    switchToSimMenu()

def exitRequest(msg):
    log_info("Received a message for exitRequest " + str(msg['event']))
    log_info("Termination requested!")
    lsrv.requestTermination()

def debugFrameRequest(msg):
    log_info("Received a message for frame debugger. Will debug one frame")
    lsrv.debugOneFrame()

# ------------ Main code -------------

# A small thought: A callback would be created in database service to support redraws.
# But would be handled here by PythonCallback (DatabaseProgressMessage), like this:
# class LoadSaveScreen: .... Holds sheet and ui elements, publishes methods to set progress...
# loadScreen = LoadSaveScreen.create(guisrv)
def databaseProgressUpdate(msg):
    log_info("Loading progress : " + str(msg['completed']  * 100))
#    loadScreen.setLeftGauge(msg['progress_master'])
#    loadScreen.setRightGauge(msg['progress_slave'])
#    rendersrv.renderOneFrame()
#
#    if (msg.finished()):
#	loadScreen.finished()
# 

# Setting bind context. This can be used to distingush between, for example, editor and game, etc.
isrv.createBindContext("game")
isrv.setBindContext("game")

# Callback on escape key trap
isrv.registerCommandTrap("exit_request", exitRequest)
# Debug frame shortcut trap
isrv.registerCommandTrap("debug_frame", debugFrameRequest)

# Map to escape and 1 keys (TODO: hardcoded_bind command could come handy to avoid breakage)
isrv.command("bind esc exit_request") # Would be sim_menu switcher binding, for example.
isrv.command("bind 1 debug_frame")

# guisrv = Services.GUIService()
# guisrv.setVisible(True)
# guisrv.setActive(True)

# A sample mission load
dbsrv = Services.DatabaseService()
dbsrv.setProgressListener(databaseProgressUpdate)
# dbsrv.load("miss1.mis")

# Loop setup and execution
if (not lsrv.requestLoopMode("GUIOnlyLoopMode")):
    log_error("Could not set loop mode to GUIOnlyLoopMode!")
else:
    lsrv.run() # Run the main loop
    
# Termination
log_info("Terminating Opde");

isrv.unregisterCommandTrap("debug_frame")
isrv.unregisterCommandTrap("exit_request")
dbsrv.unsetProgressListener()