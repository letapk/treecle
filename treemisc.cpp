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

//Last modified Sept 11, 2026

#include "treecle.h"

#include <QLockFile>
#include <QEvent>
#include <QRegularExpression>
#include <QImage>
#include <QImageReader>
#include <QTextBlock>
#include <QTextFragment>

//Much of this is shamelessly copied and hacked from the examples within the Qt software
void MainWindow::setup_menu_and_toolbar ()
{

    //file menu
    filemenu = menuBar()->addMenu(tr("&File"));

    QIcon newIcon = QIcon::fromTheme("", QIcon(":/images/filenew.png"));
    QAction *newfile = new QAction(tr("&New file"), this);
    filemenu->addAction(newfile);
    newfile->setIcon(newIcon);
    newfile->setShortcut(QKeySequence::New);
    connect(newfile, &QAction::triggered, this, &MainWindow::new_file);

    QIcon openIcon = QIcon::fromTheme("", QIcon(":/images/fileopen.png"));
    QAction *openfile = new QAction(tr("&Open file"), this);
    filemenu->addAction(openfile);
    openfile->setIcon(openIcon);
    openfile->setShortcut(QKeySequence::Open);
    connect(openfile, &QAction::triggered, this, &MainWindow::open_file);

    QIcon saveIcon = QIcon::fromTheme("", QIcon(":/images/filesave.png"));
    QAction *savefile = new QAction(tr("&Save file"), this);
    filemenu->addAction(savefile);
    savefile->setIcon(saveIcon);
    savefile->setShortcut(QKeySequence::Save);
    connect(savefile, &QAction::triggered, this, &MainWindow::save_file);

    QIcon saveIconas = QIcon::fromTheme("", QIcon(":/images/filesaveas.png"));
    QAction *savefileas = new QAction(tr("&Save file as..."), this);
    filemenu->addAction(savefileas);
    savefileas->setIcon(saveIconas);
    savefileas->setShortcut(QKeySequence::SaveAs);
    connect(savefileas, &QAction::triggered, this, &MainWindow::save_file_as);

    filemenu->addSeparator();

    QAction *quit = new QAction(tr("E&xit"), this);
    filemenu->addAction(quit);
    quit->setShortcut(QKeySequence::Quit);
    connect(quit, &QAction::triggered, this, &MainWindow::quit);

    //tree menu
    treemenu = menuBar()->addMenu(tr("&Tree"));

    QIcon addbrIcon = QIcon::fromTheme("", QIcon( ":/images/add-branch.png"));
    QAction *addbranch = new QAction(tr("&New Branch"), this);
    treemenu->addAction(addbranch);
    addbranch->setIcon(addbrIcon);
    connect(addbranch, &QAction::triggered, this, &MainWindow::tree_addbranch);

    QIcon addsubbrIcon = QIcon::fromTheme("", QIcon(":/images/add-subbranch.png"));
    QAction *addsubbranch = new QAction(tr("New &Sub-branch"), this);
    treemenu->addAction(addsubbranch);
    addsubbranch->setIcon(addsubbrIcon);
    connect(addsubbranch, &QAction::triggered, this, &MainWindow::tree_addsubbranch);

    treemenu->addSeparator();

    QIcon cutIcon = QIcon::fromTheme("", QIcon(":/images/editcut.png"));
    QAction *cutbranch = new QAction(tr("Cu&t branch"), this);
    treemenu->addAction(cutbranch);
    cutbranch->setIcon(cutIcon);
    connect(cutbranch, &QAction::triggered, this, &MainWindow::tree_cutbranch);

    QIcon copyIcon = QIcon::fromTheme("", QIcon(":/images/editcopy.png"));
    QAction *copybranch = new QAction(tr("&Copy branch"), this);
    treemenu->addAction(copybranch);
    copybranch->setIcon(copyIcon);
    connect(copybranch, &QAction::triggered, this, &MainWindow::tree_copybranch);

    QIcon pasteIcon = QIcon::fromTheme("", QIcon(":/images/editpaste.png"));
    QAction *pastebranch = new QAction(tr("&Paste branch"), this);
    treemenu->addAction(pastebranch);
    pastebranch->setIcon(pasteIcon);
    connect(pastebranch, &QAction::triggered, this, &MainWindow::tree_pastebranch);

    treemenu->addSeparator();

    QIcon expandIcon = QIcon::fromTheme("", QIcon(":/images/expandtree.png"));
    QAction *expand = new QAction(tr("&Expand all branches"), this);
    treemenu->addAction(expand);
    expand->setIcon(expandIcon);
    connect(expand, &QAction::triggered, this, &MainWindow::expand_tree);

    QIcon collapseIcon = QIcon::fromTheme("", QIcon(":/images/collapsetree.png"));
    QAction *collapse = new QAction(tr("Co&llapse all branches"), this);
    treemenu->addAction(collapse);
    collapse->setIcon(collapseIcon);
    connect(collapse, &QAction::triggered, this, &MainWindow::collapse_tree);

    treemenu->addSeparator();

    QIcon sortAscIcon = QIcon::fromTheme("", QIcon(":/images/view-sort-ascending.png"));
    QAction *sortAsc = new QAction(tr("&Sort tree in ascending order"), this);
    treemenu->addAction(sortAsc);
    sortAsc->setIcon(sortAscIcon);
    connect(sortAsc, &QAction::triggered, this, &MainWindow::sort_asc_tree);

    QIcon sortDescIcon = QIcon::fromTheme("", QIcon(":/images/view-sort-descending.png"));
    QAction *sortDesc = new QAction(tr("&Sort tree in descending order"), this);
    treemenu->addAction(sortDesc);
    sortDesc->setIcon(sortDescIcon);
    connect(sortDesc, &QAction::triggered, this, &MainWindow::sort_desc_tree);

    treemenu->addSeparator();

    QIcon delIcon = QIcon::fromTheme("", QIcon(":/images/editdelete.png"));
    QAction *delbranch = new QAction(tr("&Delete branch"), this);
    treemenu->addAction(delbranch);
    delbranch->setIcon(delIcon);
    connect(delbranch, &QAction::triggered, this, &MainWindow::tree_delbranch);

    //help menu
    helpmenu = menuBar()->addMenu(tr("&Help"));

    QAction *helpitem = new QAction(tr("&Help"), this);
    helpmenu->addAction(helpitem);
    connect(helpitem, &QAction::triggered, this, &MainWindow::help);

    QAction *aboutitem = new QAction(tr("&About"), this);
    helpmenu->addAction(aboutitem);
    connect(aboutitem, &QAction::triggered, this, &MainWindow::about);

    helpmenu->addSeparator();

    QAction *aboutQtitem = new QAction(tr("About &Qt"), this);
    helpmenu->addAction(aboutQtitem);
    connect(aboutQtitem, &QAction::triggered, qApp, &QApplication::aboutQt);

    //toolbar
    tb1 = new QToolBar(tr("Tree tools"), this);
    tb1->setFloatable (false);
    tb1->setMovable(false);
    addToolBar(tb1);

    //keep the icon-heavy toolbars on separate rows so nothing overflows the window
    addToolBarBreak();

    tb2 = new QToolBar(tr("Editor tools"), this);
    tb2->setFloatable (false);
    tb2->setMovable(false);
    addToolBar(tb2);

    actionTextBold = new QAction(QIcon::fromTheme("", QIcon(":/images/textbold.png")), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL | Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, &QAction::triggered, this, &MainWindow::textBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("", QIcon(":/images/textitalic.png")), tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL | Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, &QAction::triggered, this, &MainWindow::textItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("", QIcon(":/images/textunder.png")), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL | Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, &QAction::triggered, this, &MainWindow::textUnderline);
    actionTextUnderline->setCheckable(true);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, &QActionGroup::triggered, this, &MainWindow::textAlign);

    actionAlignLeft = new QAction(QIcon::fromTheme("", QIcon(":/images/textleft.png")),tr("&Left align text"), this);
    actionAlignCenter = new QAction(QIcon::fromTheme("",QIcon(":/images/textcenter.png")),tr("C&entered text"), this);
    actionAlignRight = new QAction(QIcon::fromTheme("",QIcon(":/images/textright.png")),tr("&Right align text"), this);
    actionAlignJustify = new QAction(QIcon::fromTheme("",QIcon(":/images/textjustify.png")),tr("&Justify text"), this);

    grp->addAction (actionAlignLeft);
    grp->addAction (actionAlignCenter);
    grp->addAction (actionAlignRight);
    grp->addAction (actionAlignJustify);

    actionAlignLeft->setShortcut(Qt::CTRL | Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);

    actionAlignCenter->setShortcut(Qt::CTRL | Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);

    actionAlignRight->setShortcut(Qt::CTRL | Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);

    actionAlignJustify->setShortcut(Qt::CTRL | Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    actionInsertImage= new QAction(QIcon::fromTheme("", QIcon(":/images/insert-image.png")), tr("&Insert image"), this);
    actionInsertImage->setPriority(QAction::LowPriority);
    connect(actionInsertImage, &QAction::triggered, this, &MainWindow::insertImage);
    actionInsertImage->setCheckable(true);

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("Text color..."), this);
    connect(actionTextColor, &QAction::triggered, this, &MainWindow::textColor);

    comboFont = new QFontComboBox(tb2);
    comboFont->setEditable(false);
    comboFont->setFontFilters(QFontComboBox::ScalableFonts | QFontComboBox::ProportionalFonts);
    comboFont->addItems(QFontDatabase::families(QFontDatabase::WritingSystem (1)));
    comboFont->setCurrentFont(QApplication::font());
    connect(comboFont, &QFontComboBox::currentFontChanged, this, &MainWindow::fontFamily);

    comboSize = new QComboBox(tb2);
    comboSize->setObjectName("comboSize");
    comboSize->setEditable(false);
    for (int size : QFontDatabase::standardSizes())
        comboSize->addItem(QString::number(size));
    connect(comboSize, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::textSize);
    //QApplication::font().setPointSize(6);
    comboSize->setCurrentIndex(comboSize->findText(QString::number(6)));

    srchbox = new QLineEdit (tb1);
    srchbox->setObjectName("srchbox");
    srchbox->setMaxLength(50);
    srchbox->setPlaceholderText("Enter search text...");
    srchbox->setMaximumWidth(200);
    srchbox->setMinimumWidth(200);

    QIcon srchnxtIcon = QIcon::fromTheme("", QIcon(":/images/go-down-search.png"));
    QAction *srchnxt = new QAction(tr("Search &next"), this);
    srchnxt->setIcon(srchnxtIcon);
    connect(srchnxt, &QAction::triggered, this, &MainWindow::tree_srch_nxt);

    QIcon srchpreIcon = QIcon::fromTheme("", QIcon(":/images/go-up-search.png"));
    QAction *srchpre = new QAction(tr("Search &previous"), this);
    srchpre->setIcon(srchpreIcon);
    connect(srchpre, &QAction::triggered, this, &MainWindow::tree_srch_pre);

    tb1->addAction(addbranch);
    tb1->addAction(addsubbranch);
    tb1->addSeparator();

    tb1->addAction(expand);
    tb1->addAction(collapse);
    tb1->addSeparator();

    tb1->addAction(sortAsc);
    tb1->addAction(sortDesc);
    tb1->addSeparator();

    tb1->addAction(cutbranch);
    tb1->addAction(copybranch);
    tb1->addAction(pastebranch);
    tb1->addSeparator();

    tb1->addAction(delbranch);
    tb1->addSeparator();

    tb1->addWidget(srchbox);
    tb1->addAction(srchnxt);
    tb1->addAction(srchpre);

    tb2->addAction(newfile);
    tb2->addAction(openfile);
    tb2->addAction(savefile);
    tb2->addAction(savefileas);
    tb2->addSeparator();

    tb2->addAction(actionTextBold);
    tb2->addAction(actionTextItalic);
    tb2->addAction(actionTextUnderline);
    tb2->addSeparator();

    tb2->addActions(grp->actions());
    tb2->addSeparator();

    tb2->addAction(actionInsertImage);
    tb2->addSeparator();

    tb2->addAction(actionTextColor);
    tb2->addWidget(comboFont);
    tb2->addWidget(comboSize);
}

