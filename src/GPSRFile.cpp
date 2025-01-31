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
// Created On : 2010-03-29 14:03:23
//
// $Id: GPSRFile.cpp 230 2013-05-10 09:28:15Z jcl $
//
//***************************************************************************

#include "stable.h"

#if Q_BYTE_ORDER != Q_LITTLE_ENDIAN
#error GPSRFile class does not support BIG endian platforms yet !
#endif


//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------
static const struct
{
  quint16     id;
  const char* label;
}
CHUNKNAMES[] =
{
  { GPSRFile::CHUNK_MESSAGE,          "Message" },
  { GPSRFile::CHUNK_LOCATIONFIX,      "LocationFix" },
  { GPSRFile::CHUNK_LOCATIONFIX_LOST, "LocationFixLost" },
  { GPSRFile::CHUNK_SNAP,             "Snap" },
  { GPSRFile::CHUNK_NAMEDSNAP,        "NamedSnap" },
  { GPSRFile::CHUNK_PAUSED,           "Paused" },
  { GPSRFile::CHUNK_RESUMED,          "Resumed" },
  { GPSRFile::CHUNK_NEWTRACK,         "NewTrack" },
  { GPSRFile::CHUNK_MEANSTRANSPORT,   "MeansOfTransport" },
};

static const int CHUNKNAMES_COUNT = sizeof(CHUNKNAMES) / sizeof(CHUNKNAMES[0]);

static const struct
{
  quint8      id;
  const char* label;
}
TRANSPORTNAMES[] =
{
  { GPSRFile::MEANSTRANSPORT_UNKNOWN,      QT_TR_NOOP("N/A") },

  { GPSRFile::MEANSTRANSPORT_FOOT,         QT_TR_NOOP("Foot") },
  { GPSRFile::MEANSTRANSPORT_ROLLER,       QT_TR_NOOP("Roller") },
  { GPSRFile::MEANSTRANSPORT_SKATEBOARD,   QT_TR_NOOP("Skateboard") },
  { GPSRFile::MEANSTRANSPORT_BIKE,         QT_TR_NOOP("Bike") },

  { GPSRFile::MEANSTRANSPORT_SKI,          QT_TR_NOOP("Ski") },
  { GPSRFile::MEANSTRANSPORT_SNOWBOARD,    QT_TR_NOOP("Snowboard") },

  { GPSRFile::MEANSTRANSPORT_CAR,          QT_TR_NOOP("Car") },
  { GPSRFile::MEANSTRANSPORT_MOTORBIKE,    QT_TR_NOOP("Motorbike") },
  { GPSRFile::MEANSTRANSPORT_SCOOTER,      QT_TR_NOOP("Scooter") },
  { GPSRFile::MEANSTRANSPORT_TRUCK,        QT_TR_NOOP("Truck") },
  { GPSRFile::MEANSTRANSPORT_TAXI,         QT_TR_NOOP("Taxi") },
  { GPSRFile::MEANSTRANSPORT_BUS,          QT_TR_NOOP("Bus") },
  { GPSRFile::MEANSTRANSPORT_SUBWAY,       QT_TR_NOOP("Subway") },
  { GPSRFile::MEANSTRANSPORT_TRAMWAY,      QT_TR_NOOP("Tramway") },
  { GPSRFile::MEANSTRANSPORT_TUKTUK,       QT_TR_NOOP("Tuk Tuk") },
  { GPSRFile::MEANSTRANSPORT_TRAIN,        QT_TR_NOOP("Train") },

  { GPSRFile::MEANSTRANSPORT_HORSE,        QT_TR_NOOP("Horse") },
  { GPSRFile::MEANSTRANSPORT_PONY,         QT_TR_NOOP("Pony") },
  { GPSRFile::MEANSTRANSPORT_DONKEY,       QT_TR_NOOP("Donkey") },
  { GPSRFile::MEANSTRANSPORT_CAMEL,        QT_TR_NOOP("Camel") },
  { GPSRFile::MEANSTRANSPORT_CALASH,       QT_TR_NOOP("Calash") },
  { GPSRFile::MEANSTRANSPORT_SULKY,        QT_TR_NOOP("Sulky") },

  { GPSRFile::MEANSTRANSPORT_BOAT,         QT_TR_NOOP("Boat") },
  { GPSRFile::MEANSTRANSPORT_JETSKI,       QT_TR_NOOP("Jet Ski") },
  { GPSRFile::MEANSTRANSPORT_SAILINGBOAT,  QT_TR_NOOP("Sailing Boat") },
  { GPSRFile::MEANSTRANSPORT_BARGE,        QT_TR_NOOP("Barge") },
  { GPSRFile::MEANSTRANSPORT_SHIP,         QT_TR_NOOP("Ship") },
  { GPSRFile::MEANSTRANSPORT_FERRY,        QT_TR_NOOP("Ferry") },
  { GPSRFile::MEANSTRANSPORT_CARGO,        QT_TR_NOOP("Cargo") },

  { GPSRFile::MEANSTRANSPORT_PLANE,        QT_TR_NOOP("Plane") },
  { GPSRFile::MEANSTRANSPORT_SEAPLANE,     QT_TR_NOOP("Seaplane") },
  { GPSRFile::MEANSTRANSPORT_GLIDER,       QT_TR_NOOP("Glider") },
  { GPSRFile::MEANSTRANSPORT_ULTRALIGHT,   QT_TR_NOOP("Ultralight") },
  { GPSRFile::MEANSTRANSPORT_HELICOPTER,   QT_TR_NOOP("Helicopter") },
  { GPSRFile::MEANSTRANSPORT_GYROCOPTER,   QT_TR_NOOP("Gyrocopter") },
  { GPSRFile::MEANSTRANSPORT_AIRSHIP,      QT_TR_NOOP("Airship") },
  { GPSRFile::MEANSTRANSPORT_FREEFALL,     QT_TR_NOOP("Free Fall") },
  { GPSRFile::MEANSTRANSPORT_PARAGLIDER,   QT_TR_NOOP("Paraglider") },
  { GPSRFile::MEANSTRANSPORT_DELTAPLANE,   QT_TR_NOOP("Deltaplane") },
  { GPSRFile::MEANSTRANSPORT_BALLOON,      QT_TR_NOOP("Balloon") },
  { GPSRFile::MEANSTRANSPORT_ZEPPELIN,     QT_TR_NOOP("Zeppelin") },

  { GPSRFile::MEANSTRANSPORT_OTHER,        QT_TR_NOOP("Other") },
  { GPSRFile::MEANSTRANSPORT_FLYINGSAUCER, QT_TR_NOOP("Flying Saucer") },
};

