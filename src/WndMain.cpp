//***************************************************************************
//
// GPS Recorder
// A GPS data logger for the Maemo platform.
//
// Copyright (C) 2010 Jean-Charles Lefebvre <polyvertex+gpsrecorder [AT] gmail [DOT] com>
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions :
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//
// Author     : Jean-Charles Lefebvre
// Created On : 2010-03-25 14:53:26
//
// $Id: WndMain.cpp 230 2013-05-10 09:28:15Z jcl $
//
//***************************************************************************

#include "stable.h"
#include <cmath>


//---------------------------------------------------------------------------
// WndMain
//---------------------------------------------------------------------------
WndMain::WndMain (QMainWindow* pParent/*=0*/)
: WndBase(pParent)
{
  Q_ASSERT(App::instance());
  Q_ASSERT(App::instance()->location());

  this->setWindowTitle(App::applicationLabel());

  this->menuBar()->clear();
  m_pMenuStartStop = this->menuBar()->addAction(tr("Start"), this, SLOT(onClickedStartStop()));
  m_pMenuConfig    = this->menuBar()->addAction(tr("Settings"), this, SLOT(onClickedConfig()));
  m_pMenuConvert   = this->menuBar()->addAction(tr("Convert"), this, SLOT(onClickedConvert()));
  m_pMenuAbout     = this->menuBar()->addAction(tr("About"), this, SLOT(onClickedAbout()));
  this->menuBar()->hide();

  this->createWidgets();
  this->clearFixFields();
  this->showHome();

  this->connect(
    App::instance(),
    SIGNAL(sigAppStatePixChanged(QPixmap*)),
    SLOT(onAppStatePixChanged(QPixmap*)) );

  this->connect(
    App::instance()->location(),
    SIGNAL(sigLocationFix(Location*, const LocationFixContainer*, bool)),
    SLOT(onLocationFix(Location*, const LocationFixContainer*, bool)) );

  this->connect(
    App::instance()->battery(),
    SIGNAL(sigBatteryStatusChanged(int, int, int, bool)),
    SLOT(onBatteryStatusChanged(int, int, int, bool)) );

  // force refresh battery status
  App::instance()->battery()->pollBattery();
}

//---------------------------------------------------------------------------
// ~WndMain
//---------------------------------------------------------------------------
WndMain::~WndMain (void)
{
}



//---------------------------------------------------------------------------
// setMeansOfTransport
//---------------------------------------------------------------------------
void WndMain::setMeansOfTransport (quint8 ucMeansOfTransport, const QString& strOtherMeansOfTransport)
{
  if (m_pLblMeansOfTransport)
  {
    AppSettings& settings = *App::instance()->settings();

    m_pLblMeansOfTransport->setText(GPSRFile::fullMeansOfTransportToLabel(
      settings.getLastMeansOfTransport(),
      settings.getLastOtherMeansOfTransport().constData() ));
  }
}



//---------------------------------------------------------------------------
// closeEvent
//---------------------------------------------------------------------------
void WndMain::closeEvent (QCloseEvent* pEvent)
{
  if (App::instance() && App::instance()->getState() != App::STATE_STOPPED)
  {
    QMessageBox::StandardButton eBtn = QMessageBox::question(
      this,
      tr("Quit ?"),
      tr("You are about to stop recording a track by closing this application ! Is it what you want ?"),
      QMessageBox::Yes | QMessageBox::Cancel,
      QMessageBox::Cancel);

    if (eBtn != QMessageBox::Yes && eBtn != QMessageBox::Ok)
    {
      pEvent->ignore();
      return;
    }
    else
    {
      this->onClickedStartStop();
    }
  }

  pEvent->accept();
}



