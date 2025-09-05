// RecordFile.h
//
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

#include "nomfc_stdafx.h"

#include <string>
#include <fstream>

#ifndef RECORD_FILE_H
#define RECORD_FILE_H

#ifdef MR_UTIL
   #define MR_DllDeclare //   __declspec( dllexport )
#else
   #define MR_DllDeclare //   __declspec( dllimport )
#endif

namespace NoMFC 
{
   class CFile
   {
      public:
      CFile();
      ~CFile();
      
      BOOL OpenForRead(std::string fileName);
      LONG Seek(LONG offset, std::ios::seekdir direction);

      size_t Read(void* buffer, size_t maxBytes);
      
      template <typename T>
      CFile& Read(T& value) {
         stream.read(reinterpret_cast<char*>(&value), sizeof(T));
         return *this;
      }

      // Overload >> for std::string (handling length-prefixed strings)
      CFile& Read(std::string& value) {
         unsigned char length;
         this->Read(length);
         value.resize(length);
         stream.read(&value[0], length);
         return *this;
     }

      void Close();

      private:
      std::ifstream stream;
   };

   class CArchive
   {
      private:
         CFile* mCFile;

      public:
      enum Mode { store = 0, load = 1, };

      CArchive(CFile* pCFile, Mode mode);
      
      BOOL IsStoring();
      UINT Read(void* lpBuf, UINT nMax);
      void Write(const void* lpBuf, INT nMax);

      // Overload >> for primitive types (e.g., int, float, etc.)
      template <typename T>
      CArchive& operator>>(T& value) {
         mCFile->Read(value);
         return *this;
      }

      // Overload >> for std::string (handling length-prefixed strings)
      CArchive& operator>>(std::string& value) {
         mCFile->Read(value);
         return *this;
      }

      template <typename T>
      CArchive& operator<<(T& value) {
         ASSERT( FALSE );
         throw std::runtime_error("Not implemented");
      }
   };
}

class MR_RecordFileTable;

class MR_RecordFile
{
   private:
      NoMFC::CFile*       mFile;
      MR_RecordFileTable* mTable;
      int                 mCurrentRecord; // for read and write, -1 = not specified
      BOOL                mConstructionMode;

   public:
      // Constructors
	   MR_DllDeclare MR_RecordFile();
	   MR_DllDeclare ~MR_RecordFile();

      // Read operation
	   MR_DllDeclare BOOL OpenForRead( const char* pFileName, BOOL pValidateChkSum = FALSE );
      MR_DllDeclare void SelectRecord( int pRecordNumber );

	   uint32_t GetCheckSum();

      // File information functions
      MR_DllDeclare int  GetNbRecords()const;
      MR_DllDeclare int  GetNbRecordsMax()const;
      MR_DllDeclare int  GetCurrentRecordNumber()const;
      
	   MR_DllDeclare LONG Seek(LONG offset, std::ios::seekdir direction);

	   MR_DllDeclare void Close();

      NoMFC::CFile*  File();
};

#undef MR_DllDeclare

#endif
