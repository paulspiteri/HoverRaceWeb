#define MR_DllDeclare //  __declspec( dllexport )

// Functions declaration

extern "C"
{
   MR_DllDeclare void                  MR_InitModule( HMODULE pModule );
   MR_DllDeclare void                  MR_CleanModule();
   MR_DllDeclare MR_UInt16             MR_GetObjectTypeCount  ();
   MR_DllDeclare MR_UInt16             MR_GetObjectTypeCount  ();
   MR_DllDeclare CString               MR_GetObjectFamily     ( MR_UInt16 pClassId );
   MR_DllDeclare CString               MR_GetObjectDescription( MR_UInt16 pClassId );
   MR_DllDeclare MR_ObjectFromFactory* MR_GetObject           ( MR_UInt16 pClassId );
};
