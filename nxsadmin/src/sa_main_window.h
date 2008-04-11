/***************************************************************************
 *   Copyright (C) 2007 by Maxim Stjazhkin                                 *
 *   maxt_t@drohobych.com.ua                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SA_MAIN_WINDOW_H
#define	_SA_MAIN_WINDOW_H

#include <string>
#include <map>
#include <gtkmm.h>
#include <gtkmm/aboutdialog.h>
#include <gtk/gtkaboutdialog.h>
#include "sa_process_window.h"

class MyMainWindow : public Gtk::Window
{
    
public:
    
    MyMainWindow();
    virtual ~MyMainWindow();
    
protected:
    
    // Signal handlers
    virtual void onMenuFileQuit();
    
    virtual void onMenuSessionSuspend();
    virtual void onMenuSessionSuspendAll();
    virtual void onMenuSessionTerminate();
    virtual void onMenuSessionTerminateAll();
    
    virtual void onMenuMessageSend();
    virtual void onMenuMessageSendToAll();
    
    virtual void onMenuViewRefreshList();
    virtual void onMenuViewProcesses();
    
    virtual void onMenuHelpAbout();
    
    virtual bool onTimer();
    
    virtual void onButtonClearLogClicked();
    
    virtual bool on_button_press_event(GdkEventButton * theEvent);
    // Signal handler for popup menu items:
    virtual void on_menu_file_popup_generic();
    
protected:
    
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        
    public:
        
        ModelColumns()
        {
            add(theSelect);
            add(theSessionName);
            add(theDisplay);
            add(theStatus);
            add(theClientAddress);
            add(theUserName);
            add(theType);
            add(theSessionId);
        }
        
        Gtk::TreeModelColumn<bool> theSelect;
        Gtk::TreeModelColumn<Glib::ustring> theSessionName;
        Gtk::TreeModelColumn<unsigned int> theDisplay;
        Gtk::TreeModelColumn<Glib::ustring> theStatus;
        Gtk::TreeModelColumn<Glib::ustring> theClientAddress;
        Gtk::TreeModelColumn<Glib::ustring> theUserName;
        Gtk::TreeModelColumn<Glib::ustring> theType;
        Gtk::TreeModelColumn<Glib::ustring> theSessionId;
    };
    
    struct SessionInfo
    {
        int startTime;
        int display;
        int sessionRootlessMode;
        int creationTime;
        Glib::ustring sessionName;
        Glib::ustring status;
        Glib::ustring foreignAddress;
        Glib::ustring type;
        Glib::ustring sessionId;
        Glib::ustring userName;
        Glib::ustring serverPid;
        Glib::ustring screenInfo;
        Glib::ustring geometry;
        Glib::ustring host;
        Glib::ustring sessionFileName;
    };
    
    typedef std::map<Glib::ustring, SessionInfo> SessionsMap;
    
protected:
    
    // Methods for internal use
    void createMenuAndToolbar();
    void createTreeView();
    void createAboutDialog();
    void createSendMessageDialog();
    void createLogPaned();
    
    bool readSessionFiles(const Glib::ustring & aPath);
    bool readSession(const Glib::ustring & aFileName, SessionsMap & aMap);
    void fillTreeModelView(const Glib::ustring & aPath);
    
    bool isNXServRun() const;
    
    // FIXME: Trouble with Glib::spawn_command_line_sync, then execute nxserver --send or nxserver --broadcast commands
    // Use the argument aMessageSend = true, then need nxserver --send
    // or nxserver --broadcast.
    // if aMessageSend still false, than nxsadmin not responding, after
    // execute nxserver --send or nxserver --broadcast commands
    // because freenx server wait response of users, that recieved a message
    void ExecLog(const std::string & aCommand, bool aMessageSend = false);
    
protected:
    
    ModelColumns theColumns;
    
    Gtk::AboutDialog * theAboutDialog;
    Gtk::VBox * theBox;
    Gtk::ScrolledWindow * theScrolledWindow;
    Gtk::Statusbar * theStatusBar;
    
    Gtk::TreeView * theTreeView;
    Glib::RefPtr<Gtk::ListStore> theTreeModel;
    Glib::RefPtr<Gtk::TreeModelSort> theTreeModelSort;
    
    Glib::RefPtr<Gtk::UIManager> theUIManager;
    Glib::RefPtr<Gtk::ActionGroup> theActionGroup;
    
    Gtk::VPaned * theVPaned;
    
    Gtk::ScrolledWindow * theScrolledWindow2;
    Gtk::TextView * theTextView;
    Glib::RefPtr<Gtk::TextBuffer> theTextBuffer;
    Gtk::HButtonBox * theHButtonBox;
    Gtk::Button * theButtonClearLog;
    
    SessionsMap theMap;
    
    static Glib::ustring theNXDbPath;
    
    gint theRunningNum;
    gint theSuspendedNum;
    
    Gtk::Entry * theMessageEntry;
    Gtk::Dialog * theMessageDialog;
    
    Gtk::Tooltips * theToolTips;
    
    Glib::RefPtr<Gtk::TextTag> theBoldFont;
    Glib::RefPtr<Gtk::TextTag> theNormalFont;
    Glib::RefPtr<Gtk::TextTag> theNormalRedFont;
    
    MyProcessWindow * theProcessWindow;
    
    Gtk::Menu * theMenuPopUp;
};

#endif	/* _SA_MAIN_WINDOW_H */
