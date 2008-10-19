"""Object tree dumper

Example application, which outputs gamesys as a text representation of the 
object tree it contains.

Parameters:
    -h, --help   - displays this message
    -g, --gamesys - specifies which gamesys should be loaded and dumped,
		filename only (has to be in the config path, sorry for that)
    -t, --type [t1, t2, ss2] - specifies the game type
    -c, --config - specifies the path to the config file
    -p, --props  - Property verbosity ('[]') - when specified, even the property
		values are output
    -l, --links  - Link verbosity ('<>') - if specified outputs all the links 
		that source from the object (MetaProp links are skipped)
    -m, --meta   - Metaprop verbosity ('{}') - if specified, metaprops are 
		listed in the output
    -a, --all    - a shortcut for all three (p,l,m)
    -v Verbose output - link/property values are included in the output
		(currently only prop. data)
    -d Debug - enables opde.log creation and loglevel 4 (verbose)
    
Config file:
    script_path=/some/path/to/thief.pldef_and_such_files
    script_path_extra can point to another place, for example the directory 
		containing dark.gam if such
    file is not located in the current directory.

This script was written by Volca, and can be modified and distributed freely. 
It is not an example of a propper python script, and should not be taken as such
(No exception handling, code is not in main, etc.)

Sort-of-a-Disclamer: No guarantee is made about the function of this, and also
note it might make your PC burn if something goes wrong, or anything like that;)
"""
import getopt, sys, Opde

def usage():
	sys.stderr.write( __doc__)

try:
	opts, args = getopt.getopt(sys.argv[1:], "hdg:t:c:plva", ["help", "gamsys=","gametype=","config=", "props", "links", "all"])
except getopt.GetoptError, err:
	# Print what went wrong and exit
	sys.stderr.write("Error parsing the arguments : %s\n" % err)
	usage()
	sys.exit(2)

# Default values:
gamesys="dark.gam"
gametype = "t1"
config = "tree.cfg"
debug = 0
detailProps = 0
detailLinks = 0
detailMeta = 0
verbose = 0

for o, a in opts:
	if o == "-d":
		debug = 1
	elif o == "-v":
		verbose = 1
	elif o in ("-p", "--props"):
		detailProps = 1
	elif o in ("-l", "--links"):
		detailLinks = 1
	elif o in ("-m", "--meta"):
		detailMeta = 1
	elif o in ("-a", "--all"):
		detailLinks = 1
		detailProps = 1
		detailMeta = 1
	elif o in ("-h", "--help"):
		usage()
		sys.exit()
	elif o in ("-g", "--gamesys"):
		gamesys = a
	elif o in ("-t", "--gametype"):
		gametype = a
	elif o in ("-c", "--config"):
		config = a
	else:
		sys.stderr.write("Unknown option %s\n" % o)
		usage()
		sys.exit()



# tell Opde.Root to only create CORE services (those which are sufficient for tools, no renderer, etc)
opderoot = Opde.createRoot(Opde.Services.SERVICE_CORE)

# Logging - defaults to none. the line below would enable logging to opde.log in the current directory
# Which is good for debugging
# Log level : 0-4 
if (debug != 0):
	opderoot.logToFile("opde.log")
	opderoot.setLogLevel(4)
else:
	opderoot.setLogLevel(0)

# Opde logging can be done from python with Opde.log_debug, Opde.log_error and such functions (see bindings.h)

# Setup resources
# Should have some better way of providing the resource config...
# Loading resources autotmatically will result in an assert over here (No texture manager singleton)
# This file should better exist, otherwise an exception will jump up from the call...
opderoot.loadConfigFile(config)

# Just some resource path initializations
# These configurations should be based on some platform service
cfgsrv = Opde.Services.getConfigService()
opderoot.addResourceLocation(cfgsrv.getParam("script_path"), "Dir", "General", False)

if (cfgsrv.hasParam("script_path_extra") == True):
	opderoot.addResourceLocation(cfgsrv.getParam("script_path_extra"), "Dir", "General", False)

# Load the dype scripts (this varies depending on the game type)
opderoot.loadDTypeScript(gametype + "-types.dtype", "General")
opderoot.loadPLDefScript(gametype + "-links.pldef", "General")
opderoot.loadPLDefScript(gametype + "-props.pldef", "General")

# Bootstrapping finished. No more initialization needed, inform the services
opderoot.bootstrapFinished();

# Service globals
dbsrv = Opde.Services.getDatabaseService()
linksrv = Opde.Services.getLinkService()
inhsrv = Opde.Services.getInheritService()
objsrv = Opde.Services.getObjectService()
psrv = Opde.Services.getPropertyService()

# Load the gamesys (note: dbsrv.load("miss1.mis") would load both mission-2 and gamesys-1)
# ogre is case sensitive on linux. Will have to resolve this
dbsrv.loadGameSys(gamesys)

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

def getPropStr(oid,p):
	# get all prop field names
	pd = psrv.getPropertyFieldsDesc(p)

	pv = []

	for d in pd:
		# get value as string
		
		# exception... maybe more these should be specified, we'll see (or have a hidden attrib.)
		# these are those modified on every .GAM save
		if (not (p=="ClassTags" and d.name=="value")) and (not (p=="ParticleGroup" and d.name.startswith("zero3"))):
			pv.append("%s" % psrv.get(oid, p, d.name))

	return "{" + (" ".join(pv)) + "}"


def getObjProps(oid):
	"""Lists properties the object has"""
	res = []
	
	# iterate over all prop names, find those that are owned by the object
	for s in psrv.getAllPropertyNames():
		if psrv.owns(oid, s) and s not in ['DonorType','SymbolicName']: # Skip DonorType and SymbolicName, they are mandatory
			res.append(s) # Here, we could convert prop values to {val,} as dark does it, having all the object's props listed
		
			if verbose != 0:
				res.append(getPropStr(oid, s))
	
	return " ".join(res)
	
def getObjLinks(oid):
	"""Lists links the object has"""
	res = []
	
	# iterate over all prop names, find those that are owned by the object
	for ln in linksrv.getAllLinkNames():
		# Ignore reverse links
		if ln.startswith('~'):
			continue;
		
		# Ignore metaprop
		if ln == "MetaProp":
			continue;
		
		# ask if the link has any outgoing
		rel = linksrv.getRelation(ln)
		
		tgt = rel.getAllLinks(oid, 0)
		
		# Single link output
		lres = []
		
		for l in tgt:
			lres.append("%s" % l.dst)
		
		if lres != []:
			res.append("%s [%s]" % (ln, " ".join(lres)))
	
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
	props = ""
	links = ""
	mprops = ""
	
	if detailProps:
		props = getObjProps(oid)
		
	if detailLinks:
		links = getObjLinks(oid)
	
	# And MP's which are linked to this object
	if detailMeta:
		mprops = getObjMProps(oid)

	# some formatting
	if props != '':
		props =  ' [' + props + ']'
		
	if links != '':
		links =  ' <' + links + '>'

	if mprops != '':
		mprops =  ' {' + mprops + '}'

	dtypestr = " ";
	dtype = 0;

	# does the object have DonorType property? All gamesys object should!
	if (psrv.has(oid, "DonorType")):
		# get DonorType property value
		dtype = psrv.get(oid, "DonorType", "")

	# if the donor type is nonzero, we encountered a MetaProperty!
	if (dtype == 1):
		dtypestr = "[M] "

	# print out the object info
	print indent + '-+ ' + dtypestr + name + '(' + str(oid)  + ')' + props + mprops + links 

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
Opde.log_info("Terminating Opde");