static const int TRANSPORTNAMES_COUNT = sizeof(TRANSPORTNAMES) / sizeof(TRANSPORTNAMES[0]);



//---------------------------------------------------------------------------
// GPSRFile
//---------------------------------------------------------------------------
GPSRFile::GPSRFile (void)
{
  m_pFile = 0;

  m_bDiscardAfterClose = false;
  m_bWriting           = false;
  m_uiAppendOffset     = 0;
  m_bError             = false;
  m_bEOF               = false;
  m_bIncomplete        = false;

  m_bDiscoveryRead = false;
  m_nReadIndex     = -1;
}

//---------------------------------------------------------------------------
// ~GPSRFile
//---------------------------------------------------------------------------
GPSRFile::~GPSRFile (void)
{
  this->close();
}



//---------------------------------------------------------------------------
// close
//---------------------------------------------------------------------------
void GPSRFile::close (void)
{
  if (m_pFile)
  {
    if (m_bWriting && m_bDiscardAfterClose)
    {
      if (m_uiAppendOffset > 0)
      {
        // here, we want to truncate this file since no location fix was
        // written in it. file is not deleted because we are writing in
        // 'append' mode so we want to restore the file as it was before
        // we opened it.

        qDebug("Truncating %s because track was inconsistent (no location fix) !", m_strFilePath.constData());
        rewind(m_pFile);
        if (ftruncate(fileno(m_pFile), m_uiAppendOffset) != 0)
          qWarning("Failed to truncate %s at offset %u ! Error %d : %s", m_strFilePath.constData(), m_uiAppendOffset, errno, strerror(errno));

        fclose(m_pFile);
        m_pFile = 0;
      }
      else
      {
        // here, we want to delete this file since no location fix was
        // written in it.

        qDebug("Deleting %s because track was inconsistent (no location fix) !", m_strFilePath.constData());
        fclose(m_pFile);
        m_pFile = 0;
        QFile::remove(m_strFilePath);
      }
    }

    if (m_pFile)
    {
      if (m_bWriting)
        qDebug("Closing %s", m_strFilePath.constData());
      fclose(m_pFile);
      m_pFile = 0;
    }
  }

  m_pFile = 0;
  m_strFilePath.clear();

  // we do not reset the m_bDiscardAfterClose flag because it can be read
  // between a close() and an open*() calls with the discardedAfterClose()
  // method.
  //m_bDiscardAfterClose = false;

  m_bWriting          = false;
  m_uiAppendOffset    = 0;
  m_bError            = false;
  m_bEOF              = false;
  m_bIncomplete       = false;

  m_bDiscoveryRead = false;
  m_nReadIndex     = -1;
  m_vecReadChunks.clear();
}

//---------------------------------------------------------------------------
// openNew
//---------------------------------------------------------------------------
bool GPSRFile::openNew (const char* pszFile, const char* pszTrackName, bool bForceReplace)
{
  Q_ASSERT(this->isOpen() == false);
  if (this->isOpen())
    return false;

  if (!bForceReplace && Util::fileExists(pszFile))
  {
    m_bError = true;
    return false;
  }

  m_pFile = fopen(pszFile, "wb");
  if (!m_pFile)
  {
    m_bError = true;
    qWarning("Could not create %s ! Error %d : %s", pszFile, errno, strerror(errno));
    return false;
  }

  m_strFilePath        = pszFile;
  m_bDiscardAfterClose = true;
  m_bWriting           = true;
  m_uiAppendOffset     = 0;
  m_bError             = false;

  // write header
  {
    Header* pHeader;

    m_Swap.reserve(sizeof(*pHeader) +1);
    pHeader = (Header*)m_Swap.data();

    pHeader->aucMagic[0]     = 'G';
    pHeader->aucMagic[1]     = 'P';
    pHeader->aucMagic[2]     = 'S';
    pHeader->aucMagic[3]     = 'R';
    pHeader->ucFormat        = FORMAT_VERSION;
    pHeader->uiTime          = time(0);
    pHeader->iTimeZoneOffset = Util::timeZoneOffset();

    this->writeData((char*)pHeader, sizeof(*pHeader));
  }

  // write 'new track' chunk
  this->writeNewTrack(time(0), pszTrackName);

  qDebug("Created %s", pszFile);

  return true;
}

