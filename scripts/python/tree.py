"""Object tree dumper

Example application, which outputs gamesys as a text representation of the object tree it contains.

For now, this application should be run with opdeScript ("opdeScript tree.py"), and needs tree.cfg
in the current directory containing at a line like this:
script_path=/some/path/to/thief.pldef_and_such_files

script_path_extra can point to another place, for example the directory containing dark.gam if such
file is not located in the current directory.

Currently, this script is hardcoded to thief1 files. Change the load*Script lines to adapt to the
other two games.

This script was written by Volca, and can be modified and distributed freely.

Sort-of-a-Disclamer: No guarantee is made to the function of this, and also note it can make your PC
burn or something like that ;)
"""
from Opde import *


# tell Opde.Root to only create CORE services (those which are sufficient for tools, no renderer, etc)
opderoot = createRoot(Services.SERVICE_CORE)

# Logging - defaults to none. the line below would enable logging to opde.log in the current directory
# Which is good for debugging
# opderoot.logToFile("opde.log")
# Log level : 0-4 
opderoot.setLogLevel(0)
# Opde logging can be done from python with Opde.log_debug, Opde.log_error and such functions (see bindings.h)

# Setup resources
# Should have some better way of providing the resource config...
# Loading resources autotmatically will result in an assert over here (No texture manager singleton)
# This file should better exist, otherwise an exception will jump up from the call...
opderoot.loadConfigFile("tree.cfg")

# Just some resource path initializations
# These configurations should be based on some platform service
cfgsrv = Services.getConfigService()
opderoot.addResourceLocation(cfgsrv.getParam("script_path"), "FileSystem", "General", False)

if (cfgsrv.hasParam("script_path_extra") == True):
    opderoot.addResourceLocation(cfgsrv.getParam("script_path_extra"), "FileSystem", "General", False)

# Load the dype scripts (this varies depending on the game type)
opderoot.loadDTypeScript("t1-types.dtype", "General")
opderoot.loadPLDefScript("t1-links.pldef", "General")
opderoot.loadPLDefScript("t1-props.pldef", "General")

# Bootstrapping finished. No more initialization needed, inform the services
opderoot.bootstrapFinished();

# Service globals
dbsrv = Services.getDatabaseService()
linksrv = Services.getLinkService()
inhsrv = Services.getInheritService()
objsrv = Services.getObjectService()
psrv = Services.getPropertyService()

# Load the gamesys (note: dbsrv.load("miss1.mis") would load both mission-2 and gamesys-1)
# ogre is case sensitive on linux. Will have to resolve this
dbsrv.loadGameSys("Dark.gam")

# an example (not used here): Query which properties the given object owns, and which it inherits
def objectInfo(oid):
    name = objsrv.getName(oid)
    print "Object Info for object " + str(oid) + " named '" + name + "':"

    for s in psrv.getAllPropertyNames():
	if psrv.has(oid, s):
	    if psrv.owns(oid, s):
		print " * Object " + str(oid) + " has property: " + s
	    else:
		print " * Object " + str(oid) + " inherits property: " + s

    # If there was a method to get all relations, we could do the same with links

def getObjProps(oid):
    """Lists properties the object has"""
    res = []
    
    # iterate over all prop names, find those that are owned by the object
    for s in psrv.getAllPropertyNames():
	if psrv.owns(oid, s) and s not in ['DonorType','SymbolicName']: # Skip DonorType and SymbolicName, they are mandatory
	    res.append(s) # Here, we could convert prop values to {val,} as dark does it, having all the object's props listed

    return " ".join(res)

def getObjMProps(oid):
    """Lists metaproperties the object has"""
    res = []

    # iterate over inheritance sources, find those with nonzero priority    
    for s in inhsrv.getSources(oid):
	if s.priority != 0:
	    res.append(objsrv.getName(s.src))

    return " ".join(res)

# Object tree renderer
def otree(oid,indent):
    """Renders the object tree to the standard output"""
    # get the obj's name
    name = objsrv.getName(oid)

    # load prop names the object implements
    props = getObjProps(oid)
    # And MP's which are linked to this object
    mprops = getObjMProps(oid)
    
    # some formatting
    if props != '':
        props =  ' [' + props + ']'
	
    if mprops != '':
        mprops =  ' {' + mprops + '}'

    dtypestr = " ";
    
    # does the object have DonorType property? All gamesys object should!
    if (psrv.has(oid, "DonorType")):
        # get DonorType property value
        dtype = psrv.get(oid, "DonorType", "")
    
	# if the donor type is nonzero, we encountered a MetaProperty!
        if (dtype == 1):
	    dtypestr = "[M] "
    
    # print out the object info
    print indent + '-+ ' + dtypestr + name + '(' + str(oid)  + ')' + props + mprops 
    
    # a formatting related variable
    single = 1

    # get inheritance targets, iterate over those
    for l in inhsrv.getTargets(oid):
	# the inh. target is not inheriting as MP, but as archetype
	if l.priority == 0:
	    # so we'll format
	    single = 0
	    # recurse with the target property
	    otree(l.dst, indent + ' |')
	
    if not single:
	print indent

# An example Object tree list for all the archetype base objects (id can change, name hopefully not)
for srcobj in ["Object", "MetaProperty", "Stimulus", "Flow Group", "Base Room", "Texture"]:
    print "================ Object tree for " + srcobj + " ====================="
    otree(objsrv.named(srcobj), "")
    print

# see, this is the way to log from python (the python log calls are prefixed "Python:" in the log)
log_info("Terminating Opde");
