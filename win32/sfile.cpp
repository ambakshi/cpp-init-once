#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#define _WIN32_WINNT 0x0600
#include <Windows.h>
#include <malloc.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>


typedef struct _FILE_RENAME_INFORMATION {
  BOOLEAN ReplaceIfExists;
  HANDLE  RootDirectory;
  ULONG   FileNameLength;
  WCHAR   FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

typedef enum _FILE_INFORMATION_CLASS { 
  FileDirectoryInformation  = 1,
  FileFullDirectoryInformation = 2,
  FileBothDirectoryInformation = 4,
  FileBasicInformation = 4,
  FileStandardInformation = 5,
  FileInternalInformation = 6,
  FileEaInformation = 7,
  FileAccessInformation = 8,
  FileNameInformation = 9,
  FileRenameInformation = 10,
  FileLinkInformation = 11,
  FileNamesInformation = 12
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef struct _IO_STATUS_BLOCK {
  union {
    LSTATUS Status;
    PVOID    Pointer;
  };
  ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


typedef LSTATUS (NTAPI*pfnSetInfoFile)(HANDLE hFile, IO_STATUS_BLOCK* iosb, PVOID pData, ULONG dataLen, FILE_INFORMATION_CLASS infoClass);

pfnSetInfoFile SetInformationFile;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((LSTATUS)(Status)) >= 0)
#endif

static FILE* fopen_unixlike(const char *file, const char *mode, HANDLE* phandle) {
  DWORD dwDesiredAccess = 0;
  DWORD dwCreationDisposition = 0;
  //const DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    const DWORD dwFlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
  const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  size_t i;

  HANDLE handle;
  int fd;

  /* Infer flags. */
  for(i = 0; mode[i] != '\0'; i++) {
    switch(mode[i]) {
      case 'r':
        dwDesiredAccess |= GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
        break;
      case 'a':
        dwDesiredAccess |= GENERIC_WRITE;
        dwCreationDisposition = CREATE_NEW;
        break;
      case 'w':
        dwDesiredAccess |= GENERIC_WRITE;
        dwCreationDisposition = CREATE_ALWAYS;
        break;
      case '+':
        dwDesiredAccess |= GENERIC_READ;
        dwDesiredAccess |= GENERIC_WRITE;
        break;
    }
  }

  /* Create the file. */
  handle = CreateFileA(file,
                       dwDesiredAccess,
                       dwShareMode,
                       NULL,
                       dwCreationDisposition,
                       dwFlagsAndAttributes,
                       NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    return NULL;
  }

  /* Associates a C run-time file descriptor with a file HANDLE. */
  fd = _open_osfhandle((intptr_t) handle, _O_BINARY);
  if (fd == -1) {
    CloseHandle(handle);
    return NULL;
  }

    *phandle = handle;
  /* Associates a stream with a C run-time file descriptor. */
  return _fdopen(fd, mode);
}

bool use_dir = false;
int WinLowLevelRename( HANDLE file, const char* fullpath)
{
    DWORD err = 0;
    IO_STATUS_BLOCK iob = {0};
    char fdrive[MAX_PATH], fdir[MAX_PATH], fname[MAX_PATH], fext[MAX_PATH];
    char dir[MAX_PATH], filename[MAX_PATH];



    _splitpath( fullpath, fdrive, fdir, fname, fext);
    _snprintf( dir, MAX_PATH, "%s%s", fdrive, fdir );
    //_snprintf( filename, MAX_PATH, "%s%s", fname, fext);
    _snprintf( filename, MAX_PATH, "%s%s%s%s", fdrive, fdir, fname, fext);

    size_t len = strlen(filename);
    size_t bufl= 2 * ( len + 1    );
    size_t bytes = sizeof(FILE_RENAME_INFORMATION) + bufl;
    
    HANDLE hDir = NULL;
    if (use_dir) {
        hDir = CreateFile(dir, GENERIC_READ | GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL, OPEN_EXISTING , FILE_FLAG_BACKUP_SEMANTICS, NULL );
        if ( hDir == INVALID_HANDLE_VALUE )
        {
            fprintf( stderr,"ERROR:%d:[%s] Failed to open directory\n", err, dir );
            goto error;
        }
    }

    DWORD written = 0;
    if (!WriteFile( file, filename, len, &written, NULL))
    {
        err = GetLastError();
        fprintf( stderr, "ERROR:0x%08x:[%s] Failed to write to handle\n", err, filename);
    }
    fprintf(stderr," Wrote %d/%d bytes \n", written, len);
    FILE_RENAME_INFORMATION* finfo;
    
    finfo = (FILE_RENAME_INFORMATION *)malloc( bytes );
    swprintf( finfo->FileName, bufl,  L"%hs", filename);
    fwprintf( stderr, L"%s\n", finfo->FileName);
    finfo->FileNameLength = (ULONG)( len * 2 ) ;
    finfo->ReplaceIfExists = TRUE;
    finfo->RootDirectory = hDir;

    LSTATUS sts = SetInformationFile( file, &iob, finfo, (ULONG)bytes, FileRenameInformation );
    if (!NT_SUCCESS(sts))
    {
        err = sts;
        fprintf( stderr, "ERROR:0x%08x:[%s] Failed to rename file\n", err, filename);
    }
    if (use_dir) {
        CloseHandle( hDir );
    }
    free( finfo );
error:
    return err;
}

int main()
{
    HMODULE hNtDll = GetModuleHandle("ntdll.dll");
    SetInformationFile = (pfnSetInfoFile)GetProcAddress(hNtDll, "ZwSetInformationFile");
    if (!SetInformationFile)
    {
        DWORD err = GetLastError();
        fprintf( stderr,"ERROR:%d:[%s] Failed to get function point\n", err, "SetInformationFile" );
        return 1;
    }

    int err = 0;
    FILE *fp = fopen("C:\\test.txt","wt");
    fprintf( fp, "%s\n", "hello world" );
    fprintf( fp, "%s\n", "phase1" );
    fclose ( fp );

    HANDLE handle = INVALID_HANDLE_VALUE;
    fp = fopen_unixlike("C:\\test.tmp","wt", &handle);
    fprintf( fp, "%s\n", "hello world" );
    fprintf( fp, "%s\n", "phase2" );
    fprintf( fp, "%s\n", "hello world" );
    fprintf( fp, "%s\n", "hello world" );
    fflush( fp );
    
    //_iobuf *io = (_iobuf *)fp;
    fprintf( stderr," Handle is 0x%08x\n", handle );
    err = WinLowLevelRename( handle, "test.txt" );
    if ( err )
    {
        char msgBuf[1024];
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM 
                        | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        err,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        msgBuf,
                        1024,
                        NULL);
        fprintf( stderr, "ERROR:0x%08x:[%s] Failed to WinLowLevelRename. %s.\n", err, "C:\\test.txt", msgBuf );
    }
    if ( fclose( fp ) )
    {
        err = errno;
        fprintf( stderr, "ERROR:%d:[%s] Failed to fclose\n", err, "C:\\test.tmp" );
        
    }
    return err;
}