//---------------------------------------------------------------------------
// openAppend
//---------------------------------------------------------------------------
bool GPSRFile::openAppend (const char* pszFile, const char* pszTrackName)
{
  uint uiValidSize = 0;

  Q_ASSERT(this->isOpen() == false);
  if (this->isOpen())
    return false;

  // check existing file first :
  // * if file does not exist or is invalid, return error.
  // * if file is incomplete, truncate right behind the last valid chunk we
  //   found.
  {
    GPSRFile gpsrfile;

    if (!gpsrfile.openRead(pszFile))
      return false;

    if (gpsrfile.getReadChunksCount() > 0)
    {
      const ChunkReadInfo& criLast = gpsrfile.getReadChunksInfo().last();
      uiValidSize = criLast.uiOffset + criLast.uiSize;
    }

    gpsrfile.close();
  }

  // if no valid chunks were found in existing file, truncate it completely
  if (uiValidSize == 0)
  {
    qWarning("No valid chunk found in %s ! Creating a new file.", pszFile);
    return this->openNew(pszFile, pszTrackName, true);
  }

  // otherwise, open file to append new data
  m_pFile = fopen(pszFile, "ab");
  if (!m_pFile)
  {
    m_bError = true;
    qWarning("Could not create %s ! Error %d : %s", pszFile, errno, strerror(errno));
    return false;
  }
  rewind(m_pFile);

  // overwrite format version in the existing file header
  {
    size_t uiOffset = 4;
    quint8 ucFormat = FORMAT_VERSION;

    if (fseek(m_pFile, uiOffset, SEEK_SET) < 0)
    {
      qWarning("Failed to fseek in %s at offset %u ! Error %d : %s", pszFile, uiOffset, errno, strerror(errno));
      fclose(m_pFile);
      m_pFile = NULL;
      return false;
    }

    if (fwrite(&ucFormat, 1, 1, m_pFile) != 1)
    {
      qWarning("Failed to overwrite format version to %d in %s at offset %u ! Error %d : %s", ucFormat, pszFile, uiOffset, errno, strerror(errno));
      fclose(m_pFile);
      m_pFile = NULL;
      return false;
    }
  }

  // truncate file
  rewind(m_pFile);
  if (ftruncate(fileno(m_pFile), uiValidSize) != 0)
  {
    qWarning("Failed to truncate %s at offset %u ! Error %d : %s", pszFile, uiValidSize, errno, strerror(errno));
    fclose(m_pFile);
    m_pFile = NULL;
    return false;
  }

  // seek to end
  if (fseek(m_pFile, (int)uiValidSize, SEEK_SET) < 0)
  {
    qWarning("Failed to fseek in %s at offset %u ! Error %d : %s", pszFile, uiValidSize, errno, strerror(errno));
    fclose(m_pFile);
    m_pFile = NULL;
    return false;
  }

  // init members
  m_strFilePath        = pszFile;
  m_bDiscardAfterClose = true;
  m_bWriting           = true;
  m_uiAppendOffset     = uiValidSize;
  m_bError             = false;

  // write 'new track' chunk
  this->writeNewTrack(time(0), pszTrackName);

  qDebug("Appending to %s", pszFile);

  return true;
}

//---------------------------------------------------------------------------
// openRead
//---------------------------------------------------------------------------
bool GPSRFile::openRead (const char* pszFile)
{
  if (this->isOpen())
  {
    if (!this->isWriting())
      return true;
    this->close();
  }

  // open files
  m_pFile = fopen(pszFile, "rb");
  if (!m_pFile)
  {
    m_bError = true;
    qWarning("Could not open %s ! Error %d : %s", pszFile, errno, strerror(errno));
    return false;
  }
  //qDebug("Opened %s", pszFile);

  // setup members
  m_strFilePath        = pszFile;
  m_bDiscardAfterClose = false;
  m_bWriting           = false;
  m_uiAppendOffset     = 0;
  m_bError             = false;
  m_bEOF               = false;

  // first, silently read the entire file in one pass to discover its content
  // and to give us the ability to navigate easily through chunks
  {
    bool bRes;

    m_bDiscoveryRead = true;
    m_nReadIndex     = -1;
    m_vecReadChunks.clear();

    if (!this->seekFirst())
      return false;

    while (bRes = this->readNext()) { ; }
    if (!bRes && m_bError)
      return false;

    m_bDiscoveryRead = false;
  }

  return this->seekFirst();
}



//---------------------------------------------------------------------------
// writeMessage
//---------------------------------------------------------------------------
void GPSRFile::writeMessage (time_t uiTime, const char* pszMessage)
{
  this->writeChunkAsciiz(CHUNK_MESSAGE, uiTime, pszMessage);
}

//---------------------------------------------------------------------------
// writeLocationFix
//---------------------------------------------------------------------------
void GPSRFile::writeLocationFix (time_t uiTime, const LocationFixContainer& fixCont)
{
  Chunk* pChunk;
  uint   uiChunkSize;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(this->isWriting());
  if (!this->isOpen() || !this->isWriting())
    return;

  Q_ASSERT(fixCont.hasFix());
  if (!fixCont.hasFix())
    return;

  uiChunkSize = sizeof(Chunk) + fixCont.getFixSize();

  m_Swap.reserve(uiChunkSize +1);
  pChunk = (Chunk*)m_Swap.data();

  pChunk->ucMagic = '@';
  pChunk->uiId    = CHUNK_LOCATIONFIX;
  pChunk->uiSize  = uiChunkSize;
  pChunk->uiTime  = uiTime ? uiTime : time(0);

  memcpy(pChunk->aData, fixCont.getFix(), fixCont.getFixSize());

  this->writeData((char*)pChunk, uiChunkSize);

  // this tells to the close() method not to delete or discard this track
  // because it is now consistent since we've got at least one location fix
  m_bDiscardAfterClose = false;
}

//---------------------------------------------------------------------------
// writeLocationFixLost
//---------------------------------------------------------------------------
void GPSRFile::writeLocationFixLost (time_t uiTime)
{
  this->writeChunkSimple(CHUNK_LOCATIONFIX_LOST, uiTime);
}

//---------------------------------------------------------------------------
// writeSnap
//---------------------------------------------------------------------------
#if 0 // obsolete
void GPSRFile::writeSnap (time_t uiTime)
{
  this->writeChunkSimple(CHUNK_SNAP, uiTime);
}
#endif

