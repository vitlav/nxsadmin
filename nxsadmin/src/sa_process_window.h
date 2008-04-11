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

#ifndef _SA_PROCESS_WINDOW_H
#define	_SA_PROCESS_WINDOW_H

#include <gtkmm.h>

class MyProcessWindow : public Gtk::Window
{
    
public:
    
    MyProcessWindow();
    virtual ~MyProcessWindow();
    
    bool createProcessesList(std::string & aUserName);
    
protected:
    
    // Signal handlers
    virtual void onButtonRefreshClicked();
    virtual void onButtonCloseClicked();
    virtual void onButtonKillClicked();
    
protected:
    
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        
    public:
        
        ModelColumns()
        {
            add(theSelect);
            add(theUSER);
            add(thePID);
            add(theCPU);
            add(theMEM);
            add(theVSZ);
            add(theRSS);
            add(theTTY);
            add(theSTAT);
            add(theSTART);
            add(theTIME);
            add(theCOMMAND);
        }
        
        Gtk::TreeModelColumn<bool> theSelect;
        Gtk::TreeModelColumn<Glib::ustring> theUSER;
        Gtk::TreeModelColumn<Glib::ustring> thePID;
        Gtk::TreeModelColumn<Glib::ustring> theCPU;
        Gtk::TreeModelColumn<Glib::ustring> theMEM;
        Gtk::TreeModelColumn<Glib::ustring> theVSZ;
        Gtk::TreeModelColumn<Glib::ustring> theRSS;
        Gtk::TreeModelColumn<Glib::ustring> theTTY;
        Gtk::TreeModelColumn<Glib::ustring> theSTAT;
        Gtk::TreeModelColumn<Glib::ustring> theSTART;
        Gtk::TreeModelColumn<Glib::ustring> theTIME;
        Gtk::TreeModelColumn<Glib::ustring> theCOMMAND;
    };
    
    void createTreeView();
    
protected:
    
    std::string theUserName;
    
    ModelColumns theColumns;
    
    Gtk::VBox * theVBox;
    Gtk::ScrolledWindow * theScrolledWindow;
    Gtk::HButtonBox * theHButtonBox;
    Gtk::Button * theButtonRefresh;
    Gtk::Button * theButtonClose;
    Gtk::Button * theButtonKill;
    
    Gtk::TreeView * theTreeView;
    Glib::RefPtr<Gtk::ListStore> theTreeModel;
    Glib::RefPtr<Gtk::TreeModelSort> theTreeModelSort;
};

#endif	/* _SA_PROCESS_WINDOW_H */

