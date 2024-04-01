/* Treecle
 *
 * Copyright (c) Kartik Patel
 * E-mail: letapk@gmail.com
 * Download from: https://letapk.wordpress.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "treecle.h"

QString userpath;
QString Lockfilename;

void check_qtdata_dir ();
bool check_lockfile(void);
void create_lockfile ();
void delete_lockfile ();

int main(int argc, char *argv[])
//start user-interface
{
bool ok = false;

    Q_INIT_RESOURCE(treecle);
    QApplication app(argc, argv);

    QTranslator appTranslator;
    appTranslator.load("treecle_" + QLocale::system().name(), qApp->applicationDirPath());
    app.installTranslator(&appTranslator);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), qApp->applicationDirPath());
    app.installTranslator(&qtTranslator);

    //get the path to the user's home directory
    userpath.clear();
    userpath.append (getenv ("HOME"));
    userpath.append("/.treecle");//home/{account-name}/.treecle

    Lockfilename.append(userpath);
    Lockfilename.append("/treelockfile.lck");

    ok = check_lockfile ();
    if (ok == false)//lockfile present, exit
        return 0;
    create_lockfile();

    //check for the tdj data directory and create it if required
    check_qtdata_dir();

    MainWindow mainwindow;
    mainwindow.setWindowTitle(QObject::tr("Treecle"));
    mainwindow.show();

    return app.exec();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
//set up the user-interface
{
    //initial window size
    this->setMinimumHeight(300);
    this->setMinimumWidth(700);

    //menu, and two toolbars
    setup_menu_and_toolbar();

    //splitter for the tree and editor
    splitter = new QSplitter(this);

    //Tree and HTML editor windows are within the splitter
    //tree window on the left
    tree = new QTreeWidget ();
    tree->setColumnCount(1);
    tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect (tree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(set_branch (QTreeWidgetItem *)));
    tree->setHeaderLabel(tr("Filename"));

    //HTML editor on right
    leafview = new QWebView ();
    leafview->page()->setContentEditable(true);
    leafview->page()->settings()->setDefaultTextEncoding("UTF-8");

    connect (leafview->page(), SIGNAL(contentsChanged()), this, SLOT(get_data_from_leaf()));
    connect(leafview->pageAction(QWebPage::ToggleBold), SIGNAL(changed()), SLOT(adjustActions()));

    splitter->addWidget(tree);
    splitter->addWidget(leafview);

    //status bar on the bottom
    statustext = new QLabel (this);
    statustext->setText(tr("Status messages appear here"));
    statustext->setFrameStyle(QFrame::Plain);
    statustext->setAlignment(Qt::AlignBottom);

    catflag = 1;

    //set the Homepath
    Homepath.append(userpath);
    //set the working directory to the data subdirectory
    QDir::setCurrent(Homepath);

    //help file to read
    Helpfilename.append (userpath);
    Helpfilename.append ("/treeclehelp.pdf");

    //"COPYING" file to read
    Gnugplfilename.append (userpath);
    Gnugplfilename.append ("/COPYING");

    //Currentfile name
    Currentfile.append("Noname.trc");

    //initial window size
    this->resize(990,630);

    //read the user's preferences
    readprefs();

    this->setFocus();
}

MainWindow::~MainWindow()
{
    delete_lockfile();
}

void MainWindow::quit()
{
    delete_lockfile();
    MainWindow::close();
}