//---------------------------------------------------------------------------
// writeNamedSnap
//---------------------------------------------------------------------------
void GPSRFile::writeNamedSnap (time_t uiTime, const char* pszName)
{
  this->writeChunkAsciiz(CHUNK_NAMEDSNAP, uiTime, pszName);
}

//---------------------------------------------------------------------------
// writePaused
//---------------------------------------------------------------------------
void GPSRFile::writePaused (time_t uiTime, const char* pszName)
{
  this->writeChunkAsciiz(CHUNK_PAUSED, uiTime, pszName);
}

//---------------------------------------------------------------------------
// writeResumed
//---------------------------------------------------------------------------
void GPSRFile::writeResumed (time_t uiTime)
{
  this->writeChunkSimple(CHUNK_RESUMED, uiTime);
}

//---------------------------------------------------------------------------
// writeNewTrack
//---------------------------------------------------------------------------
void GPSRFile::writeNewTrack (time_t uiTime, const char* pszName)
{
  Chunk* pChunk;
  uint   uiChunkSize;
  qint32 iTimeZoneOffset;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(this->isWriting());
  if (!this->isOpen() || !this->isWriting())
    return;

  uiChunkSize  = sizeof(Chunk);
  uiChunkSize += sizeof(qint32); // time zone offset
  uiChunkSize += pszName ? strlen(pszName) : 0;
  uiChunkSize += 1; // +1 for the trailing '\0'

  m_Swap.reserve(uiChunkSize +1);
  pChunk = (Chunk*)m_Swap.data();

  pChunk->ucMagic = '@';
  pChunk->uiId    = CHUNK_NEWTRACK;
  pChunk->uiSize  = uiChunkSize;
  pChunk->uiTime  = uiTime ? uiTime : time(0);

  // data format :
  // 0 : time zone offset
  // 4-var : asciiz, optional label
  {
    quint8* pui = (quint8*)&pChunk->aData[0];

    *((qint32*)pui) = Util::timeZoneOffset();
    pui += sizeof(qint32);

    if (pszName)
      memcpy((char*)pui, pszName, strlen(pszName) + 1);
    else
      pui[0] = 0;
  }

  this->writeData((char*)pChunk, uiChunkSize);
}

//---------------------------------------------------------------------------
// writeMeansOfTransport
//---------------------------------------------------------------------------
void GPSRFile::writeMeansOfTransport (time_t uiTime, quint8 ucMeansOfTransport, const char* pszOptionalLabel)
{
  Chunk* pChunk;
  uint   uiChunkSize;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(this->isWriting());
  if (!this->isOpen() || !this->isWriting())
    return;

  uiChunkSize  = sizeof(Chunk);
  uiChunkSize += sizeof(ucMeansOfTransport);
  uiChunkSize += pszOptionalLabel ? strlen(pszOptionalLabel) : 0;
  uiChunkSize += 1; // +1 for the trailing '\0'

  m_Swap.reserve(uiChunkSize +1);
  pChunk = (Chunk*)m_Swap.data();

  pChunk->ucMagic = '@';
  pChunk->uiId    = CHUNK_MEANSTRANSPORT;
  pChunk->uiSize  = uiChunkSize;
  pChunk->uiTime  = uiTime ? uiTime : time(0);

  // data format :
  // 0 : means of transport byte
  // 1-var : asciiz, optional label
  pChunk->aData[0] = ucMeansOfTransport;
  if (pszOptionalLabel)
    memcpy((char*)&pChunk->aData[1], pszOptionalLabel, strlen(pszOptionalLabel) + 1);
  else
    pChunk->aData[1] = 0;

  this->writeData((char*)pChunk, uiChunkSize);
}



//---------------------------------------------------------------------------
// seekFirst
//---------------------------------------------------------------------------
bool GPSRFile::seekFirst (void)
{
  Header* pHeader;

  if (m_bError)
    return false;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(!this->isWriting());
  if (!this->isOpen() || this->isWriting())
  {
    this->signalReadError(ERROR_NOTOPEN);
    return false;
  }

  // seek to start
  rewind(m_pFile);
  if (ferror(m_pFile) || feof(m_pFile))
  {
    if (feof(m_pFile))
      m_bEOF = true;
    this->signalReadError(ERROR_READ);
    return false;
  }
  m_bEOF = false;
  m_nReadIndex = -1;

  // prepare swap buffer
  m_Swap.reserve(sizeof(*pHeader) + 1);
  pHeader = (Header*)m_Swap.data();

  // read file header
  if (!this->readSize((char*)pHeader, sizeof(*pHeader), true, &m_bEOF))
    return false;

  // validate header
  {
    if (pHeader->aucMagic[0] != 'G' ||
        pHeader->aucMagic[1] != 'P' ||
        pHeader->aucMagic[2] != 'S' ||
        pHeader->aucMagic[3] != 'R')
    {
      this->signalReadError(ERROR_FORMAT);
      return false;
    }

    if (pHeader->ucFormat < FORMAT_VERSION_V1)
    {
      this->signalReadError(ERROR_FORMAT_VERSION);
      return false;
    }
  }

  if (!m_bDiscoveryRead)
    emit sigReadSOF(this, pHeader->uiTime, pHeader->ucFormat, pHeader->iTimeZoneOffset);

  return true;
}

//---------------------------------------------------------------------------
// readNext
//---------------------------------------------------------------------------
bool GPSRFile::readNext (void)
{
  return this->readChunk(m_nReadIndex + 1);
}

//---------------------------------------------------------------------------
// readPrevious
//---------------------------------------------------------------------------
bool GPSRFile::readPrevious (void)
{
  Q_ASSERT(m_nReadIndex >= 1);
  if (m_nReadIndex < 1)
    return false;

  return this->readChunk(m_nReadIndex - 1);
}

