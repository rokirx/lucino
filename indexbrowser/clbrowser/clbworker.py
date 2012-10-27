import sys
import time
import os

# Released under GPL
# $Id$
# Valerij Klein <vklein@users.sourceforge.net>


try:
    import pygtk
    #tell pyGTK, if possible, that we want GTKv2
    pygtk.require("2.0")
except:
    #Some distributions come with GTK2, but not pyGTK
    pass

try:
    import gtk
    import gtk.glade
    import gobject
    import pango
except:
    print "Cannot import gtk modules"
    print "An error occured"
    sys.exit(1)

import gnome.ui
from misc import ClbConst

import threading
import Queue
import traceback
try:
    import lucino
except:
    print "  An error occured: cannot import lucino bindings."
    print "  Please install lucino bindings first."
    sys.exit(2)


class ClbWorkerThread(threading.Thread):

    waitcursor = gtk.gdk.Cursor(gtk.gdk.WATCH)
    defaultcursor = gtk.gdk.Cursor(gtk.gdk.LEFT_PTR)   

    def __init__(self, wTree, clbdir, indexdir, sessname, position):
        threading.Thread.__init__(self)
        self.wTree = wTree
        self.queue = Queue.Queue()
        self.sessname = sessname
        self.indexdir = indexdir
        self.textview = None
        # create new lucino searcher
        try:
            self._searcher = lucino.Searcher(self.indexdir)            
            (self.wTree.get_widget("indexdir_label")).set_text(self.indexdir)       
        except Exception, ex:
            raise ex
        # create session tab
        (self.wTree.get_widget( "session_notebook" )).set_current_page(self._addSessionTab(position))
                
            
    #
    #
    # Image fuer die Toolbar
    def _create_execute_image(self):
        img = gtk.Image()
        img.set_from_stock(gtk.STOCK_EXECUTE, gtk.ICON_SIZE_SMALL_TOOLBAR)
        return img

    #
    #
    # Neues Tab (Neue Session) anlegen
    def _addSessionTab(self, position):
        ''' add query tab '''    
        self.tooltips = gtk.Tooltips()
        query_notebook = self.wTree.get_widget( "session_notebook" )
        #query_tree = pkgtree.PkgTree(self.wTree)  
        
        # create tab, add treeview and fill with data
        query_tab = gtk.VBox(homogeneous=False, spacing=0)   

        # create toolbar
        toolbar = gtk.Toolbar()
               
        toolbutton_execute = gtk.ToolButton(icon_widget=self._create_execute_image())  
        toolbutton_execute.set_label("execute (F9)")
        self.tooltips.set_tip(toolbutton_execute, "Execute")      
        toolbutton_execute.connect("clicked", self._executeQuery, None)
        
        toolbar.add(toolbutton_execute)
        
        vpaned = gtk.VPaned()
        scrwin1 = gtk.ScrolledWindow()
        scrwin1.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scrwin1.set_size_request(-1, 200)
        
        # add textview 
        self.textview = gtk.TextView()
        self.textview.connect("key_press_event", self._keyPressedTextview, None)
        scrwin1.add(self.textview)        
        
        # resultbox
        resultbox = gtk.VBox(homogeneous=False, spacing=0)

        scrwin2 = gtk.ScrolledWindow()
        scrwin2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)        
        self.treestore = gtk.TreeStore( long, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str,
                                        str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str  ) 
        self.treeview = gtk.TreeView(self.treestore)
        self.treeview.set_rules_hint(True)
        scrwin2.add(self.treeview)
        
        result_helpers_hbox = gtk.HBox()
        self.resultlabel = gtk.Label("")
        self.resultlabel.set_size_request(-1, 30)
        self.resultlabel.set_alignment(0.05, 0.0)
        
        spinnerlabel = gtk.Label("Amount rows: ")
        self.rownumspinner = gtk.SpinButton(None, 0.0, 0)
        self.rownumspinner.set_increments(1.0, 10.0)        
        self.rownumspinner.set_range(0.0, 10000.0)
        self.rownumspinner.set_numeric(True)
        self.rownumspinner.set_value(49)
        self.rownumspinner.spin(gtk.SPIN_STEP_FORWARD, increment=1.0)
        
        result_helpers_hbox.pack_start(self.resultlabel, expand=False, fill=False, padding=0)
        result_helpers_hbox.pack_end(self.rownumspinner, expand=False, fill=False, padding=0)
        result_helpers_hbox.pack_end(spinnerlabel, expand=False, fill=False, padding=0)
                        
        resultbox.pack_start(result_helpers_hbox, expand=False, fill=False, padding=0)
        resultbox.add(scrwin2)

        vpaned.add1(scrwin1)
        vpaned.add2(resultbox)

        query_tab.pack_start(toolbar, False, False, 0)
        query_tab.add(vpaned)

        query_tab.show_all()
        
        return query_notebook.insert_page(query_tab, self._queryTabLabel(position), position)
        
    #
    #
    # fuehrt Lucino-Query aus
    # Dazu wird das markierte Bereich aus dem Textfeld gelesen und an den searcher uebergeben        
    def _executeQuery(self, widget, data):
        textbuffer = self.textview.get_buffer()
        lcnquery = None
        try:
            textbuffer = self.textview.get_buffer()
            textiter = textbuffer.get_iter_at_mark(textbuffer.get_insert())
            startlineiter = textbuffer.get_iter_at_line(textiter.get_line())
            textiter.forward_to_line_end()

            #(iterStart, iterEnd) = textbuffer.get_selection_bounds()
            lcnquery = textbuffer.get_text(startlineiter, textiter)
            if len(lcnquery)==0:
                error_dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, "Current line is empty")
                if error_dialog.run():
                    error_dialog.hide()
                
        except:
            error_dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 
            "Cannot execute query %s, the error was\n%s\n%s\n%s" % (lcnquery, sys.exc_type, sys.exc_value, traceback.format_tb(sys.exc_traceback)))
            if error_dialog.run():
                error_dialog.hide()
            
        if lcnquery is not None and len(lcnquery)>0:
            hits = None
            try:
                hits = self._searcher.search(lcnquery)
            except:
                error_dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 
                "Cannot execute query %s, the error was\n%s\n%s\n%s" % (lcnquery, sys.exc_type, sys.exc_value, traceback.format_tb(sys.exc_traceback)))
                if error_dialog.run():
                    error_dialog.hide()
                
            if hits is not None:
                try:
                    self._createResultTree(hits)
                except:
                    error_dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 
                    "Cannot create result tree, the error was\n%s\n%s\n%s" % (sys.exc_type, sys.exc_value, traceback.format_tb(sys.exc_traceback)))
                    if error_dialog.run():
                        error_dialog.hide()
                                    
    
    #
    #
    #
    def _keyPressedTextview(self, widget, event, data):
        if event.keyval == gtk.keysyms.F9:
            self._executeQuery(widget, None)     


    #
    #
    # Zeigt Ergebnis-Tree
    def _createResultTree(self, hits):
        try:
            for column in self.treeview.get_columns():
                self.treeview.remove_column(column)
            self.treestore = gtk.TreeStore( str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str,
                                            str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str, str  ) 
            self.treeview.set_model(self.treestore)

            i = 0
            j = 1
            iter0 = self.treestore.append(None)
            fieldslist = []
            self.resultlabel.set_text("Query hits: %d" % hits.length)

            renderer = gtk.CellRendererText()
            column = gtk.TreeViewColumn("rownum", renderer, markup=0)
            column.set_resizable(True)
            column.set_spacing(2)
            column.set_sizing(gtk.TREE_VIEW_COLUMN_GROW_ONLY)
            self.treeview.append_column(column)
            
            while i < hits.length and i<self.rownumspinner.get_value():
                doc = hits.doc(i)
                if i==0:
                    for field in doc.list_fields():
                        renderer = gtk.CellRendererText()
                        column = gtk.TreeViewColumn(field.replace('_', '__'), renderer, text=j)
                        column.set_resizable(True)
                        column.set_spacing(2)
                        column.set_sizing(gtk.TREE_VIEW_COLUMN_GROW_ONLY)
                        self.treeview.append_column(column)
                        j = j+1
                        fieldslist.append(field)
                else:
                    for field in doc.list_fields():
                        try:
                            fieldindex = fieldslist.index(field)
                        except:
                            renderer = gtk.CellRendererText()
                            column = gtk.TreeViewColumn(field.replace('_', '__'), renderer, text=j)
                            column.set_resizable(True)
                            column.set_spacing(2)
                            column.set_sizing(gtk.TREE_VIEW_COLUMN_GROW_ONLY)
                            self.treeview.append_column(column)
                            j=j+1
                            fieldslist.append(field)
                        
                self.treestore.set_value(iter0, 0, "<span color='gray'>%d</span>" % (i+1))
                for field in doc.list_fields():
                    value = ""
                    try:                    
                        value = doc.field_value(field)

                    except:
                        value = "<not readable>"
                    self.treestore.set_value(iter0, (fieldslist.index(field)+1), ("%s" % value).decode("ISO-8859-1"))                    
                iter0 = self.treestore.append(None)
                i = i+1
            self.treeview.show_all()
        except Exception, ex:    
            print "Exception :"
            print ex

    #
    #
    # Erstellt neues Tab-Label
    def _queryTabLabel(self, position):
        ''' create tablabel for query tab '''        
        tablabel = gtk.HBox()
        hbox = gtk.HBox()
                      
        ebox = gtk.EventBox()
        ebox.set_visible_window(False)        
        self.tooltips.set_tip(ebox, self.indexdir)
        hbox.add(ebox)
        
        ebox.add(tablabel)
        
        tabclosebutton = gtk.Button()
        tabclosebutton.set_relief(gtk.RELIEF_NONE)
        self.tooltips.set_tip(tabclosebutton, "Close tab")        
        image = gtk.image_new_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_MENU)
        image.set_size_request(9, 9)
        tabclosebutton.add(image)
        if len(self.sessname)>15:
            labeltext = self.sessname[:12]+"..."
        else:
            labeltext = self.sessname

        label = gtk.Label(str=("%s" % (labeltext)))
        tablabel.pack_start(label, True, False, 0)
        tablabel.pack_start(tabclosebutton, False, False, 0)

        hbox.show_all()        
        tabclosebutton.connect("clicked", self._queryTabClose, position)
        
        return hbox

    #
    #
    # Schliesst das Tab
    def _queryTabClose(self, widget, data):
        ''' close query tab '''
        i = data
        if i>=0:
            query_notebook = self.wTree.get_widget("session_notebook")
            query_notebook.remove_page(i) 
        else:
            self.errorcb("Unexpected error: cannot close tab")        



