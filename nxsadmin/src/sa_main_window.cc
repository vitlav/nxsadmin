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

#include <iostream>
#include <fstream>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glibmm/i18n.h>
#include "sa_sys_utils.h"
#include "sa_main_window.h"
//----------------------------------------------------------------------------
#define MAX_MESSAGE_LENGTH 1024
//----------------------------------------------------------------------------
Glib::ustring MyMainWindow::theNXDbPath("/var/lib/nxserver/db/running/");
//----------------------------------------------------------------------------
MyMainWindow::MyMainWindow()
{
    theRunningNum = theSuspendedNum = 0;
    this->set_default_size(700, 350);
    this->set_border_width(5);
    this->set_position(Gtk::WIN_POS_CENTER);
    this->set_title(_("FreeNX Sessions Administrator"));
    
    theStatusBar = Gtk::manage(new Gtk::Statusbar);
    theBox = Gtk::manage(new Gtk::VBox);
    
    this->add(*theBox);
    
    this->createAboutDialog();
    this->createSendMessageDialog();
    this->createMenuAndToolbar();
    this->createLogPaned();
    this->createTreeView();
    
    theBox->pack_start(*theVPaned, Gtk::PACK_EXPAND_WIDGET);
    theBox->pack_start(*theHButtonBox, Gtk::PACK_SHRINK, 5);
    theBox->pack_end(*theStatusBar, Gtk::PACK_SHRINK);
    
    // Refreshing TreeModelView - called once every 50ms
    // FIXME: Replace a timer based sessions list refreshing
    Glib::signal_timeout().connect(sigc::mem_fun(*this,
            &MyMainWindow::onTimer), 5000);
    
    theProcessWindow = new MyProcessWindow();
    
    // Create and fill popup menu
    theMenuPopUp = new Gtk::Menu();
    Gtk::Menu::MenuList & menulist = theMenuPopUp->items();
    menulist.push_back(Gtk::Menu_Helpers::MenuElem(_("_View processes list"),
            sigc::mem_fun(*this, &MyMainWindow::on_menu_file_popup_generic)));
    theMenuPopUp->accelerate(*this);
    theTreeView->signal_button_press_event().connect(sigc::mem_fun(*this,
            &MyMainWindow::on_button_press_event), false);
    
    this->show_all_children();
    
    this->ExecLog("nxserver --status");
}
//----------------------------------------------------------------------------
MyMainWindow::~MyMainWindow()
{
    delete theAboutDialog;
    delete theMessageEntry;
    delete theMessageDialog;
    delete theToolTips;
    delete theProcessWindow;
    delete theMenuPopUp;
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuFileQuit()
{
    // Closes the main window to stop the Gtk::Main::run()
    this->hide();
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuSessionSuspend()
{
    Glib::ustring tmpsess;
    
    Gtk::TreeModel::Children children = theTreeModel->children();
    
    for (Gtk::TreeModel::Children::iterator iter = children.begin(); iter
            != children.end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if ((row[theColumns.theSelect]) && (row[theColumns.theStatus]
                == "Running"))
        {
            tmpsess = row[theColumns.theSessionId];
            this->ExecLog("nxserver --suspend " + tmpsess);
        }
    }
    this->fillTreeModelView(theNXDbPath);
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuSessionSuspendAll()
{
    this->ExecLog("nxserver --suspend *");
    this->fillTreeModelView(theNXDbPath);
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuSessionTerminate()
{
    Glib::ustring tmpsess;
    
    Gtk::TreeModel::Children children = theTreeModel->children();
    
    for (Gtk::TreeModel::Children::iterator iter = children.begin(); iter
            != children.end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (row[theColumns.theSelect])
        {
            tmpsess = row[theColumns.theSessionId];
            this->ExecLog("nxserver --terminate " + tmpsess);
        }
    }
    this->fillTreeModelView(theNXDbPath);
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuSessionTerminateAll()
{
    this->ExecLog("nxserver --cleanup");
    this->fillTreeModelView(theNXDbPath);
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuMessageSend()
{
    Glib::ustring msg;
    Glib::ustring tmpsess;
    
    Gtk::TreeModel::Children children = theTreeModel->children();
    
    theMessageEntry->set_text("");
    theMessageEntry->show();
    
    gint result = theMessageDialog->run();
    
    if (result == Gtk::RESPONSE_CANCEL)
    {
        theMessageEntry->hide();
        theMessageDialog->hide();
    }
    
    if (result == Gtk::RESPONSE_OK)
    {
        msg = theMessageEntry->get_text();
        
        theMessageEntry->hide();
        theMessageDialog->hide();
        
        gint count = strlen(msg.c_str());
        
        if (count <= 0)
        {
            return;
        }
        
        // FIXME: Fix the error message
        if (count > MAX_MESSAGE_LENGTH)
        {
            Gtk::MessageDialog
                    dialog(*this, "ERROR", false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text(_("The message length more than 1024 symbols!!"));
            dialog.run();
            return;
        }
        for (Gtk::TreeModel::Children::iterator iter = children.begin(); iter
                != children.end(); ++iter)
        {
            Gtk::TreeModel::Row row = *iter;
            
            if (row[theColumns.theSelect])
            {
                tmpsess = row[theColumns.theSessionId];
                this->ExecLog("nxserver --send " + tmpsess + " " + msg, true);
            }
        }
    }
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuMessageSendToAll()
{
    Glib::ustring msg;
    
    theMessageEntry->set_text("");
    theMessageEntry->show();
    
    gint result = theMessageDialog->run();
    
    if (result == Gtk::RESPONSE_CANCEL)
    {
        theMessageEntry->hide();
        theMessageDialog->hide();
    }
    
    if (result == Gtk::RESPONSE_OK)
    {
        msg = theMessageEntry->get_text();
        theMessageEntry->hide();
        theMessageDialog->hide();
        
        gint count = strlen(msg.c_str());
        
        if (count <= 0)
        {
            return;
        }
        
        // FIXME: Fix the error message
        if (count > MAX_MESSAGE_LENGTH)
        {
            Gtk::MessageDialog
                    dialog(*this, "ERROR", false, Gtk::MESSAGE_ERROR);
            dialog.set_secondary_text(_("The message length more than 1024 symbols!!"));
            dialog.run();
            return;
        }
        this->ExecLog("nxserver --broadcast " + msg, true);
    }
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuViewRefreshList()
{
    this->fillTreeModelView(theNXDbPath);
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuHelpAbout()
{
    gint result = theAboutDialog->run();
    theAboutDialog->present();
    
    if (result == Gtk::RESPONSE_CANCEL)
    {
        theAboutDialog->hide();
    }
}
//----------------------------------------------------------------------------
void MyMainWindow::createMenuAndToolbar()
{
    theActionGroup = Gtk::ActionGroup::create();
    
    // File menu
    theActionGroup->add(Gtk::Action::create("FileMenu", _("File")));
    theActionGroup->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT),
            sigc::mem_fun(*this, &MyMainWindow::onMenuFileQuit));
    
    // Session menu
    theActionGroup->add(Gtk::Action::create("SessionMenu", _("Session")));
    theActionGroup->add(Gtk::Action::create("SessionSuspend",
            Gtk::Stock::MEDIA_PAUSE, _("Suspend session")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuSessionSuspend));
    theActionGroup->add(Gtk::Action::create("SessionTerminate",
            Gtk::Stock::CLOSE, _("Terminate session")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuSessionTerminate));
    theActionGroup->add(Gtk::Action::create("SessionSuspendAll",
            Gtk::Stock::CANCEL, _("Suspend all sessions")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuSessionSuspendAll));
    theActionGroup->add(Gtk::Action::create("SessionTerminateAll",
            Gtk::Stock::STOP, _("Terminate all sessions")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuSessionTerminateAll));
    
    // Message menu
    theActionGroup->add(Gtk::Action::create("MessageMenu", _("Message")));
    theActionGroup->add(Gtk::Action::create("MessageSend", _("Send")),
            sigc::mem_fun(*this, &MyMainWindow::onMenuMessageSend));
    theActionGroup->add(Gtk::Action::create("MessageSendToAll", _("Send to all")),
            sigc::mem_fun(*this, &MyMainWindow::onMenuMessageSendToAll));
    
    // View menu
    theActionGroup->add(Gtk::Action::create("ViewMenu", _("View")));
    theActionGroup->add(Gtk::Action::create("ViewRefresh", Gtk::Stock::REFRESH,
            _("Refresh sessions list")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuViewRefreshList));
    theActionGroup->add(Gtk::Action::create("ViewProcesses", Gtk::Stock::ZOOM_IN,
            _("View processes list")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuViewProcesses));
    
    // Help menu
    theActionGroup->add(Gtk::Action::create("HelpMenu", _("Help")));
    theActionGroup->add(Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT,
            _("About NX session administrator")), sigc::mem_fun(*this,
            &MyMainWindow::onMenuHelpAbout));
    
    theUIManager = Gtk::UIManager::create();
    theUIManager->insert_action_group(theActionGroup);
    
    // Layout the actions in a menubar and toolbar:
    Glib::ustring uiInfo = "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='FileMenu'>"
            "      <menuitem action='FileQuit'/>"
            "    </menu>"
            "    <menu action='SessionMenu'>"
            "      <menuitem action='SessionSuspend'/>"
            "      <menuitem action='SessionTerminate'/>"
            "	   <separator/>"
            "      <menuitem action='SessionSuspendAll'/>"
            "      <menuitem action='SessionTerminateAll'/>"
            "    </menu>"
            "    <menu action='MessageMenu'>"
            "       <menuitem action='MessageSend'/>"
            "       <menuitem action='MessageSendToAll'/>"
            "    </menu>"
            "    <menu action='ViewMenu'>"
            "      <menuitem action='ViewRefresh'/>"
            "      <menuitem action='ViewProcesses'/>"
            "    </menu>"
            "    <menu action='HelpMenu'>"
            "      <menuitem action='HelpAbout'/>"
            "    </menu>"
            "  </menubar>"
            "  <toolbar name='ToolBar'>"
            "    <toolitem action='FileQuit'/>"
            "    <separator/>"
            "    <toolitem action='SessionSuspend'/>"
            "    <toolitem action='SessionTerminate'/>"
            "    <separator/>"
            "    <toolitem action='SessionSuspendAll'/>"
            "    <toolitem action='SessionTerminateAll'/>"
            "    <separator/>"
            "    <toolitem action='ViewRefresh'/>"
            "  </toolbar>"
            "</ui>";
    try
    {
        theUIManager->add_ui_from_string(uiInfo);
    }
    catch(const Glib::Error & er)
    {
        std::cerr << "building menus failed: " << er.what();
    }
    
    //Get the menubar and toolbar widgets, and add them to a container widget:
    Gtk::Widget * menuBar = theUIManager->get_widget("/MenuBar");
    if (menuBar)
        theBox->pack_start(*menuBar, Gtk::PACK_SHRINK);
    
    Gtk::Widget * toolBar = theUIManager->get_widget("/ToolBar");
    if (toolBar)
        theBox->pack_start(*toolBar, Gtk::PACK_SHRINK);
    
    // Create tooltips for toolbar widgets
    theToolTips = new Gtk::Tooltips();
    Gtk::Widget * toolItem = theUIManager->get_widget("/ToolBar/FileQuit");
    if (toolItem)
        theToolTips->set_tip(*toolItem, _("Quit from program"));
    
    toolItem = theUIManager->get_widget("/ToolBar/SessionSuspend");
    if (toolItem)
        theToolTips->set_tip(*toolItem, _("Suspend the session(s)"));
    
    toolItem = theUIManager->get_widget("/ToolBar/SessionTerminate");
    if (toolItem)
        theToolTips->set_tip(*toolItem, _("Terminate session(s)"));
    
    toolItem = theUIManager->get_widget("/ToolBar/SessionSuspendAll");
    if (toolItem)
        theToolTips->set_tip(*toolItem, _("Suspend all sessions"));
    
    toolItem = theUIManager->get_widget("/ToolBar/SessionTerminateAll");
    if (toolItem)
        theToolTips->set_tip(*toolItem, _("Terminate all sessions"));
    
    toolItem = theUIManager->get_widget("/ToolBar/ViewRefresh");
    if (toolItem)
        theToolTips->set_tip(*toolItem, _("Refresh sessions list"));
    
    theToolTips->enable();
}
//----------------------------------------------------------------------------
void MyMainWindow::createTreeView()
{
    theTreeView = Gtk::manage(new Gtk::TreeView);
    theScrolledWindow = Gtk::manage(new Gtk::ScrolledWindow);
    
    // Add the TreeView, inside a ScrolledWindow
    theScrolledWindow->add(*theTreeView);
    theScrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    theScrolledWindow->set_shadow_type(Gtk::SHADOW_OUT);
    // XXX: theBox->pack_start(*theScrolledWindow);
    theVPaned->pack1(*theScrolledWindow, Gtk::FILL);
    
    // Create the Tree model:
    theTreeModel = Gtk::ListStore::create(theColumns);
    theTreeModelSort = Gtk::TreeModelSort::create(theTreeModel);
    
    theTreeView->set_model(theTreeModelSort);
    
    // Fill the TreeView's model
    this->fillTreeModelView(theNXDbPath);
    
    // Add the TreeView's view columns:
    theTreeView->append_column_editable(_("Select"), theColumns.theSelect);
    theTreeView->append_column(_("User"), theColumns.theUserName);
    theTreeView->append_column(_("Session"), theColumns.theSessionName);
    theTreeView->append_column(_("Display"), theColumns.theDisplay);
    theTreeView->append_column(_("Address"), theColumns.theClientAddress);
    theTreeView->append_column(_("Type"), theColumns.theType);
    theTreeView->append_column(_("Status"), theColumns.theStatus);
    theTreeView->append_column(_("Session Id"), theColumns.theSessionId);
    
    Gtk::TreeView::Column * column = theTreeView->get_column(0);
    if (column)
        column->set_sort_column(theColumns.theSelect);
    
    column = theTreeView->get_column(1);
    if (column)
        column->set_sort_column(theColumns.theUserName);
    
    column = theTreeView->get_column(2);
    if (column)
        column->set_sort_column(theColumns.theSessionName);
    
    column = theTreeView->get_column(3);
    if (column)
        column->set_sort_column(theColumns.theDisplay);
    
    column = theTreeView->get_column(4);
    if (column)
        column->set_sort_column(theColumns.theClientAddress);
    
    column = theTreeView->get_column(5);
    if (column)
        column->set_sort_column(theColumns.theType);
    
    column = theTreeView->get_column(6);
    if (column)
        column->set_sort_column(theColumns.theStatus);
    
    column = theTreeView->get_column(7);
    if (column)
        column->set_sort_column(theColumns.theSessionId);
    
    for (guint i = 0; i < 8; ++i)
    {
        column = theTreeView->get_column(i);
        column->set_alignment(Gtk::ALIGN_CENTER);
        column->set_resizable(true);
    }
}
//----------------------------------------------------------------------------
void MyMainWindow::createAboutDialog()
{
    theAboutDialog = new Gtk::AboutDialog();
    theAboutDialog->set_name(_("FreeNX Sessions Administrator"));
    theAboutDialog->set_version("0.2.1");
    theAboutDialog->set_copyright(_("(C) 2008 by Maxim Stjazhkin"));
    theAboutDialog->set_comments(_("FreeNX Sessions Administrator provides a graphical tool for managment of active NX sessions on FreeNX server"));
    theAboutDialog->set_license("GPL");
    theAboutDialog->set_website("http://cgroup.drohobych.com.ua");
    theAboutDialog->set_website_label("cgroup.drohobych.com.ua");
    std::list<Glib::ustring> list_autors;
    list_autors.push_back(_("Maxim Stjazhkin, maxt_t@drohobych.com.ua"));
    theAboutDialog->set_authors(list_autors);
}
//----------------------------------------------------------------------------
void MyMainWindow::createSendMessageDialog()
{
    theMessageEntry = new Gtk::Entry();
    theMessageEntry->set_text("");
    theMessageEntry->set_editable(true);
    theMessageEntry->set_activates_default(true);
    theMessageEntry->set_max_length(MAX_MESSAGE_LENGTH);
    
    theMessageDialog = new Gtk::Dialog();
    theMessageDialog->set_default_size(400, 100);
    theMessageDialog->set_title(_("Send message"));
    theMessageDialog->set_position(Gtk::WIN_POS_CENTER);
    Gtk::VBox * vbox = theMessageDialog->get_vbox();
    vbox->pack_start(*theMessageEntry, Gtk::PACK_SHRINK);
    theMessageDialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    theMessageDialog->add_button(_("Send"), Gtk::RESPONSE_OK);
    theMessageDialog->set_default_response(Gtk::RESPONSE_OK);
}
//----------------------------------------------------------------------------
void MyMainWindow::createLogPaned()
{
    theVPaned = Gtk::manage(new Gtk::VPaned);
    this->get_size(theRunningNum, theSuspendedNum);
    theVPaned->set_position(theSuspendedNum / 2);
    
    theTextBuffer = Gtk::TextBuffer::create();
    
    theTextView = Gtk::manage(new Gtk::TextView);
    theTextView->set_buffer(theTextBuffer);
    theTextView->set_editable(false);
    
    theScrolledWindow2 = Gtk::manage(new Gtk::ScrolledWindow);
    theScrolledWindow2->add(*theTextView);
    theScrolledWindow2->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    theScrolledWindow2->set_shadow_type(Gtk::SHADOW_OUT);
    
    theButtonClearLog = Gtk::manage(new Gtk::Button(_("Clear log")));
    theButtonClearLog->signal_clicked().connect(sigc::mem_fun(*this,
            &MyMainWindow::onButtonClearLogClicked));
    theHButtonBox = Gtk::manage(new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 5));
    theHButtonBox->pack_start(*theButtonClearLog, Gtk::PACK_SHRINK, 5);
    
    // Preparing tags for theTextBuffer
    theBoldFont = theTextBuffer->create_tag("bold_ft");
    theBoldFont->property_weight() = Pango::WEIGHT_BOLD;
    theBoldFont->property_foreground() = "#3333FF";
    theNormalFont = theTextBuffer->create_tag("normal_ft");
    theNormalFont->property_weight() = Pango::WEIGHT_NORMAL;
    theNormalRedFont = theTextBuffer->create_tag("normal_red_ft");
    theNormalRedFont->property_weight() = Pango::WEIGHT_NORMAL;
    theNormalRedFont->property_foreground() = "#FF0000";
    
    theVPaned->pack2(*theScrolledWindow2, Gtk::EXPAND);
    
    theRunningNum = theSuspendedNum = 0;
}
//----------------------------------------------------------------------------
bool MyMainWindow::readSessionFiles(const Glib::ustring & aPath)
{
    int statErr;
    DIR * dir;
    struct dirent * ent;
    struct stat buf;
    Glib::ustring tempPath;
    
    if (!(dir = opendir(aPath.c_str())))
    {
        perror("opendir");
        return false;
    }
    
    errno = 0;
    
    while ((ent = readdir(dir)))
    {
        tempPath = aPath + (ent->d_name);
        
        statErr = stat(tempPath.c_str(), &buf);
        
        if (statErr != 0)
        {
            perror("stat");
            return false;
        }
        
        if (S_ISREG((buf.st_mode & S_IFMT)))
        {
            this->readSession(tempPath, theMap);
        }
        
        errno = 0;
    }
    
    if (errno)
    {
        perror("readdir");
        return false;
    }
    
    closedir(dir);
    
    return true;
}
//----------------------------------------------------------------------------
bool MyMainWindow::readSession(const Glib::ustring & aFileName,
        SessionsMap & aMap)
{
    SessionInfo tmpSession;
    std::ifstream file;
    gchar buf[255];
    
    Glib::ustring tmp;
    Glib::ustring option;
    Glib::ustring optionName;
    
    file.open(aFileName.c_str(), std::ios::in);
    
    if (!file)
    {
        return false;
    }
    
    tmpSession.sessionFileName = aFileName;
    
    while ((!file.eof()) && (!file.fail()))
    {
        file.getline(buf, sizeof(buf));
        tmp = buf;
        option = tmp.substr((tmp.find("=") + 1), tmp.size());
        optionName = tmp.substr(0, tmp.find("="));
        
        if (optionName == "sessionName")
        {
            tmpSession.sessionName = option;
        }
        else if (optionName == "display")
        {
            tmpSession.display = atoi(option.c_str());
        }
        else if (optionName == "status")
        {
            tmpSession.status = option;
        }
        else if (optionName == "startTime")
        {
            tmpSession.startTime = atoi(option.c_str());
        }
        else if (optionName == "foreignAddress")
        {
            tmpSession.foreignAddress = option;
        }
        else if (optionName == "sessionRootlessMode")
        {
            tmpSession.sessionRootlessMode = atoi(option.c_str());
        }
        else if (optionName == "type")
        {
            tmpSession.type = option;
        }
        else if (optionName == "sessionId")
        {
            tmpSession.sessionId = option;
        }
        else if (optionName == "creationTime")
        {
            tmpSession.creationTime = atoi(option.c_str());
        }
        else if (optionName == "userName")
        {
            tmpSession.userName = option;
        }
        else if (optionName == "serverPid")
        {
            tmpSession.serverPid = option;
        }
        else if (optionName == "screeninfo")
        {
            tmpSession.screenInfo = option;
        }
        else if (optionName == "geometry")
        {
            tmpSession.geometry = option;
        }
        else if (optionName == "geometry")
        {
            tmpSession.geometry = option;
        }
        else if (optionName == "host")
        {
            tmpSession.host = option;
        }
    }
    file.close();
    aMap[tmpSession.sessionId] = tmpSession;
    return true;
}
//----------------------------------------------------------------------------
void MyMainWindow::fillTreeModelView(const Glib::ustring & aPath)
{
    typedef Gtk::TreeModel::Children TChildren;
    TChildren children = theTreeModel->children();
    
    Gtk::TreeModel::Row row;
    SessionInfo sinfo;
    
    theRunningNum = theSuspendedNum = 0;
    gchar buff[200];
    
    TChildren::iterator ci = children.begin();
    
    theMap.clear();
    
    this->readSessionFiles(aPath);
    
    for (SessionsMap::iterator i = theMap.begin(); i != theMap.end(); ++i)
    {
        sinfo = (*i).second;
        
        if (ci != children.end())
        {
            row = *ci;
            ci++;
        }
        else
        {
            row = *(theTreeModel->append());
            row[theColumns.theSelect] = false;
        }
        row[theColumns.theSessionName] = sinfo.sessionName;
        row[theColumns.theDisplay] = sinfo.display;
        row[theColumns.theStatus] = sinfo.status;
        row[theColumns.theClientAddress] = sinfo.foreignAddress;
        row[theColumns.theUserName] = sinfo.userName;
        row[theColumns.theType] = sinfo.type;
        
        if ((row[theColumns.theSessionId] != sinfo.sessionId))
        {
            row[theColumns.theSelect] = false;
        }
        row[theColumns.theSessionId] = sinfo.sessionId;
        
        if (row[theColumns.theStatus] == "Running")
        {
            ++theRunningNum;
        }
        else if (row[theColumns.theStatus] == "Suspended")
        {
            ++theSuspendedNum;
        }
    }
    
    if ((children.size()) > (theMap.size()))
    {
        while (ci != children.end())
        {
            ci = theTreeModel->erase(ci);
        }
    }
    
    theStatusBar->pop(0);
    g_snprintf(buff, 200, _("Sessions: all - %d  running - %d  suspended - %d"),
            children.size(), theRunningNum, theSuspendedNum);
    theStatusBar->push(buff, 0);
}
//----------------------------------------------------------------------------
bool MyMainWindow::onTimer()
{
    this->fillTreeModelView(theNXDbPath);
    return true;
}
//----------------------------------------------------------------------------
void MyMainWindow::onButtonClearLogClicked()
{
    theTextBuffer->set_text("");
}
//----------------------------------------------------------------------------
bool MyMainWindow::isNXServRun() const
{
    std::string stdmsg;
    std::string cmd("nxserver --status");
    
    Glib::spawn_command_line_sync(cmd, &stdmsg, NULL, NULL);
    
    gint result = stdmsg.find("running");
    
    if (result > 0)
    {
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
void MyMainWindow::ExecLog(const std::string & aCommand, bool aMessageSend)
{
    gint result;
    std::string stdmsg;
    std::string stderr;
    char * time = new char[40];
    
    System::getTimeStr(time);
    std::string timestr(time);
    
    Gtk::TextIter it = theTextBuffer->end();
    theTextBuffer->insert_with_tag(it, timestr + '\n', theBoldFont);
    
    if (aMessageSend)
    {
        try
        {
            Glib::spawn_command_line_async(aCommand);
        }
        catch (Glib::Exception & e)
        {
            stderr = static_cast<Glib::ustring>(e.what());
            it = theTextBuffer->end();
            theTextBuffer->insert_with_tag(it, stderr, theNormalRedFont);
        }
    }
    else
    {
        try
        {
            Glib::spawn_command_line_sync(aCommand, &stdmsg, &stderr, &result);
        }
        catch (Glib::Exception & e)
        {
            it = theTextBuffer->end();
            theTextBuffer->insert_with_tag(it, stderr, theNormalRedFont);
        }
        it = theTextBuffer->end();
        theTextBuffer->insert_with_tag(it, stdmsg, theNormalFont);
        
        it = theTextBuffer->end();
        theTextBuffer->insert_with_tag(it, stderr, theNormalRedFont);
        
        it = theTextBuffer->end();
        theTextView->scroll_to(it);
    }
    delete [] time;
}
//----------------------------------------------------------------------------
bool MyMainWindow::on_button_press_event(GdkEventButton * theEvent)
{
    if ((theEvent->type == GDK_BUTTON_PRESS) && (theEvent->button == 3) )
    {
        if (theMenuPopUp)
        {
            theMenuPopUp->popup(theEvent->button, theEvent->time);
            return true; 
        }
    }
    return false;
}
//----------------------------------------------------------------------------
void MyMainWindow::on_menu_file_popup_generic()
{
    Glib::RefPtr<Gtk::TreeView::Selection> selection =
            theTreeView->get_selection();    
    if (selection)
    {
        Gtk::TreeModel::iterator it = selection->get_selected();
        if (it)
        {
            Glib::ustring str = (*it)[theColumns.theUserName];
            std::string str2 = str;
            theProcessWindow->createProcessesList(str2);
            theProcessWindow->show();
        }
    }
}
//----------------------------------------------------------------------------
void MyMainWindow::onMenuViewProcesses()
{
    this->on_menu_file_popup_generic();
}