//---------------------------------------------------------------------------
// readChunk
//---------------------------------------------------------------------------
bool GPSRFile::readChunk (int nChunkIndex)
{
  Chunk  chunkHeader;
  Chunk* pChunk;
  int    nChunkOffset;

  if (m_bError)
    return false;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(!this->isWriting());
  if (!this->isOpen() || this->isWriting())
  {
    this->signalReadError(ERROR_NOTOPEN);
    return false;
  }

  if (m_bDiscoveryRead)
  {
    // behave like a 'next' action

    // eof already reached ?
    if (m_bEOF)
      return false;

    Q_ASSERT(nChunkIndex == m_nReadIndex + 1);
    ++m_nReadIndex;

    nChunkOffset = (int)ftell(m_pFile);
  }
  else
  {
    Q_ASSERT(m_bDiscoveryRead == false);
    Q_ASSERT(nChunkIndex >= 0);
    if (nChunkIndex < 0 || nChunkIndex >= m_vecReadChunks.count())
    {
      if (!m_bEOF && nChunkIndex >= m_vecReadChunks.count())
      {
        m_bEOF = true;
        if (m_bIncomplete)
          this->signalReadError(ERROR_TRUNCATED);
        emit sigReadEOF(this);
      }
      return false;
    }

    if (fseek(m_pFile, (int)m_vecReadChunks[nChunkIndex].uiOffset, SEEK_SET) < 0)
    {
      this->signalReadError(ERROR_READ);
      return false;
    }
    nChunkOffset = (int)ftell(m_pFile);

    m_nReadIndex = nChunkIndex;
  }

  // read chunk header
  if (!this->readSize((char*)&chunkHeader, sizeof(chunkHeader), false, &m_bEOF))
    return false;
  if (chunkHeader.ucMagic != '@')
  {
    this->signalReadError(ERROR_FORMAT);
    return false;
  }
  m_Swap.reserve(chunkHeader.uiSize +1);
  pChunk = (Chunk*)m_Swap.data();
  memcpy(pChunk, &chunkHeader, sizeof(chunkHeader));

  // read chunk data if needed
  if (chunkHeader.uiSize > sizeof(chunkHeader))
  {
    if (!this->readSize((char*)pChunk + sizeof(chunkHeader), pChunk->uiSize - sizeof(chunkHeader), false, &m_bEOF))
      return false;
  }

  // discovery mode ?
  if (m_bDiscoveryRead)
  {
    Q_ASSERT(m_nReadIndex == m_vecReadChunks.count());
    Q_ASSERT(nChunkOffset > 0);

    if (m_nReadIndex == m_vecReadChunks.count() && nChunkOffset > 0)
    {
      ChunkReadInfo chunkReadInfo;

      chunkReadInfo.uiOffset = (uint)nChunkOffset;
      chunkReadInfo.uiId     = chunkHeader.uiId;
      chunkReadInfo.uiSize   = chunkHeader.uiSize;
      chunkReadInfo.uiTime   = chunkHeader.uiTime;

      m_vecReadChunks.push_back(chunkReadInfo);
    }
  }
  else
  {
    // signal this chunk
    switch (pChunk->uiId)
    {
      case CHUNK_MESSAGE :
        emit sigReadChunkMessage(this, pChunk->uiTime, (char*)&pChunk->aData);
        break;
      case CHUNK_LOCATIONFIX :
        emit sigReadChunkLocationFix(this, pChunk->uiTime, (LocationFix&)*(pChunk->aData));
        break;
      case CHUNK_LOCATIONFIX_LOST :
        emit sigReadChunkLocationFixLost(this, pChunk->uiTime);
        break;
      case CHUNK_SNAP :
        emit sigReadChunkSnap(this, pChunk->uiTime);
        break;
      case CHUNK_NAMEDSNAP :
        emit sigReadChunkNamedSnap(this, pChunk->uiTime, (char*)&pChunk->aData);
        break;
      case CHUNK_PAUSED :
        emit sigReadChunkPaused(this, pChunk->uiTime, (char*)&pChunk->aData);
        break;
      case CHUNK_RESUMED :
        emit sigReadChunkResumed(this, pChunk->uiTime);
        break;
      case CHUNK_NEWTRACK :
        emit sigReadChunkNewTrack(this, pChunk->uiTime, qint32(*((qint32*)&pChunk->aData[0])), (char*)&pChunk->aData[4]);
        break;
      case CHUNK_MEANSTRANSPORT :
        emit sigReadChunkMeansOfTransport(this, pChunk->uiTime, pChunk->aData[0], (char*)&pChunk->aData[1]);
        break;
      default :
        qWarning("Unknown chunk #%u @ offset %d in file %s !", (uint)pChunk->uiId, nChunkOffset, m_strFilePath.constData());
        emit sigReadChunkUnknown(this, pChunk);
        break;
    }
  }

  return true;
}



//---------------------------------------------------------------------------
// getReadChunksCount
//---------------------------------------------------------------------------
int GPSRFile::getReadChunksCount (quint16 uiChunkId) const
{
  return GPSRFile::chunksCount(m_vecReadChunks, uiChunkId);
}



//---------------------------------------------------------------------------
// writeChunkSimple
//---------------------------------------------------------------------------
void GPSRFile::writeChunkSimple (quint16 uiChunkId, time_t uiTime)
{
  Chunk* pChunk;
  uint   uiChunkSize;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(this->isWriting());
  if (!this->isOpen() || !this->isWriting())
    return;

  uiChunkSize = sizeof(Chunk);

  m_Swap.reserve(uiChunkSize +1);
  pChunk = (Chunk*)m_Swap.data();

  pChunk->ucMagic = '@';
  pChunk->uiId    = uiChunkId;
  pChunk->uiSize  = uiChunkSize;
  pChunk->uiTime  = uiTime ? uiTime : time(0);

  this->writeData((char*)pChunk, uiChunkSize);
}

