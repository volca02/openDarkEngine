import wx
import sys, getopt, opde

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


# Tell Opde.Root to only create CORE services (those which are sufficient for tools, no renderer, etc)
if (debug != 0):
	opderoot = opde.createRoot(opde.services.SERVICE_CORE, "opde.log")
	opderoot.setLogLevel(4)
else:
	opderoot = opde.createRoot(opde.services.SERVICE_CORE)
	opderoot.setLogLevel(0)

# Opde logging can be done from python with Opde.log_debug, Opde.log_error and such functions (see bindings.h)

# Setup resources
# Should have some better way of providing the resource config...
# Loading resources autotmatically will result in an assert over here (No texture manager singleton)
# This file should better exist, otherwise an exception will jump up from the call...
opderoot.loadConfigFile(config)

# Just some resource path initializations
# These configurations should be based on some platform service
cfgsrv = opde.services.getConfigService()
spath = cfgsrv.getParam("script_path")

if (spath):
	opderoot.addResourceLocation(spath, "Dir", "General", False)

if (cfgsrv.hasParam("script_path_extra") == True):
	opderoot.addResourceLocation(cfgsrv.getParam("script_path_extra"), "Dir", "General", False)

# Load the dtype scripts (this varies depending on the game type)
opderoot.loadDTypeScript(gametype + "-types.dtype", "General")
opderoot.loadPLDefScript(gametype + "-links.pldef", "General")
opderoot.loadPLDefScript(gametype + "-props.pldef", "General")

# Bootstrapping finished. No more initialization needed, inform the services
opderoot.bootstrapFinished();

# Service globals
dbsrv = opde.services.getDatabaseService()
linksrv = opde.services.getLinkService()
inhsrv = opde.services.getInheritService()
objsrv = opde.services.getObjectService()
psrv = opde.services.getPropertyService()

# Load the gamesys (note: dbsrv.load("miss1.mis") would load both mission-2 and gamesys-1)
# Ogre is case sensitive on linux. Will have to resolve this

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


def getLinkStr(lid,flav):
	# get all prop field names
	pd = linksrv.getFieldsDesc(flav)
	rel = linksrv.getRelation(flav)
	
	lv = []

	for d in pd:
		# get value as string
		lv.append("%s" % rel.getLinkField(lid, d.name))

	return "{" + (" ".join(lv)) + "}"


def getObjProps(oid):
	"""Lists properties the object has"""
	res = []
	
	# iterate over all prop names, find those that are owned by the object
	for s in psrv.getAllPropertyNames():
		if psrv.owns(oid, s):
			res.append((s, getPropStr(oid, s))) 
	
	return res


# ================ WX ================
class LazyTree(wx.TreeCtrl):
    ''' LazyTree is a simple "Lazy Evaluation" tree, that is, it only adds 
        items to the tree view when they are needed.'''

    def __init__(self, *args, **kwargs):
        super(LazyTree, self).__init__(*args, **kwargs)
        self.Bind(wx.EVT_TREE_ITEM_EXPANDING, self.OnExpandItem)
        self.Bind(wx.EVT_TREE_ITEM_COLLAPSING, self.OnCollapseItem)
        self.__collapsing = False

	dbsrv.load(gamesys, opde.services.DBM_FILETYPE_GAM)
	
	rootname = "Object"
        id = objsrv.named(rootname)
	root = self.AddRoot(rootname)
	self.SetPyData(root, id)
	
	if inhsrv.hasTargets(id):
		self.SetItemHasChildren(root)
	

    def setObjectHasChildren(item):
	oid = self.getPyData(item)
	if inhsrv.hasTargets(oid):
		self.SetItemHasChildren(item)

    def OnExpandItem(self, event):
        item = event.GetItem()
	oid = self.GetPyData(item)
	
	for l in inhsrv.getTargets(oid):
		# the inh. target is not inheriting as MP, but as archetype
		if l.priority == 0:
			id = l.dst
			name = objsrv.getName(id)
			child = self.AppendItem(item, name)
			self.SetPyData(child, id)
			if inhsrv.hasTargets(id):
				self.SetItemHasChildren(child)

    def OnCollapseItem(self, event):
        # Be prepared, self.CollapseAndReset below may cause
        # another wx.EVT_TREE_ITEM_COLLAPSING event being triggered.
        if self.__collapsing:
            event.Veto()
        else:
            self.__collapsing = True
            item = event.GetItem()
            self.CollapseAndReset(item)
            self.SetItemHasChildren(item)
            self.__collapsing = False


