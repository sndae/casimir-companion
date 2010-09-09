#ifndef LOWLEVEL_H
#define LOWLEVEL_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <vector>

#define _WIN32_WINNT 0x0501


#include <windows.h>
#include <stdio.h>
#include <winbase.h>
#include <Ntsecapi.h>
#include <winioctl.h>

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

/*typedef struct _DISK_EXTENT {  DWORD DiskNumber;  LARGE_INTEGER StartingOffset;  LARGE_INTEGER ExtentLength;
} DISK_EXTENT, *PDISK_EXTENT;


typedef struct _VOLUME_DISK_EXTENTS {  DWORD NumberOfDiskExtents;  DISK_EXTENT Extents[1];
} VOLUME_DISK_EXTENTS, *PVOLUME_DISK_EXTENTS;

typedef struct _PARTITION_INFORMATION {  LARGE_INTEGER StartingOffset;  LARGE_INTEGER PartitionLength;  DWORD HiddenSectors;  DWORD PartitionNumber;  BYTE PartitionType;  BOOLEAN BootIndicator;  BOOLEAN RecognizedPartition;  BOOLEAN RewritePartition;
} PARTITION_INFORMATION, *PPARTITION_INFORMATION;


typedef struct _DISK_GEOMETRY_EX {  DISK_GEOMETRY Geometry;  LARGE_INTEGER DiskSize;  BYTE Data[1];
} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;

*/
extern "C"
{
   /*WINBASEAPI HANDLE WINAPI FindFirstVolumeMountPointA(
       LPCSTR lpszRootPathName,
       LPSTR lpszVolumeMountPoint,
       DWORD cchBufferLength
       );
   WINBASEAPI HANDLE WINAPI FindFirstVolumeMountPointW(
       LPCWSTR lpszRootPathName,
       LPWSTR lpszVolumeMountPoint,
       DWORD cchBufferLength
       );
   WINBASEAPI HANDLE WINAPI FindFirstVolumeA(
       LPSTR lpszVolumeName,
       DWORD cchBufferLength
       );
   WINBASEAPI HANDLE WINAPI FindFirstVolumeW(
       LPWSTR lpszVolumeName,
       DWORD cchBufferLength
       );
   WINBASEAPI BOOL WINAPI FindNextVolumeA(
       HANDLE hFindVolume,
       LPSTR lpszVolumeName,
       DWORD cchBufferLength
       );
   WINBASEAPI BOOL WINAPI FindNextVolumeW(
       HANDLE hFindVolume,
       LPWSTR lpszVolumeName,
       DWORD cchBufferLength
       );
   WINBASEAPI BOOL WINAPI FindVolumeClose(
       HANDLE hFindVolume
       );
   WINBASEAPI BOOL WINAPI FindVolumeMountPointClose(
       HANDLE hFindVolumeMountPoint
       );

   WINBASEAPI BOOL WINAPI GetVolumeNameForVolumeMountPointA(
       LPCSTR lpszVolumeMountPoint,
       LPSTR lpszVolumeName,
       DWORD cchBufferLength
       );
   WINBASEAPI BOOL WINAPI GetVolumeNameForVolumeMountPointW(
       LPCWSTR lpszVolumeMountPoint,
       LPWSTR lpszVolumeName,
       DWORD cchBufferLength
		 );*/

	/*NTSTATUS NtCreateFile(
       PHANDLE FileHandle,
       ACCESS_MASK DesiredAccess,
       POBJECT_ATTRIBUTES ObjectAttributes,
       PIO_STATUS_BLOCK IoStatusBlock,
       PLARGE_INTEGER AllocationSize OPTIONAL,
       ULONG FileAttributes,
       ULONG ShareAccess,
       ULONG CreateDisposition,
       ULONG CreateOptions,
       PVOID EaBuffer OPTIONAL,
       ULONG EaLength
       );


   extern NTSTATUS WINAPI (*NtOpenFile)(
       PHANDLE FileHandle,
       ACCESS_MASK DesiredAccess,
       POBJECT_ATTRIBUTES ObjectAttributes,
       PIO_STATUS_BLOCK IoStatusBlock,
       ULONG ShareAccess,
       ULONG OpenOptions);
*/
/*
   extern NTSTATUS WINAPI (*NtQueryDirectoryObject)(
     HANDLE DirectoryHandle,
     PVOID Buffer,
     ULONG Length,
     BOOLEAN ReturnSingleEntry,
     BOOLEAN RestartScan,
     PULONG Context,
     PULONG ReturnLength);

   extern NTSTATUS WINAPI (*NtOpenSymbolicLinkObject)(
         PHANDLE SymLinkObjHandle,
         ACCESS_MASK DesiredAccess,
         POBJECT_ATTRIBUTES ObjectAttributes);

   extern NTSTATUS WINAPI (*NtQuerySymbolicLinkObject)(
         HANDLE SymLinkObjHandle,
         PUNICODE_STRING LinkName,
         PULONG DataWritten);

   extern void (*RtlInitUnicodeString)(PUNICODE_STRING,PCWSTR);
   extern NTSTATUS WINAPI (*NtOpenDirectoryObject)(
      PHANDLE DirObjHandle,
      ACCESS_MASK DesiredAccess,
      POBJECT_ATTRIBUTES ObjectAttributes);*/
}

#ifdef UNICODE
#define FindFirstVolumeMountPoint FindFirstVolumeMountPointW
#else
#define FindFirstVolumeMountPoint FindFirstVolumeMountPointA
#endif // !UNICODE
#ifdef UNICODE
#define FindFirstVolume FindFirstVolumeW
#else
#define FindFirstVolume FindFirstVolumeA
#endif // !UNICODE