//---------------------------------------------------------------------------
// writeChunkAsciiz
//---------------------------------------------------------------------------
void GPSRFile::writeChunkAsciiz (quint16 uiChunkId, time_t uiTime, const char* pszText)
{
  Chunk* pChunk;
  uint   uiChunkSize;
  uint   uiMsgSize;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(this->isWriting());
  if (!this->isOpen() || !this->isWriting())
    return;

  uiMsgSize   = (pszText ? strlen(pszText) : 0) + 1; // +1 for the trailing '\0'
  uiChunkSize = sizeof(Chunk) + uiMsgSize;

  m_Swap.reserve(uiChunkSize +1);
  pChunk = (Chunk*)m_Swap.data();

  pChunk->ucMagic = '@';
  pChunk->uiId    = uiChunkId;
  pChunk->uiSize  = uiChunkSize;
  pChunk->uiTime  = uiTime ? uiTime : time(0);

  if (pszText)
    memcpy(pChunk->aData, pszText, uiMsgSize);
  pChunk->aData[uiMsgSize - 1] = 0;

  this->writeData((char*)pChunk, uiChunkSize);
}

//---------------------------------------------------------------------------
// writeData
//---------------------------------------------------------------------------
void GPSRFile::writeData (const char* pData, uint uiSize)
{
  uint uiWritten = 0;
  uint uiRes;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(this->isWriting());
  if (!this->isOpen() || !this->isWriting())
    return;

  Q_ASSERT(pData);
  Q_ASSERT(uiSize > 0);
  if (!pData || !uiSize)
    return;

  while (uiWritten < uiSize)
  {
    clearerr(m_pFile);

    uiRes = fwrite(pData + uiWritten, 1, (uiSize - uiWritten), m_pFile);
    if (uiRes < (uiSize - uiWritten) && ferror(m_pFile))
      qFatal("Failed to write %u bytes into %s !", uiSize, m_strFilePath.constData());

    uiWritten += uiRes;
  }
}

//---------------------------------------------------------------------------
// readSize
//---------------------------------------------------------------------------
bool GPSRFile::readSize (char* pOutData, uint uiExpectedSize, bool bIsFileHeader, bool* pbGotEOF)
{
  uint uiRead = 0;
  uint uiRes;

  Q_ASSERT(this->isOpen());
  Q_ASSERT(!this->isWriting());
  if (!this->isOpen() || this->isWriting())
  {
    this->signalReadError(ERROR_NOTOPEN);
    return false;
  }

  Q_ASSERT(pOutData);
  Q_ASSERT(uiExpectedSize > 0);
  Q_ASSERT(pbGotEOF);
  if (!pOutData || !uiExpectedSize)
    qFatal("GPSRFile::readSize() : invalid parameters !");

  while (uiRead < uiExpectedSize)
  {
    clearerr(m_pFile);

    uiRes = fread(pOutData + uiRead, 1, (uiExpectedSize - uiRead), m_pFile);
    if (uiRes < (uiExpectedSize - uiRead))
    {
      if (feof(m_pFile))
      {
        if (pbGotEOF)
          *pbGotEOF = true;

        if (uiRes == 0 && uiRead == 0)
        {
          if (bIsFileHeader)
            qWarning("Empty GPSR file %s !", m_strFilePath.constData());

          if (!m_bDiscoveryRead)
            emit sigReadEOF(this);
        }
        else
        {
          m_bIncomplete = true;
          qWarning("Incomplete GPSR file %s !", m_strFilePath.constData());

          if (bIsFileHeader)
          {
            // we should never get here in non-discovery mode since the
            // purpose of the discovery mode is to have a first reading pass
            // to count available and complete chunks from the file.
            // thus, readChunk() will never try to read a chunk with an
            // index greater or equal than count of discovered chunks.
            Q_ASSERT(m_bDiscoveryRead == true);
            this->signalReadError(ERROR_TRUNCATED);
          }
        }

        return false;
      }
      else
      {
        this->signalReadError(ERROR_READ);
        return false;
      }
    }
    else
    {
      uiRead += uiRes;
    }
  }

  return true;
}

//---------------------------------------------------------------------------
// signalReadError
//---------------------------------------------------------------------------
void GPSRFile::signalReadError (Error eError)
{
  m_bError = true;
  emit sigReadError(this, eError);
}



//---------------------------------------------------------------------------
// chunkIdToLabel
//---------------------------------------------------------------------------
const char* GPSRFile::chunkIdToLabel (quint16 uiChunkId)
{
  static const char* c_pszUnknown = "?";

  for (int i = 0; i < CHUNKNAMES_COUNT; ++i)
  {
    if (CHUNKNAMES[i].id == uiChunkId)
      return CHUNKNAMES[i].label;
  }

  return c_pszUnknown;
}

//---------------------------------------------------------------------------
// fullMeansOfTransportToLabel
//---------------------------------------------------------------------------
QString GPSRFile::fullMeansOfTransportToLabel (quint8 ucMeansOfTransport, const QString& strOtherMeansOfTransport)
{
  QString strLabel(GPSRFile::meansOfTransportToLabel(ucMeansOfTransport));

  if (ucMeansOfTransport == GPSRFile::MEANSTRANSPORT_OTHER)
  {
    if (strOtherMeansOfTransport.isEmpty())
      strLabel += " : ?";
    else
      strLabel += QString(" : \"%1\"").arg(strOtherMeansOfTransport);
  }

  return strLabel;
}

