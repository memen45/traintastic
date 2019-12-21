/**
 * client/src/mainwindow.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mainwindow.hpp"
#include <QMenuBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QVBoxLayout>
#include <QToolBar>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include "dialog/connectdialog.hpp"
#include "network/client.hpp"
#include "network/object.hpp"
#include "network/property.hpp"
#include "subwindow/hardwarelistsubwindow.hpp"
#include "subwindow/objecteditsubwindow.hpp"
#include "subwindow/serversettingssubwindow.hpp"
#include "subwindow/serverconsolesubwindow.hpp"

#define SETTING_PREFIX "mainwindow/"
#define SETTING_GEOMETRY SETTING_PREFIX "geometry"
#define SETTING_WINDOWSTATE SETTING_PREFIX "windowstate"

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  m_mdiArea{new QMdiArea()}
{
  instance = this;

  setWindowTitle("Traintastic");

  QAction* actFullScreen;

  m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  {
    setMenuBar(new QMenuBar());
    QMenu* menu;
    //QMenu* subMenu;
    //QAction* act;

    menu = menuBar()->addMenu(tr("File"));
    m_actionConnectToServer = menu->addAction(tr("Connect to server") + "...", this, &MainWindow::connectToServer);
    m_actionDisconnectFromServer = menu->addAction(tr("Disconnect from server"), this, &MainWindow::disconnectFromServer);
    menu->addSeparator();
    m_actionNewWorld = menu->addAction(tr("New world"), this, &MainWindow::newWorld);
    m_actionLoadWorld = menu->addAction(tr("Load world") + "...", this, &MainWindow::loadWorld);
    m_actionSaveWorld = menu->addAction(tr("Save world"), this, &MainWindow::saveWorld);
    menu->addSeparator();
    m_actionImportWorld = menu->addAction(tr("Import world") + "...", this, &MainWindow::importWorld);
    m_actionExportWorld = menu->addAction(tr("Export world") + "...", this, &MainWindow::exportWorld);
    menu->addSeparator();
    menu->addAction(tr("Quit"), this, &MainWindow::close);

    menu = menuBar()->addMenu(tr("View"));
    actFullScreen = menu->addAction(tr("Fullscreen"), this, &MainWindow::toggleFullScreen);
    actFullScreen->setCheckable(true);
    actFullScreen->setShortcut(Qt::Key_F11);

    menu = menuBar()->addMenu(tr("Objects"));
    m_actionHardware = menu->addAction(QIcon(":/dark/hardware.svg"), tr("Hardware") + "...", this, &MainWindow::showHardware);

    menu = menuBar()->addMenu(tr("Tools"));
    menu->addAction(tr("Client settings") + "...");
    menu->addSeparator();
    m_actionServerSettings = menu->addAction(tr("Server settings") + "...", this, &MainWindow::showServerSettings);
    m_actionServerConsole = menu->addAction(tr("Server console") + "...", this, &MainWindow::showServerConsole);
    m_actionServerConsole->setShortcut(Qt::Key_F12);

    menu = menuBar()->addMenu(tr("Help"));
    menu->addAction(tr("Help"));
    menu->addSeparator();
    menu->addAction(tr("About Qt") + "...", qApp, &QApplication::aboutQt);
    menu->addAction(tr("About") + "...", this, &MainWindow::showAbout);
  }

  QToolBar* toolbar = new QToolBar();
  m_actionGroupMode = new QActionGroup(this);
  m_actionGroupMode->setExclusive(true);
  m_actionModeRun = toolbar->addAction(QIcon(":/dark/run.svg"), tr("Run"), [this](){ setMode(TraintasticMode::Run); });
  m_actionModeRun->setCheckable(true);
  m_actionGroupMode->addAction(m_actionModeRun);
  m_actionModeStop = toolbar->addAction(QIcon(":/dark/stop.svg"), tr("Stop"), [this](){ setMode(TraintasticMode::Stop); });
  m_actionModeStop->setCheckable(true);
  m_actionGroupMode->addAction(m_actionModeStop);
  m_actionModeEdit = toolbar->addAction(QIcon(":/dark/edit.svg"), tr("Edit"), [this](){ setMode(TraintasticMode::Edit); });
  m_actionModeEdit->setCheckable(true);
  m_actionGroupMode->addAction(m_actionModeEdit);
  toolbar->addSeparator();
  toolbar->addAction(m_actionHardware);

  QVBoxLayout* l = new QVBoxLayout();
  l->setMargin(0);
  l->addWidget(toolbar);
  l->addWidget(m_mdiArea);

  QWidget* w = new QWidget();
  w->setLayout(l);

  setCentralWidget(w);
  setStatusBar(new QStatusBar());

  QSettings settings;
  if(settings.contains(SETTING_GEOMETRY))
    restoreGeometry(settings.value(SETTING_GEOMETRY).toByteArray());
  if(settings.contains(SETTING_WINDOWSTATE))
    setWindowState(static_cast<Qt::WindowState>(settings.value(SETTING_WINDOWSTATE).toInt()));
  actFullScreen->setChecked(isFullScreen());

  connect(Client::instance, &Client::stateChanged, this, &MainWindow::clientStateChanged);

  clientStateChanged();
}

MainWindow::~MainWindow()
{
}

void MainWindow::connectToServer()
{
  ConnectDialog* d = new ConnectDialog(this);
  d->setModal(true);
  d->show();
}

void MainWindow::disconnectFromServer()
{
  Client::instance->disconnectFromHost();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.setValue(SETTING_GEOMETRY, saveGeometry());
  settings.setValue(SETTING_WINDOWSTATE, static_cast<int>(windowState()));
  QMainWindow::closeEvent(event);
}

void MainWindow::setMode(TraintasticMode value)
{
  Client& client = *Client::instance;
  if(client.state() == Client::State::Connected && client.traintastic())
    client.traintastic()->getProperty("mode")->setValueInt64(static_cast<int64_t>(value));
  else
    updateModeActions();
}

void MainWindow::newWorld()
{
}

void MainWindow::loadWorld()
{
}

void MainWindow::saveWorld()
{
}

void MainWindow::importWorld()
{
}

void MainWindow::exportWorld()
{
}

void MainWindow::toggleFullScreen()
{
  const bool fullScreen = qobject_cast<QAction*>(sender())->isChecked();
  if(fullScreen == isFullScreen())
    return;

  if(fullScreen)
  {
    m_beforeFullScreenGeometry = saveGeometry();
    showFullScreen();
  }
  else
  {
    showNormal(); // required to exit fullscreen mode
    if(!m_beforeFullScreenGeometry.isEmpty())
      restoreGeometry(m_beforeFullScreenGeometry);
    else
      showMaximized();
 }
}

void MainWindow::showObjectEdit(const QString& id)
{
  if(!m_mdiSubWindow.objectEdit.contains(id))
  {
    ObjectEditSubWindow* window = new ObjectEditSubWindow(id);
    m_mdiArea->addSubWindow(window);
    window->setAttribute(Qt::WA_DeleteOnClose);
    connect(window, &QMdiSubWindow::destroyed, [this, id](QObject*){ m_mdiSubWindow.objectEdit.remove(id); });
    window->show();
    m_mdiSubWindow.objectEdit[id] = window;
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindow.objectEdit[id]);
}

void MainWindow::showHardware()
{
  if(!m_mdiSubWindow.hardwareList)
  {
    m_mdiSubWindow.hardwareList = new HardwareListSubWindow();
    m_mdiArea->addSubWindow(m_mdiSubWindow.hardwareList);
    m_mdiSubWindow.hardwareList->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_mdiSubWindow.hardwareList, &QMdiSubWindow::destroyed, [this](QObject*){ m_mdiSubWindow.hardwareList = nullptr; });
    m_mdiSubWindow.hardwareList->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindow.hardwareList);
}

void MainWindow::showServerSettings()
{
  if(!m_mdiSubWindow.serverSettings)
  {
    m_mdiSubWindow.serverSettings = new ServerSettingsSubWindow();
    m_mdiArea->addSubWindow(m_mdiSubWindow.serverSettings);
    m_mdiSubWindow.serverSettings->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_mdiSubWindow.serverSettings, &QMdiSubWindow::destroyed, [this](QObject*){ m_mdiSubWindow.serverSettings = nullptr; });
    m_mdiSubWindow.serverSettings->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindow.serverSettings);
}

void MainWindow::showServerConsole()
{
  if(!m_mdiSubWindow.serverConsole)
  {
    m_mdiSubWindow.serverConsole = new ServerConsoleSubWindow();
    m_mdiArea->addSubWindow(m_mdiSubWindow.serverConsole);
    m_mdiSubWindow.serverConsole->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_mdiSubWindow.serverConsole, &QMdiSubWindow::destroyed, [this](QObject*){ m_mdiSubWindow.serverConsole = nullptr; });
    m_mdiSubWindow.serverConsole->show();
  }
  else
    m_mdiArea->setActiveSubWindow(m_mdiSubWindow.serverConsole);
}

void MainWindow::showAbout()
{
  QMessageBox::about(this, tr("About Traintastic"),
    "<h2>Traintastic <small>v" + QApplication::applicationVersion() + "</small></h2>"
    "<p>Copyright &copy; 2019 Reinder Feenstra</p>"
    "<p>This program is free software; you can redistribute it and/or"
    " modify it under the terms of the GNU General Public License"
    " as published by the Free Software Foundation; either version 2"
    " of the License, or (at your option) any later version.</p>"
    "<p>This program is distributed in the hope that it will be useful,"
    " but WITHOUT ANY WARRANTY; without even the implied warranty of"
    " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the"
    " GNU General Public License for more details.</p>");
}

void MainWindow::clientStateChanged()
{
  Client& client = *Client::instance;
  const bool connected = client.state() == Client::State::Connected;

  m_mdiArea->setEnabled(connected);
  m_actionConnectToServer->setEnabled(client.isDisconnected());
  m_actionConnectToServer->setVisible(!connected);
  m_actionDisconnectFromServer->setVisible(connected);
  m_actionNewWorld->setEnabled(connected);
  m_actionLoadWorld->setEnabled(connected);
  m_actionSaveWorld->setEnabled(connected && false);
  m_actionImportWorld->setEnabled(connected);
  m_actionExportWorld->setEnabled(connected);
  m_actionHardware->setEnabled(connected);
  m_actionServerSettings->setEnabled(connected);
  m_actionServerConsole->setEnabled(connected);

  if(connected)
    connect(client.traintastic()->getProperty("mode"), &Property::valueChanged, this, &MainWindow::updateModeActions);

  updateModeActions();

  //if(client.state() == Client::State::SocketError)
  //  statusBar()->showMessage(client.errorString());
}

void MainWindow::updateModeActions()
{
  Client& client = *Client::instance;
  const bool connected = client.state() == Client::State::Connected;
  const TraintasticMode mode = connected ? static_cast<TraintasticMode>(client.traintastic()->getProperty("mode")->toInt64()) : TraintasticMode::Stop;

  m_actionModeRun->setEnabled(connected && mode != TraintasticMode::Edit);
  m_actionModeStop->setEnabled(connected);
  m_actionModeEdit->setEnabled(connected && mode != TraintasticMode::Run);

  switch(mode)
  {
    case TraintasticMode::Run:
      m_actionModeRun->setChecked(true);
      break;

    case TraintasticMode::Stop:
      m_actionModeStop->setChecked(true);
      break;

    case TraintasticMode::Edit:
      m_actionModeEdit->setChecked(true);
      break;

    default:
      Q_ASSERT(false);
      break;
  }
}
