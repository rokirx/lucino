#!/usr/bin/python

#
# clbrowser - a browser for Lucino indices
# Author Valerij Klein 
# Released under GPL
# $Id$

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


import os
import sys
import traceback

# GTK and Gnome 
try:
    import pygtk
    pygtk.require("2.0")
except:
    pass
try:
    import gtk
    import gtk.glade
    import gobject
except:
    print "cannot import gtk, check your installation"
    traceback.print_exc(file=sys.stderr)    
    sys.exit(1)

import gnome.ui

# clbrowser
try:
    import clbrowser
    from clbrowser.misc import ClbConst
except:
    print "cannot import clbrowser, check your installation"
    traceback.print_exc(file=sys.stderr)
    sys.exit(1)

# i18n
import gettext
import locale


if __name__ == "__main__":
            
    gnome.init( ClbConst.APPNAME, ClbConst.VERSION )
    clbdir = os.path.dirname(sys.argv[0])
    app=clbrowser.ClbGUI(clbdir, clbdir+"/glade/"+ClbConst.GLADEFILE)        
        
    gtk.gdk.threads_init()
    gtk.gdk.threads_enter()    
    gtk.main()  
    gtk.gdk.threads_leave()