//---------------------------------------------------------------------------
// meansOfTransportToLabel
//---------------------------------------------------------------------------
const char* GPSRFile::meansOfTransportToLabel (quint8 ucMeansOfTransport)
{
  static const char* c_pszUnknown = "?";

  for (int i = 0; i < TRANSPORTNAMES_COUNT; ++i)
  {
    if (TRANSPORTNAMES[i].id == ucMeansOfTransport)
      return TRANSPORTNAMES[i].label;
  }

  return c_pszUnknown;
}

//---------------------------------------------------------------------------
// meansOfTransportList
//---------------------------------------------------------------------------
QVector<quint8> GPSRFile::meansOfTransportList (void)
{
  QVector<quint8> vecMOT;

  for (int i = 0; i < TRANSPORTNAMES_COUNT; ++i)
    vecMOT.append(TRANSPORTNAMES[i].id);

  return vecMOT;
}

//---------------------------------------------------------------------------
// isValidMeansOfTransport
//---------------------------------------------------------------------------
bool GPSRFile::isValidMeansOfTransport (quint8 ucMeansOfTransport)
{
  return GPSRFile::meansOfTransportList().indexOf(ucMeansOfTransport) >= 0;
}

//---------------------------------------------------------------------------
// validateOtherMeansOfTransport
//---------------------------------------------------------------------------
QString GPSRFile::validateOtherMeansOfTransport (const QString& strOtherMeansOfTransport)
{
  QString str(strOtherMeansOfTransport);
  str.replace(QRegExp("[^A-Za-z0-9_ ]"), " ");
  str = str.simplified();
  return str;
}

//---------------------------------------------------------------------------
// cleanupIncomplete
//---------------------------------------------------------------------------
bool GPSRFile::cleanupIncomplete (const char* pszFile)
{
  GPSRFile gpsrFile;
  uint     uiValidSize = 0;

  if (!gpsrFile.openRead(pszFile))
    return false;

  if (gpsrFile.getReadChunksCount(GPSRFile::CHUNK_LOCATIONFIX) > 0)
  {
    const ChunkReadInfo& criLast = gpsrFile.getReadChunksInfo().last();
    uiValidSize = criLast.uiOffset + criLast.uiSize;
  }

  gpsrFile.close();

  if (uiValidSize > 0)
  {
    QFileInfo fi(pszFile);

    // here, file is incomplete and just need to be truncated so only valid
    // data remains in file.

    if ((qint64)uiValidSize < fi.size())
    {
      if (!QFile::resize(pszFile, (qint64)uiValidSize))
      {
        qWarning("Failed to truncate incomplete file %s !", pszFile);
        return false;
      }

      qDebug("Truncated incomplete file %s !", pszFile);
    }
  }
  else
  {
    // here, delete file because it is empty or does not contains any
    // location fix.

    if (!QFile::remove(pszFile))
    {
      qWarning("Failed to delete empty file %s !", pszFile);
      return false;
    }

    qDebug("Deleted empty file %s !", pszFile);
  }

  return true;
}

