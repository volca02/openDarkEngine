from Opde import *

print "Loaded Opde module! Yay!

s = Services.ConfigService()

print "Got config service! Yay Yay!"

print "Do we have a parameter test?: '" + str(s.hasParam("test")) + "'"

print "Python read config parameter: '" + s.getParam("test") + "'"

log_fatal("A fatal log test!")
log_error("Error test")
log_info("Info test")
log_debug("Debug test")
log_verbose("Verbose test")

ls = Services.LinkService()
print "Link flavor 0 name:" + ls.flavorToName(0) // Translate the link ID 1 to name

rel = ls.getRelation("TestRelation")
print "Relation's flavor : " + str(rel.getID())
print "Relation's name   : " + rel.getName()

id = rel.create(1,2) // link from 1 to 2
print "New link id       : " + str(id)
print "Link field value  : " + str(rel.getLinkField(id, ""))
rel.setLinkField(id,"",4)
print "set to 4!"
print "Link field value  : " + str(rel.getLinkField(id, ""))

print "Repr of rel : " + repr(rel)

id = rel.create(1,3) // second link, 1 to 3

rel.setLinkField(id,"",6)

for l in rel.getAllLinks(1,0): // Query all links that start in 1
	print "Link ID:" + str(l['id']) + " goes " + str(l['src']) + " -> " + str(l['dst']) + " flav. " + str(l['flavor'])
	print " * Link data val : " + str(rel.getLinkField(l['id'],''))

print "-- One Link -- "
lnk = rel.getOneLink(1,3)
print "1 -> 3 link id : " + str(lnk['id'])
