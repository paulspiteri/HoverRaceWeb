// RecordFile.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which 
// you have taken this file. If you can not find the license you can not use 
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions 
// and limitations under the License.
//


#include "RecordFile.h"
#include <stdio.h>
#include <string>
#include <stdexcept>

using namespace NoMFC;

// MR_RecordFileTableStuff

class MR_RecordFileTable
{
   public:
      std::string mFileTitle;
      BOOL    mSumValid;
      DWORD   mChkSum;      // Check sum of the control record
      int     mRecordUsed;  // Nb of record used
      int     mRecordMax;   // Ne of record allowed
      DWORD*  mRecordList;

      MR_RecordFileTable();
      MR_RecordFileTable( int pNbRecords );
      ~MR_RecordFileTable();

      void Serialize( CArchive& pArchive );

};


static DWORD ComputeSum( const char* pFileName );

MR_RecordFileTable::MR_RecordFileTable()
{
   mRecordUsed     = 0;
   mRecordMax      = 0;
   mSumValid       = FALSE;
   mChkSum         = 0;

   mRecordList     = NULL;
}

MR_RecordFileTable::MR_RecordFileTable( int pNbRecords )
{
   ASSERT( pNbRecords > 0 );

   mRecordMax      = pNbRecords;
   mRecordUsed     = 0;
   mRecordList     = new DWORD[ pNbRecords ];

   // Initialize the vector
   for( int lCounter = 0; lCounter < pNbRecords; lCounter++ )
   {
      mRecordList[lCounter] = NULL;
   }
}

MR_RecordFileTable::~MR_RecordFileTable()
{
   delete []mRecordList;
}

void MR_RecordFileTable::Serialize( CArchive& pArchive )
{

   if( pArchive.IsStoring() )
   {       
      ASSERT( FALSE );
      throw std::runtime_error("Not implemented");
      // pArchive << mFileTitle
      //          << (int) 0   // Padding for checksum purpose
      //          << (int) 0       
      //          << mSumValid
      //          << mChkSum               
      //          << mRecordUsed
      //          << mRecordMax
      //          << (int) 0
      //          << (int) 0;


      // if( mRecordMax > 0 )
      // {
      //    ASSERT( mRecordList != NULL );
      
      //    pArchive.Write( mRecordList, sizeof( mRecordList[0] ) *mRecordMax );
      // }
   }
   else
   {
      delete []mRecordList;
      mRecordList = NULL;

      int lDummy;

      pArchive >> mFileTitle
               >>lDummy
               >>lDummy
               >> mSumValid
               >> mChkSum
               >> mRecordUsed
               >> mRecordMax
               >>lDummy
               >>lDummy;

      if( mRecordMax > 0 )
      {
         mRecordList  = new DWORD[ mRecordMax ];
          
         pArchive.Read( mRecordList, sizeof( mRecordList[0] ) *mRecordMax );
      }
   }
}

// MR_RecordFile
MR_RecordFile::MR_RecordFile()
{
   mConstructionMode = FALSE;
   mTable = NULL;
   mCurrentRecord = -1;
   mFile = NULL;
}

MR_RecordFile::~MR_RecordFile()
{
   if( mTable != NULL )
   {
      Close();
      delete mTable;
   }
}

 
int MR_RecordFile::GetNbRecords()const
{
   int lReturnValue = 0;

   if( mTable != NULL )
   {
      lReturnValue = mTable->mRecordUsed;
   }
   return lReturnValue;
}

int MR_RecordFile::GetNbRecordsMax()const
{
   int lReturnValue = 0;

   if( mTable != NULL )
   {
      lReturnValue = mTable->mRecordMax;
   }
   return lReturnValue;
}
 
int MR_RecordFile::GetCurrentRecordNumber()const
{
   return mCurrentRecord;
}

BOOL MR_RecordFile::OpenForRead( const char* pFileName, BOOL pValidateChkSum )
{
   BOOL lReturnValue = FALSE;
   ASSERT( mTable == NULL);

   DWORD lSum = 0;

   if( pValidateChkSum )
   {
      lSum = ComputeSum( pFileName );
   }

   if( mTable == NULL )
   {
      mConstructionMode = FALSE;
      mCurrentRecord    = -1;

      // Try to open the file
      mFile = new CFile();
      lReturnValue = mFile->OpenForRead(pFileName);

      if( lReturnValue )
      {
         {
            CArchive lArchive(mFile, CArchive::Mode::load);

            mTable = new MR_RecordFileTable;

            mTable->Serialize( lArchive );
         }

         if( mTable == NULL )
         {
            lReturnValue = FALSE;
            mFile->Close();
         }
         else if( pValidateChkSum && (lSum != mTable->mChkSum ))
         {
            // Wrong file sum
            lReturnValue = FALSE;
            mFile->Close();
         }
         else
         {
            // Select the first record if availlable
            if( mTable->mRecordList != NULL )
            {
                mCurrentRecord = 0;
                Seek( 0, std::ios::beg );
            }
         }
      }
   }

   return lReturnValue;
}