//---------------------------------------------------------------------------
// dump
//---------------------------------------------------------------------------
bool GPSRFile::dump (const char* pszFile, QString& strDump, bool bIncludeLocationFix)
{
  QVector<ChunkReadInfo>         vecChunks;
  QVectorIterator<ChunkReadInfo> itChunk(vecChunks);
  bool       bReturn = false;
  QFile*     pFile = 0;
  QByteArray swap;
  qint64     iRes;
  uint       uiTrackStartTime;
  uint       uiSkippedFix;

  strDump.clear();

  // validate file and truncate it if it is incomplete
  if (!GPSRFile::cleanupIncomplete(pszFile))
    return false;

  // read entire file to get list of chunks
  {
    GPSRFile gpsrFile;
    if (gpsrFile.openRead(pszFile))
      vecChunks = gpsrFile.getReadChunksInfo();
    if (vecChunks.isEmpty())
      return false;
  }

  // open file
  pFile = new QFile(pszFile);
  if (!pFile)
    return false;
  if (!pFile->open(QIODevice::ReadOnly))
    goto __end;

  // dump header
  {
    Header*   pHeader;
    QFileInfo fi(pszFile);

    swap.reserve(sizeof(*pHeader) + 1);
    pHeader = (Header*)swap.data();

    iRes = pFile->read((char*)pHeader, sizeof(*pHeader));
    if (iRes != sizeof(*pHeader))
      goto __end;

    uiTrackStartTime = pHeader->uiTime;

    strDump += QString(
      "Header :\n"
      "\tFile           %1\n"
      "\tFormat         %2\n"
      "\tTime           %3 UTC (%4)\n" )
      .arg(fi.fileName())
      .arg((int)pHeader->ucFormat)
      .arg(Util::timeString(true, pHeader->uiTime).constData())
      .arg(pHeader->uiTime);

    if (pHeader->ucFormat >= 1)
    {
      strDump +=
        QString("\tTimeZoneOffset %1 (%2)\n")
        .arg(Util::timeZoneOffsetHuman(pHeader->iTimeZoneOffset).constData())
        .arg(pHeader->iTimeZoneOffset);
    }

    strDump += "\n";
  }

  // tracks statistics
  {
    int iTracks = GPSRFile::chunksCount(vecChunks, CHUNK_NEWTRACK);
    if (!iTracks)
      ++iTracks;

    uint uiTotalDuration =
      (vecChunks.last().uiTime > uiTrackStartTime) ?
      vecChunks.last().uiTime - uiTrackStartTime :
      0;

    strDump += QString(
      "Tracks :\n"
      "\tTracksCount    %1\n"
      "\tTracksDuration %2 (%3 seconds)\n"
      "\n")
      .arg(iTracks)
      .arg(Util::timeDiffHuman(uiTotalDuration).constData())
      .arg(uiTotalDuration);
  }

  // chunks statistics
  strDump += QString(
    "Chunks statistics :\n"
    "\t%1 %2\n" )
    .arg("TotalChunks", -21)
    .arg(vecChunks.size());
  for (int i = 0; i < CHUNKNAMES_COUNT; ++i)
  {
    strDump +=
      QString("\t%1 %2\n")
      .arg(QString("Chunk") + CHUNKNAMES[i].label, -21)
      .arg(GPSRFile::chunksCount(vecChunks, CHUNKNAMES[i].id));
  }
  strDump += "\n";

  // dump chunks
  itChunk = vecChunks;
  uiSkippedFix = 0;
  while (itChunk.hasNext())
  {
    const ChunkReadInfo& cri = itChunk.next();
    Chunk* pChunk = 0;
    int iFirstColWidth = 8;

    // skip location fix if we don't want it
    if ((cri.uiId == CHUNK_LOCATIONFIX) ||
      (cri.uiId == CHUNK_LOCATIONFIX_LOST))
    {
      ++uiSkippedFix;
      if (!bIncludeLocationFix)
        continue;
    }
    else if (!bIncludeLocationFix && (uiSkippedFix > 0))
    {
      strDump +=
        QString("Skipped %1 LocationFix and LocationFixLost chunks.\n\n")
        .arg(uiSkippedFix);
      uiSkippedFix = 0;
    }

    // read chunk body
    if (pFile->seek((qint64)cri.uiOffset))
    {
      swap.reserve(cri.uiSize + 1);
      pChunk = (Chunk*)swap.data();

      iRes = pFile->read((char*)pChunk, cri.uiSize);
      if (iRes != (qint64)cri.uiSize)
        pChunk = 0;
    }

    // output chunk header
    strDump += QString(
      "CHUNK %1 (id %2) :\n"
      "\t%3 %4\n"
      "\t%5 %6\n"
      "\t%7 %8 UTC (%9)\n")
      .arg(GPSRFile::chunkIdToLabel(cri.uiId))
      .arg((uint)cri.uiId)
      .arg("Offset", -iFirstColWidth)
      .arg(cri.uiOffset)
      .arg("Size", -iFirstColWidth)
      .arg(cri.uiSize)
      .arg("Time", -iFirstColWidth)
      .arg(Util::timeString(true, cri.uiTime).constData())
      .arg(cri.uiTime);

    // output chunk body
    if (cri.uiId == CHUNK_MESSAGE ||
      cri.uiId == CHUNK_NAMEDSNAP ||
      cri.uiId == CHUNK_PAUSED)
    {
      strDump +=
        QString("\t%1 %2\n")
        .arg("Label", -iFirstColWidth)
        .arg((char*)&(pChunk->aData));
    }
    else if (cri.uiId == CHUNK_NEWTRACK)
    {
      long iOffset = qintptr((qintptr*)&pChunk->aData[0]);
      char* pszName = (char*)&pChunk->aData[sizeof(qint32)];

      strDump += QString(
        "\t%1 %2 (%3)\n"
        "\t%4 %5\n")
        .arg("TimeZone", -iFirstColWidth)
        .arg(Util::timeZoneOffsetHuman(iOffset).constData())
        .arg(iOffset)
        .arg("Label", -iFirstColWidth)
        .arg(pszName);
    }
    else if (cri.uiId == CHUNK_MEANSTRANSPORT)
    {
      strDump +=
        QString("\t%1 %2 (id %3)\n")
        .arg("Label", -iFirstColWidth)
        .arg(GPSRFile::fullMeansOfTransportToLabel(pChunk->aData[0], (char*)&(pChunk->aData[1])))
        .arg((int)pChunk->aData[0]);
    }

    strDump += "\n";
  }

  strDump += "EOF\n";
  bReturn = true;

__end :
  if (pFile)
    delete pFile;
  return bReturn;
}

//---------------------------------------------------------------------------
// chunksCount
//---------------------------------------------------------------------------
int GPSRFile::chunksCount (const QVector<ChunkReadInfo>& vecChunks, quint16 uiChunkId)
{
  int nCount = 0;

  for (int i = 0; i < vecChunks.count(); ++i)
  {
    if (vecChunks[i].uiId == uiChunkId)
      ++nCount;
  }

  return nCount;
}

//---------------------------------------------------------------------------
// chunksNext
//---------------------------------------------------------------------------
int GPSRFile::chunksNext (const QVector<ChunkReadInfo>& vecChunks, quint16 uiChunkId, int iPos/*=0*/)
{
  for (int i = iPos; i < vecChunks.count(); ++i)
  {
    if (vecChunks[i].uiId == uiChunkId)
      return i;
  }

  return -1;
}

//---------------------------------------------------------------------------
// chunksLast
//---------------------------------------------------------------------------
int GPSRFile::chunksLast (const QVector<ChunkReadInfo>& vecChunks, quint16 uiChunkId)
{
  int iLastKnownPos = -1;

  for (int i = 0; i < vecChunks.count(); ++i)
  {
    if (vecChunks[i].uiId == uiChunkId)
      iLastKnownPos = i;
  }

  return iLastKnownPos;
}

//---------------------------------------------------------------------------
// chunksLastBefore
//---------------------------------------------------------------------------
int GPSRFile::chunksLastBefore (const QVector<ChunkReadInfo>& vecChunks, quint16 uiChunkIdLast, quint16 uiChunkIdBefore, int iPos/*=0*/)
{
  int iLastKnownPos = -1;

  for (int i = iPos; i < vecChunks.count(); ++i)
  {
    if (vecChunks[i].uiId == uiChunkIdBefore)
      break;
    else if (vecChunks[i].uiId == uiChunkIdLast)
      iLastKnownPos = i;
  }

  return iLastKnownPos;
}