#ifdef UNICODE
#define FindNextVolume FindNextVolumeW
#else
#define FindNextVolume FindNextVolumeA
#endif // !UNICODE

#ifdef UNICODE
#define GetVolumeNameForVolumeMountPoint  GetVolumeNameForVolumeMountPointW
#else
#define GetVolumeNameForVolumeMountPoint  GetVolumeNameForVolumeMountPointA
#endif // !UNICODE


#define OBJ_INHERIT              0x00000002
#define OBJ_PERMANENT            0x00000010
#define OBJ_EXCLUSIVE            0x00000020
#define OBJ_CASE_INSENSITIVE     0x00000040
#define OBJ_OPENIF               0x00000080
#define OBJ_OPENLINK             0x00000100
#define OBJ_VALID_ATTRIBUTES     0x000001F2
#define DIRECTORY_QUERY          0x0001
#define SYMBOLIC_LINK_QUERY      0x0001


//#define FILE_DEVICE_DISK         0x00000007

//#define IOCTL_VOLUME_BASE 86
//#define IOCTL_DISK_BASE                 FILE_DEVICE_DISK

/*#define METHOD_BUFFERED   0
#define METHOD_IN_DIRECT  1
#define METHOD_OUT_DIRECT 2
#define METHOD_NEITHER    3

#define FILE_ANY_ACCESS   0x00
#define FILE_READ_ACCESS  0x01    // file & pipe
#define FILE_WRITE_ACCESS 0x02    // file & pipe
*/


#define Media_Type_Unknown        0        // Format is unknown
#define Media_Type_F5_1Pt2_512    1        // 5.25", 1.2MB,  512 bytes/sector
#define Media_Type_F3_1Pt44_512   2        // 3.5",  1.44MB, 512 bytes/sector
#define Media_Type_F3_2Pt88_512   3        // 3.5",  2.88MB, 512 bytes/sector
#define Media_Type_F3_20Pt8_512   4        // 3.5",  20.8MB, 512 bytes/sector
#define Media_Type_F3_720_512     5        // 3.5",  720KB,  512 bytes/sector
#define Media_Type_F5_360_512     6        // 5.25", 360KB,  512 bytes/sector
#define Media_Type_F5_320_512     7        // 5.25", 320KB,  512 bytes/sector
#define Media_Type_F5_320_1024    8        // 5.25", 320KB,  1024 bytes/sector
#define Media_Type_F5_180_512     9        // 5.25", 180KB,  512 bytes/sector
#define Media_Type_F5_160_512     10       // 5.25", 160KB,  512 bytes/sector
#define Media_Type_RemovableMedia 11       // Removable media other than floppy
#define Media_Type_FixedMedia     12       // Fixed hard disk media


#define DiskGeometryGetPartition(Geometry)\
                        ((PDISK_PARTITION_INFO)((Geometry)->Data))

#define DiskGeometryGetDetect(Geometry)\
                        ((PDISK_DETECTION_INFO)(((DWORD_PTR)DiskGeometryGetPartition(Geometry)+\
                                        DiskGeometryGetPartition(Geometry)->SizeOfPartitionInfo)))

/* volume: \\?\Volume{GUID}\
   */
typedef struct
{
   QByteArray volumeName;        /* \\?\Volume{GUID}\ */
   QByteArray fileSystem;        /* String indicating the file system */
   QByteArray mountPoint;        /* a:\ */
   QByteArray volumeLink;        /* \Device\HarddiskVolume6 */
   QByteArray Name;              /* User name of the device. From GetVolumeInformation */
   DWORD dwSysFlags;             /* From GetVolumeInformation */
   DWORD serialNumber;           /* From GetVolumeInformation */

   int driveType;                /* From GetDriveType */

   // Return from GetDiskGeometryEx
   int MediaType;
   int TracksPerCylinder,SectorsPerTrack,BytesPerSector;
   long long Cylinders,DiskSize;

   // Alternative way to estimate disk size
   long long DiskSize2;

} VOLUMEDEF;


bool NativeInit();
std::vector<VOLUMEDEF> GetListOfMountPoints(QStringList List);
bool NativeDir(QString Dir,QStringList &List);
void ParseNativeDir(QStringList List);
//std::vector<VOLUMEDEF> GetListOfVolumes();
std::vector<VOLUMEDEF> GetListOfVolumes(std::vector<int> AllowedDriveType,std::vector<std::string> AllowedLinkName);
bool GetGeometry(QByteArray DeviceName,QByteArray Description);
bool GetGeometryEx(QByteArray DeviceName,int &MediaType,long long int &Cylinders,int &TracksPerCylinder,int &SectorsPerTrack,
                   int &BytesPerSector,long long &DiskSize);
bool GetSizeHandle(HANDLE h,long long &len,int &method);
bool GetSize(QByteArray DeviceName,long long &size,int &method);
bool GetDiskExtents(HANDLE hFile,QByteArray &Device,uint64_t &Offset, uint64_t &Len);
bool GetPartitionSize(HANDLE h,long long &len);

HANDLE NTCreateFile(QByteArray FileName,DWORD dwDesiredAccess,DWORD dwShareMode,PSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);

class LowLevelFile
{
	HANDLE h;
	QByteArray fileName;
public:
	LowLevelFile(QByteArray _fn);
	~LowLevelFile();
	bool open();
	bool close();
	bool read(int size,unsigned char *buffer,int &read);
	bool seek(unsigned long long pos,unsigned long long &newpos);
};


#endif // LOWLEVEL_H


