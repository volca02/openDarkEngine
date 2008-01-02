from Opde import *

ls = Services.LoopService()

def exitRequest():
    log_info("Termination requested!")
    ls.requestTermination()

if (not ls.requestLoopMode("GUIOnlyLoopMode")):
    log_error("Could not set loop mode to GUIOnlyLoopMode!")
else:
    ls.run()
    
log_info("Terminating Opde");