//---------------------------------------------------------------------------
// createWidgets
//---------------------------------------------------------------------------
void WndMain::createWidgets (void)
{
  m_pLblStatus = new QLabel();
  m_pLblStatus->setDisabled(true);

  m_pLblStatusIcon = new QLabel();
  m_pLblStatusIcon->setFixedWidth(App::instance()->getStatePix()->width() + 20);
  m_pLblStatusIcon->setAlignment(Qt::AlignCenter);
  m_pLblStatusIcon->setPixmap(*App::instance()->getStatePix());

  m_pLblFixFields = new QLabel();
  m_pLblFixFields->setDisabled(true);

  m_pLblFixMode = new QLabel();
  m_pLblFixMode->setDisabled(true);

  m_pLblFixTime = new QLabel();
  m_pLblFixTime->setDisabled(true);

  m_pLblFixSatUse = new QLabel();
  m_pLblFixSatUse->setDisabled(true);

  m_pLblFixLat = new QLabel();
  m_pLblFixLat->setDisabled(true);

  m_pLblFixLong = new QLabel();
  m_pLblFixLong->setDisabled(true);

  m_pLblFixAlt = new QLabel();
  m_pLblFixAlt->setDisabled(true);

  m_pLblFixTrack = new QLabel();
  m_pLblFixTrack->setDisabled(true);

  m_pLblFixSpeed = new QLabel();
  m_pLblFixSpeed->setDisabled(true);

  m_pLblTimeFixed = new QLabel();
  m_pLblTimeFixed->setDisabled(true);

  m_pLblFixesWritten = new QLabel();
  m_pLblFixesWritten->setDisabled(true);

  m_pLblLastWrittenFixTime = new QLabel();
  m_pLblLastWrittenFixTime->setDisabled(true);

  m_pLblHorizEp = new QLabel();
  m_pLblHorizEp->setDisabled(true);

  m_pLblMeansOfTransport = new QLabel();
  m_pLblMeansOfTransport->setDisabled(true);

  m_pLblBattery = new QLabel();
  m_pLblBattery->setDisabled(true);

  m_pBtnPauseResume = new QPushButton(QIcon(*App::instance()->pixStart()), QString());
  this->connect(m_pBtnPauseResume, SIGNAL(clicked()), SLOT(onClickedPauseResume()));

  m_pBtnSnap = new QPushButton(QIcon(*App::instance()->pixSnap()), QString());
  m_pBtnSnap->setEnabled(false);
  this->connect(m_pBtnSnap, SIGNAL(clicked()), SLOT(onClickedSnap()));

  m_pBtnMOT = new QPushButton(QIcon(*App::instance()->pixMeansOfTransport()), QString());
  m_pBtnMOT->setEnabled(false);
  this->connect(m_pBtnMOT, SIGNAL(clicked()), SLOT(onClickedMeansOfTransport()));

  m_pBtnCell = new QPushButton(tr("CELL"));
  m_pBtnCell->setEnabled(false);
  this->connect(m_pBtnCell, SIGNAL(clicked()), SLOT(onClickedCell()));
}

//---------------------------------------------------------------------------
// showHome
//---------------------------------------------------------------------------
void WndMain::showHome (void)
{
  // TODO : wallpaper + app name + app version + "record" button
  this->showFix();
}

//---------------------------------------------------------------------------
// showFix
//---------------------------------------------------------------------------
void WndMain::showFix (void)
{
  QWidget*     pWidget  = new QWidget();
  QGridLayout* pGrid    = new QGridLayout();
  QFormLayout* pForm1   = new QFormLayout();
  QFormLayout* pForm2   = new QFormLayout();
  QFormLayout* pForm3   = new QFormLayout();
  QFormLayout* pForm4   = new QFormLayout();
  QWidget*     pBlank   = new QWidget();
  QHBoxLayout* pButtons = new QHBoxLayout();

  pForm1->setSpacing(8);
  pForm1->addRow(tr("Status :"), m_pLblStatus);
  pForm1->addRow(tr("Fields :"), m_pLblFixFields);
  pForm1->addRow(tr("Mode :"),   m_pLblFixMode);
  pForm1->addRow(tr("Time :"),   m_pLblFixTime);
  pForm1->addRow(tr("SatUse :"), m_pLblFixSatUse);

  pForm2->setSpacing(8);
  pForm2->addRow(tr("Lat :"),    m_pLblFixLat);
  pForm2->addRow(tr("Long :"),   m_pLblFixLong);
  pForm2->addRow(tr("Alt :"),    m_pLblFixAlt);
  pForm2->addRow(tr("Track :"),  m_pLblFixTrack);
  pForm2->addRow(tr("Speed :"),  m_pLblFixSpeed);

  pForm3->setSpacing(8);
  pForm3->addRow(tr("Time setup :"), m_pLblTimeFixed);
  pForm3->addRow(tr("Waypoints :"), m_pLblFixesWritten);
  pForm3->addRow(tr("Last waypoint :"), m_pLblLastWrittenFixTime);

  pForm4->setSpacing(8);
  pForm4->addRow(tr("HorizEP :"), m_pLblHorizEp);
  pForm4->addRow(tr("Transport :"), m_pLblMeansOfTransport);
  pForm4->addRow(tr("Battery :"), m_pLblBattery);

  {
    QPushButton* pBtnSat   = new QPushButton(tr("SAT"));
    QPushButton* pBtnSpeed = new QPushButton(tr("SPD"));

    this->connect(pBtnSat, SIGNAL(clicked()), SLOT(onClickedSat()));
    this->connect(pBtnSpeed, SIGNAL(clicked()), SLOT(onClickedSpeed()));

    pButtons->setSpacing(5);
    pButtons->addWidget(m_pLblStatusIcon);
    pButtons->addWidget(m_pBtnPauseResume);
    pButtons->addWidget(m_pBtnSnap);
    pButtons->addWidget(m_pBtnMOT);
    pButtons->addWidget(pBtnSat);
    pButtons->addWidget(pBtnSpeed);
    pButtons->addWidget(m_pBtnCell);
  }

  pGrid->setHorizontalSpacing(5);
  pGrid->addLayout(pForm1,   0, 0);
  pGrid->addLayout(pForm2,   0, 1);
  pGrid->addWidget(pBlank,   1, 0);
  pGrid->addLayout(pForm3,   2, 0);
  pGrid->addLayout(pForm4,   2, 1);
  pGrid->addLayout(pButtons, 3, 0, 1, 2, Qt::AlignBottom);

  pWidget->setLayout(pGrid);
  this->setCentralWidget(pWidget);
}