uint32_t MR_RecordFile::GetCheckSum()
{
   if( (mTable != NULL)&&(mTable->mSumValid) )
   {
      return mTable->mChkSum;
   }
   else
   {
      return 0;
   }
}

void MR_RecordFile::SelectRecord( int pRecordNumber )
{
   ASSERT( !mConstructionMode );

   if( (mTable != NULL)&&(!mConstructionMode) )
   {
      if( pRecordNumber < mTable->mRecordUsed )
      {
         mCurrentRecord = pRecordNumber;
         Seek( 0, std::ios::beg  );
      }
      else
      {
         ASSERT( FALSE );
      }
   }
}

LONG MR_RecordFile::Seek(LONG offset, std::ios::seekdir direction)
{
   // BUG This function do not check for record overflow
   LONG lLocalOffset = 0;

   if( mCurrentRecord >= 0 )
   {
      lLocalOffset = mTable->mRecordList[ mCurrentRecord ];
   }

   return mFile->Seek( offset+lLocalOffset, direction )-lLocalOffset;
}

void MR_RecordFile::Close()
{
   mCurrentRecord = -1;  
   if( mConstructionMode && (mTable != NULL ) )
   {
      // Write the Record table
      ASSERT( FALSE );
      throw std::runtime_error("Not implemented");
      // Seek( 0,  std::ios::beg );
      // CArchive lArchive( mFile, CArchive::store );
      // mTable->Serialize( lArchive );
   }
   mCurrentRecord = -1;
   delete mTable;
   mTable = NULL;

   mFile->Close();
}

NoMFC::CFile*  MR_RecordFile::File() 
{
   return mFile;
}

DWORD ComputeSum( const char* pFileName )
{
   DWORD lReturnValue = 0;
   DWORD lBuffer[2048]; // 8 K of data
   

   FILE* lFile = fopen( pFileName, "rb" );

   if( lFile != NULL )
   {
      int lDataLen = fread( lBuffer, 1, sizeof( lBuffer ), lFile );

      if( lDataLen > 0 )
      {
         lDataLen /= sizeof( DWORD );

         BOOL lSkippedDo = FALSE;

         for( int lCounter = 0; lCounter < lDataLen; lCounter++ )
         {
            if( !lSkippedDo && (lBuffer[lCounter ] == 0) )
            {
               lSkippedDo = TRUE;
               lCounter += 6;
            }
            else
            {
               lReturnValue += lBuffer[ lCounter ] + ~(lBuffer[lCounter]>>12);
               lReturnValue = (lReturnValue<<1)+(lReturnValue>>31);
            }
         }
      }
      fclose( lFile );
   }
   return lReturnValue;
}


CFile::CFile() 
{

}

CFile::~CFile() {
   Close();
}

BOOL CFile::OpenForRead(std::string fileName) 
{
   std::ios::openmode openMode = std::ios::binary | std::ios::in;
   stream.open(fileName, openMode);
   return stream.is_open();
}

LONG CFile::Seek(LONG offset, std::ios::seekdir direction) 
{
   if (!stream.is_open()) {
      return -1;
  }

  stream.seekg(offset, direction);

  if (stream.fail()) {
      return -1;
  }

  return static_cast<LONG>(stream.tellg());  // Return new position
}

size_t CFile::Read(void* buffer, size_t maxBytes)
{
    if (!stream.is_open() || buffer == nullptr) {
        throw std::runtime_error("Invalid read attempt");
    }

    stream.read(reinterpret_cast<char*>(buffer), maxBytes);
    return static_cast<size_t>(stream.gcount()); // actual bytes read
}

void CFile::Close()
{
   if (stream.is_open()) {
      stream.close();
  }
}

CArchive::CArchive(CFile* pCFile, Mode mode)
{
   mCFile = pCFile;
   if (mode != Mode::load) 
   {
      ASSERT( FALSE );
      throw std::runtime_error("Not implemented");
   }
}

BOOL CArchive::IsStoring() 
{
   return FALSE;
}

UINT CArchive::Read(void* lpBuf, UINT nMax)
{
   size_t nRead = mCFile->Read(lpBuf, nMax);
   return nRead;
}

void CArchive::Write(const void* lpBuf, INT nMax)
{
   ASSERT( FALSE );
   throw std::runtime_error("Not implemented");
}