void MainWindow::textBold()
{
QTextCharFormat fmt;

    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::textItalic()
{
QTextCharFormat fmt;

    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);

}

void MainWindow::textUnderline()
{
QTextCharFormat fmt;

    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);

}

void MainWindow::textAlign(QAction *a)
{
QTextEdit *editor;
Qt::Alignment al;

    editor = leafview;
    al = editor->alignment ();

    if (a == actionAlignLeft)
        editor->setAlignment(Qt::AlignLeft);
    else if (a == actionAlignCenter)
        editor->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        editor->setAlignment(Qt::AlignRight);
    else if (a == actionAlignJustify)
        editor->setAlignment(Qt::AlignJustify);
}

void MainWindow::textColor()
{
QTextEdit *editor;

editor = leafview;

    QColor col = QColorDialog::getColor(editor->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void MainWindow::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void MainWindow::fontFamily()
{
QTextCharFormat fmt;

    fmt.setFont(comboFont->currentFont ());
    mergeFormatOnWordOrSelection(fmt);

}

void MainWindow::textSize(int index)
//index is the position of the highlighted number in the combobox
{
int i;
QString s;
QTextCharFormat fmt;

    s = comboSize->itemText (index);
    i = s.toInt ();
    fmt.setFontPointSize((qreal)i);
    mergeFormatOnWordOrSelection(fmt);
}


void MainWindow::insertImage()
{
QString s, filters, fname;
QFileInfo fi;
QTextEdit *editor;
QMessageBox msgBox;

    editor = leafview;

    filters += tr("Common Graphics (*.png *.jpg *.jpeg *.gif);;");
    filters += tr("Portable Network Graphics (PNG) (*.png);;");
    filters += tr("JPEG (*.jpg *.jpeg);;");
    filters += tr("Graphics Interchange Format (*.gif);;");
    filters += tr("All Files (*)");

    QString file = QFileDialog::getOpenFileName(this, tr("Open image..."), QString(), filters);
    if (file.isEmpty())
        return;
    if (!QFile::exists(file))
        return;

    fi = QFileInfo(file);
    if (fi.path() != DataDir) {
        //copy the file to the data subdirectory; make the name unique so that two
        //different source files with the same name cannot collide here
        fname.clear();
        fname.append(DataDir);
        fname.append("/");
        fname.append(fi.fileName());
        int n = 1;
        while (QFile::exists(fname)) {
            fname.clear();
            fname.append(DataDir);
            fname.append("/");
            fname.append(fi.completeBaseName());
            fname.append("_");
            fname.append(QString::number(n));
            if (fi.suffix().isEmpty() == false) {
                fname.append(".");
                fname.append(fi.suffix());
            }
            n++;
        }
        if (QFile::copy(fi.filePath(), fname) == false) {
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(QObject::tr("Could not copy the image file to the Treecle data directory: ") + fname);
            msgBox.setInformativeText(QObject::tr("The image was not inserted."));
            msgBox.exec();
            return;
        }

        s.append (QObject::tr("The image file has been copied to the Treecle data directory "));
        s.append (DataDir);
        s.append (QObject::tr("\nClick OK to continue"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(s);
        msgBox.exec();
    }
    else
        fname.append(fi.filePath());

    if (tree->topLevelItemCount() == 0)
        return;
    //store only the file name; the data dir path is prepended when displayed
    editor->insertHtml(QString("<img src=\"%1\">").arg(QFileInfo(fname).fileName()));
    //immediately fit the new image to the panel width (aspect ratio kept)
    fit_editor_images(qMax(60, leafview->viewport()->width() - 30));
}

void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
QTextEdit *editor;

    editor = leafview;

    QTextCursor cursor = editor->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    editor->mergeCurrentCharFormat(format);
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    if (file_modified == true) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Treecle"), tr("Do you wish to save or discard the current tree?\n"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Cancel) {//user cancels
            statustext->setText(tr("The tree has not been saved "));
            event->ignore();
            return;
        }

        if (ret == QMessageBox::Save) {
            if (save_file() == true) {//file saved by the user
                statustext->setText(tr("Saved"));
                writeprefs();
                event->accept();
            }
            else {//user cancelled the save operation
                statustext->setText(tr("The tree has not been saved"));
                event->ignore();
            }
            return;
        }

        if (ret == QMessageBox::Discard) {
            statustext->setText(tr("Discarded"));
            writeprefs();
            event->accept();
        }
    }
    else {
        writeprefs();
        event->accept();
    }

}

void MainWindow::help()
//open and show the user manual
{
bool ok;
QString s1;
QMessageBox msgBox;

    QFile file(Helpfilename);
    ok = file.exists();
    if (ok == false) {
        s1 = QObject::tr("The help file \"treeclehelp.pdf\"was not found\n");
        s1.append (QObject::tr("Please make sure that it is present in the treecle data directory.\n"));
        s1.append("\n");
        s1.append (DataDir);
        s1.append("\n");

        msgBox.setText(s1);
        msgBox.exec();
    }
    else {
        s1.append ("file://");
        s1.append (Helpfilename);
        ok = QDesktopServices::openUrl (QUrl (s1));
        if (ok == false) {
            msgBox.setText("Could not open help file.");
            msgBox.exec();
        }
    }
}

void MainWindow::about()
//open a window to show program information and copyright license
{
QFile file(Gnugplfilename);
QTextStream in(&file);
QTextBrowser *gnugpl;
bool ok;

    ok = file.open(QFile::ReadOnly);
    if (ok == false)
        return;

    gnugpl = new QTextBrowser ();
    gnugpl->setAttribute(Qt::WA_DeleteOnClose);
    gnugpl->setGeometry(100, 100, 800, 600);
    gnugpl->setWindowTitle (QObject::tr("About Treecle"));
    gnugpl->setPlainText(in.readAll());
    gnugpl->setAlignment(Qt::AlignLeft);
    gnugpl->show();
}

void check_and_make_data_dir (const QString &dataDir)
{
QString qtpath, s1;
QDir qtdir;
QMessageBox msgBox;

    qtpath.append (dataDir);
qtdir = QDir (qtpath);

    if (qtdir.exists() == false) {
        s1.append (QObject::tr("The treecle data directory does not exist. "));
        s1.append (QObject::tr("This is required to store your work.\n"));
        s1.append (QObject::tr("Click OK to create a new, subdirectory "));
        s1.append (QObject::tr("in your area with the name : \n"));
        s1.append (qtpath);
        s1.append ("\n");
        s1.append (QObject::tr("The help file treeclehelp.pdf should be copied\n"));
        s1.append (QObject::tr("in this location for online help to be available.\n"));
        s1.append (QObject::tr("The license file COPYING should also be copied\n"));
        s1.append (QObject::tr("in this location for the program details to be available.\n"));
        //s1.append (Lockfilename);

        msgBox.setText(s1);
        msgBox.exec();

        qtdir.mkpath(qtpath);//mkdir for nested parents (e.g. ~/.local/share)
    }
}

QString fit_images_to_width (const QString &html, const QString &dataDir, int maxWidth)
{
QRegularExpression tagRe("<img[^>]*>");
QRegularExpression srcRe("src=\"([^\"]*)\"");
QRegularExpression sizeRe("\\s+(width|height)=\\\"[^\\\"]*\\\"");
QRegularExpressionMatchIterator it;
QRegularExpressionMatch m;
QString out, tag, newTag, src, path;
QImageReader reader;
QSize orig;
int pos = 0, w = 0, h = 0;

    if (maxWidth < 1)
        return html;

    it = tagRe.globalMatch(html);
    while (it.hasNext()) {
        m = it.next();
        out.append(html.mid(pos, m.capturedStart() - pos));
        pos = m.capturedEnd();
        tag = m.captured();
        newTag = tag;

        src = srcRe.match(tag).captured(1);
        path = QUrl::fromPercentEncoding(src.toUtf8());
        //shrink only bare filenames that are expected in the data dir
        if (path.isEmpty() == false && path.contains('/') == false &&
            path.contains("://") == false) {
            reader.setFileName(dataDir + "/" + QFileInfo(src).fileName());
            orig = reader.size();
            if (orig.isValid() && orig.width() > maxWidth) {
                w = maxWidth;
                h = qMax(1, (int)((qreal)orig.height() * maxWidth / orig.width()));
                //replace any existing size attributes, then impose ours, before the
                //tag's closing ">" (or "/>" for self-closing img tags)
                newTag.replace(sizeRe, QString());
                int ins = newTag.lastIndexOf('>');
                if (ins > 0 && newTag.at(ins - 1) == '/')
                    ins--;
                newTag.insert(ins,
                              QString(" width=\"%1\" height=\"%2\"").arg(w).arg(h));
            }
        }
        out.append(newTag);
    }
    out.append(html.mid(pos));
    return out;
}

QString strip_image_sizes (const QString &html)
{
QRegularExpression tagRe("<img[^>]*>");
QRegularExpression sizeRe("\\s+(width|height)\\s*=\\s*\"[^\"]*\"");
QRegularExpressionMatchIterator it;
QRegularExpressionMatch m;
QString out, tag;
int pos = 0;

    it = tagRe.globalMatch(html);
    while (it.hasNext()) {
        m = it.next();
        out.append(html.mid(pos, m.capturedStart() - pos));
        pos = m.capturedEnd();
        tag = m.captured();
        tag.replace(sizeRe, QString());
        out.append(tag);
    }
    out.append(html.mid(pos));
    return out;
}

//rescale every on-screen image so its width fits maxWidth (aspect ratio kept).
//Only the display is affected: the char data is untouched and toHtml still
//exports the bare filename link (sizes are stripped again when stored).
void MainWindow::fit_editor_images (int maxWidth)
{
struct ImgScale { int pos; int len; QTextImageFormat fmt; };
QImageReader reader;
QString name, path;
QSize orig;
int w, h;
QList<ImgScale> work;

    if (maxWidth < 1)
        return;

    for (QTextBlock block = leafdoc->begin (); block.isValid (); block = block.next ()) {
        for (QTextBlock::iterator it = block.begin (); !it.atEnd (); ++it) {
            QTextFragment frag = it.fragment ();
            if (frag.isValid () == false)
                continue;
            QTextCharFormat cf = frag.charFormat ();
            if (cf.isImageFormat () == false)
                continue;
            QTextImageFormat img = cf.toImageFormat ();
            name = img.name ();
            if (name.isEmpty ())
                continue;
            //only bare data-dir filenames follow the panel width; external
            //links and absolute paths are left exactly as authored
            if (name.indexOf('/') != -1)
                continue;
            reader.setFileName (DataDir + "/" + name);
            orig = reader.size ();
            if (orig.isValid () == false)
                continue;
            w = -1;
            h = -1;
            if (orig.width () > maxWidth) {
                w = maxWidth;
                h = qMax(1, (int)((qreal)orig.height () * maxWidth / orig.width ()));
            }
            else if (img.width () <= 0)
                continue;   //already at its intrinsic size
            if (img.width () == w && img.height () == h)
                continue;   //already sized to the current panel
            ImgScale s;
            s.pos = frag.position ();
            s.len = frag.length ();
            s.fmt = img;
            s.fmt.setWidth (w);
            s.fmt.setHeight (h);
            work.append (s);
        }
    }

    if (work.isEmpty ())
        return;

    QTextCursor cur (leafdoc);
    cur.beginEditBlock ();
    for (const ImgScale &s : work) {
        cur.setPosition (s.pos);
        cur.setPosition (s.pos + s.len, QTextCursor::KeepAnchor);
        cur.setCharFormat (s.fmt);
    }
    cur.endEditBlock ();
}

bool MainWindow::eventFilter (QObject *obj, QEvent *ev)
{
    if (obj == leafview && ev->type () == QEvent::Resize)
        fit_editor_images (qMax(60, leafview->viewport ()->width () - 30));
    return QMainWindow::eventFilter (obj, ev);
}

bool migrate_old_data_dir (const QString &oldDir, const QString &newDir)
{
QDir newdf;
QFileInfo si;
QString src, dst;
bool did_migrate = false;

    if (oldDir.isEmpty() || oldDir == newDir || QDir(oldDir).exists() == false)
        return false;

    newdf = QDir(newDir);
    if (newdf.exists() == false) {
        if (QDir().mkpath(newDir) == false)
            return false;
    }

    const QStringList entries = QDir(oldDir).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &name : entries) {
        if (name == "treelockfile.lck")
            continue;//the lock file belongs to its old location
        src = QDir(oldDir).filePath(name);
        dst = newdf.filePath(name);
        si = QFileInfo(src);
        if (si.isDir()) {
            if (QDir(dst).exists() == false) {
                QDir tmp;
                tmp.rename(src, dst);
                did_migrate = true;
            }
        }
        else if (QFile::exists(dst) == false) {
            if (QFile::copy(src, dst) == false)
                return false;
            did_migrate = true;
        }
    }
    return did_migrate;
}