//---------------------------------------------------------------------------
// clearFixFields
//---------------------------------------------------------------------------
void WndMain::clearFixFields (void)
{
  m_pLblFixFields->clear();
  m_pLblFixMode->clear();
  m_pLblFixTime->clear();
  m_pLblFixLat->clear();
  m_pLblFixLong->clear();
  m_pLblFixAlt->clear();
  m_pLblFixTrack->clear();
  m_pLblFixSpeed->clear();
  m_pLblFixSatUse->clear();

  m_pLblTimeFixed->clear();
  m_pLblFixesWritten->clear();
  m_pLblLastWrittenFixTime->clear();

  m_pLblHorizEp->clear();
  //m_pLblMeansOfTransport->clear();
  //m_pLblBattery->clear();
}



//---------------------------------------------------------------------------
// onClickedStartStop
//---------------------------------------------------------------------------
void WndMain::onClickedStartStop (void)
{
  App* pApp = App::instance();

  if (pApp->getState() == App::STATE_STARTED ||
      pApp->getState() == App::STATE_PAUSED)
  {
    //this->clearFixFields();

    pApp->setState(App::STATE_STOPPED);
    m_pLblStatus->setText(tr("Stopped"));

    m_pMenuStartStop->setText(tr("Start"));
    m_pBtnPauseResume->setIcon(QIcon(*pApp->pixStart()));
    m_pBtnSnap->setEnabled(false);
    m_pBtnMOT->setEnabled(false);
    m_pMenuConvert->setEnabled(true);
  }
  else if (pApp->getState() == App::STATE_STOPPED)
  {
    if (!pApp->setState(App::STATE_STARTED))
      return;

    this->clearFixFields();
    m_pBtnCell->setEnabled(false);
    m_pLblStatus->setText(tr("Started"));

    m_pMenuStartStop->setText(tr("Stop"));
    m_pBtnPauseResume->setIcon(QIcon(*pApp->pixPause()));
    m_pBtnSnap->setEnabled(true);
    m_pBtnMOT->setEnabled(true);
    m_pMenuConvert->setEnabled(false);
  }
  else
  {
    Q_ASSERT(0);
  }
}

//---------------------------------------------------------------------------
// onClickedPauseResume
//---------------------------------------------------------------------------
void WndMain::onClickedPauseResume (void)
{
  App* pApp = App::instance();

  if (pApp->getState() == App::STATE_STOPPED)
  {
    this->onClickedStartStop();
  }
  else if (pApp->getState() == App::STATE_STARTED)
  {
    if (!pApp->setState(App::STATE_PAUSED))
      return;

    m_pBtnPauseResume->setIcon(QIcon(*pApp->pixStart()));
    m_pBtnSnap->setEnabled(false);
    m_pBtnMOT->setEnabled(false);
  }
  else if (pApp->getState() == App::STATE_PAUSED)
  {
    if (!pApp->setState(App::STATE_STARTED))
      return;

    m_pBtnPauseResume->setIcon(QIcon(*pApp->pixPause()));
    m_pBtnSnap->setEnabled(true);
    m_pBtnMOT->setEnabled(true);
  }
}