class LazyTreeFrame(wx.Frame):
    def __init__(self, *args, **kwargs):
        super(LazyTreeFrame, self).__init__(*args, **kwargs)

	splitter = wx.SplitterWindow(self, -1)
	splitter_v = wx.SplitterWindow(splitter, -1)

        vbox = wx.BoxSizer(wx.VERTICAL)
	vbox_d = wx.BoxSizer(wx.VERTICAL)
	vbox_p = wx.BoxSizer(wx.VERTICAL)	
	
        panel1 = wx.Panel(splitter, -1)

	panel3 = wx.Panel(splitter_v, -1)
	panel4 = wx.Panel(splitter_v, -1)

        self.tree = LazyTree(panel1, 1)

#        self.display = wx.StaticText(panel2, -1, '',(10,10), style=wx.ALIGN_CENTRE)
	self.lc = wx.ListCtrl(panel3, 2, style=wx.LC_REPORT)
        self.lc.InsertColumn(0, 'Attribute')
        self.lc.InsertColumn(1, 'Value')
	self.lc.SetColumnWidth(0, 150)
	self.lc.SetColumnWidth(1, 150)
	
	# detail list of property values
	self.detl = wx.ListCtrl(panel4, 3, style=wx.LC_REPORT)
	self.detl.InsertColumn(0, 'Attribute')
        self.detl.InsertColumn(1, 'Value')
	self.detl.SetColumnWidth(0, 150)
	self.detl.SetColumnWidth(1, 150)


	self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelChanged, id=1)
	self.lc.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnPropSelChanged, id=2)
	
	vbox.Add(self.tree, 1, wx.EXPAND)
	vbox_d.Add(self.lc, 1, wx.EXPAND)
	vbox_p.Add(self.detl, 1, wx.EXPAND)
	
	panel1.SetSizer(vbox)
	panel3.SetSizer(vbox_d)
	panel4.SetSizer(vbox_p)
	
	splitter_v.SplitHorizontally(panel3, panel4)
	splitter.SplitVertically(panel1, splitter_v)
	
	

#        self.SetSizer(splitter)
        self.Centre()
	
	self.objectId = -1
	
    def OnPropSelChanged(self, event):
    	self.detl.DeleteAllItems()
	pn = event.GetItem().GetText()
	
	# list detail
	pd = psrv.getPropertyFieldsDesc(pn)

	for d in pd:
		# get value as string
		num_items = self.detl.GetItemCount()
		
		# exception... maybe more these should be specified, we'll see (or have a hidden attrib.)
		# these are those modified on every .GAM save
		if (not (pn=="ClassTags" and d.name=="value")) and (not (pn=="ParticleGroup" and d.name.startswith("zero3"))):
			self.detl.InsertStringItem(num_items, d.name)
			value = psrv.get(self.objectId, pn, d.name)
	        	self.detl.SetStringItem(num_items, 1, str(value))

	
    def OnSelChanged(self, event):
    	# Order the Detail tree to be rebuilt
        item = event.GetItem()
	self.lc.DeleteAllItems()
	self.detl.DeleteAllItems()
	self.objectId = self.tree.GetPyData(item)
	
	# List all the properties
	for t in getObjProps(self.objectId):
		num_items = self.lc.GetItemCount()
		self.lc.InsertStringItem(num_items, t[0])
	        self.lc.SetStringItem(num_items, 1, t[1])
#		self.lc.SetItemData(num_items, t[0])

		
	
#        self.display.SetLabel(self.tree.GetItemText(item))

	


if __name__ == "__main__":
    app = wx.App(False)
    frame = LazyTreeFrame(None)
    frame.Show()
    app.MainLoop()