bool acquire_lock(QLockFile *lock)
{
QMessageBox msgBox;
QString s1, s2, s3;
qint64 pid = -1;
QString host, app;
bool ok = false;

    ok = lock->tryLock();
    if (ok == true)//lock acquired, single instance, continue
        return true;

    //the lock file already exists - a stale lock is removed automatically by
    //tryLock(), so a persistent lock means another live instance holds it
    lock->getLockInfo(&pid, &host, &app);

    s1 = QObject::tr("It seems that \"Treecle\" is already running.");

    if (pid > 0) {//the lock file holds the identity of the other instance
        s2 = QObject::tr("Click \"Continue\" to start another instance,\nelse click \"Abort\". ");
        s3 = QObject::tr("The other instance has the process ID ");
        s3.append (QString::number(pid));
        s3.append (QObject::tr(" and runs on the host \""));
        s3.append (host);
        s3.append (QObject::tr("\". "));
        s3.append (QObject::tr("If Treecle is not running, its lock is stale and \"Continue\" removes it. "));
        s3.append (QObject::tr("See the user manual about the risks of running two instances of the program at the same time.\n"));
    }
    else {//the lock file could not be read
        s2 = QObject::tr("If this is not the case, click \"Continue\", else click \"Abort\". ");
        s3 = QObject::tr("A lockfile has been found in the hidden treecle data-subdirectory. ");
        s3.append (QObject::tr("The program may be currently running in another window, in which case click \"Abort\". "));
        s3.append (QObject::tr("If you are sure that treecle is not running in this account, click \"Continue\" to remove it. "));
        s3.append (QObject::tr("See the user manual about the risks of running two instances of the program at the same time.\n"));
    }

    msgBox.setText(s1);
    msgBox.setInformativeText(s2);
    msgBox.setDetailedText(s3);

    msgBox.addButton(QObject::tr("Continue"), QMessageBox::AcceptRole);
    msgBox.addButton(QObject::tr("Abort"), QMessageBox::RejectRole);

    msgBox.exec();
    QAbstractButton *btn = msgBox.clickedButton();

    if (btn == nullptr || msgBox.buttonRole(btn) != QMessageBox::AcceptRole)//abort or dialog closed
        return false;

    //Continue: the lock may be stale (its owner process is gone). Remove it and retry.
    lock->removeStaleLockFile();
    ok = lock->tryLock();
    if (ok == true)//lock acquired after removing the stale lock
        return true;

    //a live instance really holds the lock - run without a lock anyway
    QMessageBox::warning(nullptr, "Treecle",
        QObject::tr("Treecle is still running in another window.\n"
                    "Starting a second instance without a lock;\nsaving the same tree from both windows may lose data."));
    return true;
}

void MainWindow::set_panel_focus()
{
    if (leafview->hasFocus ())
        set_tree_focus ();
    else if (tree->hasFocus ())
        set_editor_focus ();
}

void MainWindow::set_editor_focus()
{
    leafview->setFocus ();
}

void MainWindow::set_tree_focus()
{
    tree->setFocus ();
}

QTreeWidgetItem* MainWindow::get_highlighted_branch()
{
QTreeWidgetItem *it;

    it = tree->currentItem();
    if (it == nullptr)
        return nullptr;
    tree->setFocus ();
    set_branch(it);

    return (it);
}

void MainWindow::set_modified_flag()
{
    if (file_read_in_progress == false && branch_display_in_progress == false)
        file_modified = true;
}