//---------------------------------------------------------------------------
// onClickedSnap
//---------------------------------------------------------------------------
void WndMain::onClickedSnap (void)
{
  GPSRFile* pGPSRFile = App::instance()->outFile();
  QString   strName;
  time_t    uiTime = time(0);
  bool      bOk;

  Q_ASSERT(pGPSRFile);
  Q_ASSERT(pGPSRFile->isOpen());
  Q_ASSERT(pGPSRFile->isWriting());
  if (!pGPSRFile || !pGPSRFile->isOpen() || !pGPSRFile->isWriting())
    return;

  if (App::instance()->settings()->getAskPointName())
  {
    // ask for point name
    bOk = false;
    strName = QInputDialog::getText(
      this,
      tr("Point name ?"),
      tr("Please enter point name or leave blank :"),
      QLineEdit::Normal,
      "",
      &bOk).trimmed();
  }
  else
  {
    bOk = true;
  }

  if (!bOk)
  {
    QMaemo5InformationBox::information(
      this,
      tr("Action canceled !"));
  }
  else
  {
    // snap point
    pGPSRFile->writeNamedSnap(uiTime, qPrintable(strName));

    // reset last fix time to get new fix as soon as possible
    App::instance()->resetFixTime();

    // inform user
    if (!strName.isEmpty())
    {
      strName.prepend(" <i>");
      strName.append("</i>");
    }
    QMaemo5InformationBox::information(
      this,
      QString(tr("Snapped position%1 at %2 !")).arg(strName).arg(Util::timeString().constData()));
  }
}

//---------------------------------------------------------------------------
// onClickedConfig
//---------------------------------------------------------------------------
void WndMain::onClickedConfig (void)
{
  WndConfig wndConfig(this);
  wndConfig.exec();
}

//---------------------------------------------------------------------------
// onClickedConvert
//---------------------------------------------------------------------------
void WndMain::onClickedConvert (void)
{
  WndConvert wndConvert(this);
  wndConvert.exec();

  if (!App::instance()->location()->isStarted())
    this->clearFixFields();
}

//---------------------------------------------------------------------------
// onClickedMeansOfTransport
//---------------------------------------------------------------------------
void WndMain::onClickedMeansOfTransport (void)
{
  AppSettings& settings = *App::instance()->settings();
  WndMeansOfTransport wndMOT(this);
  bool bOk;
  QString strOtherLabel;

  Q_ASSERT(App::instance()->getState() == App::STATE_STARTED);
  if (App::instance()->getState() != App::STATE_STARTED)
    return;

  bOk = wndMOT.doExec(
    settings.getLastMeansOfTransport(),
    settings.getLastOtherMeansOfTransport().constData());

  if (!bOk)
  {
    QMaemo5InformationBox::information(
      this,
      tr("Action canceled !"));
    return;
  }

  if (wndMOT.meansOfTransport() == GPSRFile::MEANSTRANSPORT_OTHER)
    strOtherLabel = wndMOT.otherMeansOfTransportName();

  App::instance()->outFile()->writeMeansOfTransport(
    time(0),
    wndMOT.meansOfTransport(),
    qPrintable(strOtherLabel));

  this->setMeansOfTransport(
    settings.getLastMeansOfTransport(),
    settings.getLastOtherMeansOfTransport().constData() );
}

//---------------------------------------------------------------------------
// onClickedSat
//---------------------------------------------------------------------------
void WndMain::onClickedSat (void)
{
  App::instance()->wndSat()->show();
}

//---------------------------------------------------------------------------
// onClickedSpeed
//---------------------------------------------------------------------------
void WndMain::onClickedSpeed (void)
{
  App::instance()->wndSpeed()->show();
}

//---------------------------------------------------------------------------
// onClickedCell
//---------------------------------------------------------------------------
void WndMain::onClickedCell (void)
{
  App::instance()->wndCell()->show();
}

//---------------------------------------------------------------------------
// onClickedAbout
//---------------------------------------------------------------------------
void WndMain::onClickedAbout (void)
{
  App::instance()->wndAbout()->show();
}



//---------------------------------------------------------------------------
// onAppStatePixChanged
//---------------------------------------------------------------------------
void WndMain::onAppStatePixChanged (QPixmap* pNewStatePixmap)
{
  m_pLblStatusIcon->setPixmap(*pNewStatePixmap);
}

