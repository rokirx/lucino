#!/usr/bin/env python

#
# Released under GPL
# $Id$
# Valerij Klein <vklein@users.sourceforge.net>

import os
import sys
import time
import traceback

# Import GTK and Gnome stuff
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
    print "cannot import gtk, please check your installation"
    sys.exit(1)

import gnome.ui

# Import lucino browser deps
from misc import ClbConst
import clbworker as clbworker

# Import i18n stuff
import i18n

class ClbGUI:
    '''
    # main gui
    '''

    def __init__(self, guyumdir, gladefile):
        ''' constructor '''        
        self.gladefile=gladefile
        self.clbdir=guyumdir
        self.setupGUI()
        self.sessions = []
        self.indexdir = None
        self.sessname = None
       
       
    #
    #
    # GUI setup function
    def setupGUI(self):
    
        # read gladefile
        try:
            self.wTree = gtk.glade.XML( self.gladefile, None, ClbConst.APPNAME )
        except Exception, ex:
            print "Cannot load glade-File %s, exit" % self.gladefile  
            traceback.print_exc(file=sys.stderr)
            sys.exit(3)
        
        #Create our dictionary and connect it
        try:
            dic = { "on_quit_menuitem_activate" : self.quitall,
                    "on_about2_activate" : self.showAboutDialog,
                    "on_menuitem_about_activate" : self.showAboutDialog,                    
                    "on_new_menuitem_activate" : self.showNewSessDialog,
                    "on_dlg_sessinit_response" : self.createNewSession,
                    "on_mainwindow_delete_event" : self.quitall2,
                    "on_mainwindow_destroy" : self.quitall2,
                    "on_dlg_sessinit_btn_open_clicked" : self.openIndexDirDialog,
                    "on_choose_indexdir_response" : self.setNewIndexDir,
                    "on_session_notebook_switch_page" : self.switchSearchPage
                     }
            self.wTree.signal_autoconnect(dic)
                    
            self.window = self.wTree.get_widget("mainwindow")
            (self.wTree.get_widget("session_notebook")).remove_page(0)
        except Exception, ex:
            print "Cannot initialize GUI, exit"
            traceback.print_exc(file=sys.stderr)            
            sys.exit(4)
                     
    #
    #
    # About Dialog
    def showAboutDialog(self, data):
        aboutdialog = self.wTree.get_widget("aboutdialog")        
        aboutdialog.run()
        aboutdialog.hide()
        
    #
    #
    # "Neue Session" - Dialog anzeigen
    def showNewSessDialog(self, data):
        newsessdialog = self.wTree.get_widget("dlg_sessinit")
        sessnameentry = self.wTree.get_widget("dlg_sessinit_sessname_entry")
        sessnameentry.set_text("session%d" % (len(self.sessions)))
        newsessdialog.show()
        
    #
    #
    # Rueckgabe des Dialogs "Neue Session" verarbeiten
    def createNewSession(self, dialog, i):
        if i == gtk.RESPONSE_OK:
            indexdir = (self.wTree.get_widget("dlg_sessinit_indexdir_entry")).get_text()
            if indexdir is None or len(indexdir)==0:
                error_dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 
                "Please specify index directory")
                if error_dialog.run():
                    error_dialog.hide()
            else:
                sessname = (self.wTree.get_widget("dlg_sessinit_sessname_entry")).get_text()
                if sessname is None or len(sessname)==0:
                    sessname = indexdir
                try:
                    session = clbworker.ClbWorkerThread(self.wTree, "./", indexdir, sessname, len(self.sessions))
                    self.sessions.append(session)
                    dialog.hide()                    
                except Exception, ex:
                    error_dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_CLOSE, 
                    "Cannot create new searcher, the error was\n%s\n%s\n%s" % (sys.exc_type, sys.exc_value, traceback.format_tb(sys.exc_traceback)))
                    if error_dialog.run():
                        error_dialog.hide()
                    
                
        
    #
    #
    # Index-Verzeichnis auf
    def openIndexDirDialog(self, data):
        fileopendialog = self.wTree.get_widget("choose_indexdir")
        fileopendialog.show()
     
        
    #
    #
    # Setzt neuen Index-Verzeichnis
    def setNewIndexDir(self, file_chooser, i):
        if i == gtk.RESPONSE_OK:
            indexdirentry = self.wTree.get_widget("dlg_sessinit_indexdir_entry")
            indexdirentry.set_text(file_chooser.get_filename())            
        file_chooser.hide()

        
    #
    #
    # quit
    def quitall(self, data):
        gtk.main_quit()

        
    #
    #
    # quit
    def quitall2(self, widget, data=None):
        self.quitall(data)
        
    #
    #
    #
    def switchSearchPage(self, widget, data, page_num):
        if len(self.sessions)>0:
            try:
                recentclbw = self.sessions[page_num]
                (self.wTree.get_widget("indexdir_label")).set_text(recentclbw.indexdir)
            except:
                print "error setting indexdir?"