//---------------------------------------------------------------------------
// onLocationFix
//---------------------------------------------------------------------------
void WndMain::onLocationFix (Location* pLocation, const LocationFixContainer* pFixCont, bool bAccurate)
{
  const LocationFix& fix = *pFixCont->getFix();
  AppSettings& settings = *App::instance()->settings();
  QString str;

  this->setUpdatesEnabled(false);

  // status
  str.sprintf("%s, %s",
    App::instance()->getStateStr(),
    (bAccurate ? QT_TR_NOOP("Fixed") : pLocation->isAcquiring() ? QT_TR_NOOP("Acquiring") : QT_TR_NOOP("Lost")) );
  m_pLblStatus->setText(str);

  // fix fields
  str.clear();
  if (fix.wFixFields != FIXFIELD_NONE)
  {
    if (fix.hasFields(FIXFIELD_TIME))
      str += tr("Time ");
    if (fix.hasFields(FIXFIELD_LATLONG))
      str += tr("LatLong ");
    if (fix.hasFields(FIXFIELD_ALT))
      str += tr("Alt ");
    if (fix.hasFields(FIXFIELD_TRACK))
      str += tr("Track ");
    if (fix.hasFields(FIXFIELD_SPEED))
      str += tr("Speed ");
    if (fix.hasFields(FIXFIELD_CLIMB))
      str += tr("Climb ");
  }
  m_pLblFixFields->setText(str);

  // fix mode
  m_pLblFixMode->setText(fix.getModeStr());

  // fix time
  str.clear();
  if (fix.uiTime > 0)
    str = Util::timeString(false, fix.uiTime);
  m_pLblFixTime->setText(str);

  // fix properties
  m_pLblFixSatUse->setText(QString::number(fix.cSatUse) + " / " + QString::number(fix.cSatCount));
  m_pLblFixLat->setText(QString::number((std::isnan(fix.getLatDeg()) ? 0.0 : fix.getLatDeg()), 'f', 6) + QChar(L'\x00b0'));
  m_pLblFixLong->setText(QString::number((std::isnan(fix.getLongDeg()) ? 0.0 : fix.getLongDeg()), 'f', 6) + QChar(L'\x00b0'));
  m_pLblFixAlt->setText(QString::number(fix.hasFields(FIXFIELD_ALT) ? fix.getAlt(settings.getUnitSystem()) : 0.0) + " " + QString(fix.getAltSuffix(settings.getUnitSystem())));
  m_pLblFixTrack->setText(QString::number((std::isnan(fix.getTrackDeg()) ? 0.0 : fix.getTrackDeg()), 'f', 1) + QChar(L'\x00b0'));
  m_pLblFixSpeed->setText(QString::number((fix.hasFields(FIXFIELD_SPEED) ? fix.getSpeed(settings.getHorizSpeedUnit()) : 0.0), 'f', 2) + " " + QString(fix.getSpeedSuffix(settings.getHorizSpeedUnit())));
  m_pLblHorizEp->setText(QString::number((std::isnan(fix.getHorizEp(settings.getUnitSystem())) ? 0.0 : fix.getHorizEp(settings.getUnitSystem())), 'f', 2) + " " + QString(fix.getHorizEpSuffix(settings.getUnitSystem())));

  // miscellaneous
  m_pLblTimeFixed->setText(App::instance()->lastTimeSetup() == 0 ? tr("NO") : tr("YES"));
  m_pLblFixesWritten->setText(QString::number(App::instance()->fixesWritten()));
  if (!App::instance()->lastWrittenFixTime())
    m_pLblLastWrittenFixTime->clear();
  else
    m_pLblLastWrittenFixTime->setText(Util::timeString(false, App::instance()->lastWrittenFixTime()));

  // enable cell tower button if we received cell mode info
  if (fix.sGSM.bSetup || fix.sWCDMA.bSetup)
    m_pBtnCell->setEnabled(true);


  this->setUpdatesEnabled(true);
}

//---------------------------------------------------------------------------
// onBatteryStatusChanged
//---------------------------------------------------------------------------
void WndMain::onBatteryStatusChanged (int iCurrent, int iLastFull, int iDesign, bool bCharging)
{
  QString strStatus;

  //qDebug("Battery charge : curr:%i lastfull:%i design:%i ischarg:%i",
  //  iCurrent, iLastFull, iDesign, bCharging?1:0);

  if (bCharging)
  {
    strStatus = tr("Charging");
  }
  else
  {
    double rPercent = double(iCurrent) * 100.0 / double(iDesign);
    strStatus = QString::number(rPercent, 'f', 1) + "%";
  }

  m_pLblBattery->setText(strStatus);
}
