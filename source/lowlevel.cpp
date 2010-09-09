#define _WIN32_WINNT 0x0501


#include <windows.h>
#include <winbase.h>
#include <winioctl.h>
#include <tchar.h>
#include <stdio.h>
#include <Ntsecapi.h>

#include <vector>
#include <algorithm>

#include "lowlevel.h"

/*
  volume
  disk

  GetVolumeNameForVolumeMountPointA

   Glossary
   --------

      VolumeMountPoint           a:\, c:\mount\          (trailing backslash required)
      VolumeName                 \\?\Volume{GUID}\       GUID identifies the volume


      \Device\HarddiskVolume6

		\.\PhysicalDrive0"


   Tools
   -----
      mountvol                   lists mounted volumes


*/

extern "C"
{
   NTSTATUS WINAPI (*NtQueryDirectoryObject)(
     HANDLE DirectoryHandle,
     PVOID Buffer,
     ULONG Length,
     BOOLEAN ReturnSingleEntry,
     BOOLEAN RestartScan,
     PULONG Context,
     PULONG ReturnLength)=0;

   NTSTATUS WINAPI (*NtOpenSymbolicLinkObject)(
         PHANDLE SymLinkObjHandle,
         ACCESS_MASK DesiredAccess,
         POBJECT_ATTRIBUTES ObjectAttributes)=0;

   NTSTATUS WINAPI (*NtQuerySymbolicLinkObject)(
         HANDLE SymLinkObjHandle,
         PUNICODE_STRING LinkName,
         PULONG DataWritten)=0;

   void (*RtlInitUnicodeString)(PUNICODE_STRING,PCWSTR)=0;

   NTSTATUS WINAPI (*NtOpenDirectoryObject)(
      PHANDLE DirObjHandle,
      ACCESS_MASK DesiredAccess,
      POBJECT_ATTRIBUTES ObjectAttributes)=0;

   NTSTATUS WINAPI (*NtOpenFile)(
       PHANDLE FileHandle,
       ACCESS_MASK DesiredAccess,
       POBJECT_ATTRIBUTES ObjectAttributes,
       PIO_STATUS_BLOCK IoStatusBlock,
       ULONG ShareAccess,
       ULONG OpenOptions)=0;
}
HMODULE module_handle=0;

QString NativeReadLink(QString Link );

void getlasterror(DWORD dw)
{
   QString strerr;
   strerr.reserve(1000);
   LPWSTR lpszFunction = (LPWSTR)strerr.data();
    TCHAR szBuf[80];
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf,
        L"%s failed with error %d: %s",
        lpszFunction, dw, lpMsgBuf);

    MessageBox(NULL, szBuf, L"Error", MB_OK);
    //printf("%s\n",strerr.toStdString().c_str());

    LocalFree(lpMsgBuf);
}

/**
   Computes the length of an unicode string by looking for the terminating 16-bit null, rather than relying on QString size function.
   Use this when calling native OS unicode functions that return a null-terminated string to set the size of the QString

   maxsize: maximum size of string in TChars (16-bit)
**/
bool FixUnicodeLength(QString &str,unsigned maxsize)
{
   short *ptr;
   ptr = (short*)str.data();
   for(unsigned i=0;i<maxsize;i++)
   {
      if(*(ptr+i)==0)
      {
         str.resize(i);
         return true;
      }
   }
   return false;
}
unsigned GetUnicodeLength(PWSTR str)
{
   short *ptr;
   unsigned l=0;
   ptr = (short*)str;
   while(1)
   {
      if(*(ptr+l)==0)
      {
         return l;
      }
      l++;
   }
   return 0;
}
bool FixAnsiLength(QByteArray &str,unsigned maxsize)
{
   char *ptr;
   ptr = str.data();
   for(unsigned i=0;i<maxsize;i++)
   {
      if(*(ptr+i)==0)
      {
         str.resize(i);
         return true;
      }
   }
   return false;
}

/**
   Input
      Volume   name of volume    \\?\Volume{GUID}\
   Returns
      List of volume mount points
**/
QStringList FindVolumeMountPoints(QByteArray Volume)
{
   QStringList r;

   //QString Buffer;
   QString VolumeW = QString(Volume);
   QByteArray Buffer;
   QString BufferW;
   HANDLE h;
   unsigned size=1024;

   //printf("Volume: %s\n",Volume.toStdString().c_str());
   printf("Volume: %s\n",Volume.data());

   Buffer.reserve(size);
   BufferW.reserve(size);
   //h = FindFirstVolumeMountPointW((LPCWSTR)VolumeW.data(), (LPWSTR)Buffer.data(),size);
   //h = FindFirstVolumeMountPointA((LPCSTR)Volume.data(), (LPSTR)Buffer.data(),size);
   h = FindFirstVolumeMountPointA("\\\\?\\c:\\", (LPSTR)Buffer.data(),size);
   //h = FindFirstVolumeMountPointA("\\\\?\\Volume{ee1b67d3-52e7-11df-8700-08002700a0fa}\\", (LPSTR)Buffer.data(),size);
   printf("enumerate mount points: %d\n",h);
   if(h==INVALID_HANDLE_VALUE)
   {
      DWORD dw = GetLastError();
      printf("Last error: %d\n",dw);
      //getlasterror(dw);
   }
   while(h!=INVALID_HANDLE_VALUE)
   {
      printf("Not invalid\n");
      //FixUnicodeLength(Buffer,size);
      FixAnsiLength(Buffer,size);

      //printf("Mount point: %s\n",Buffer.toStdString().c_str());
      printf("Mount point: %s\n",Buffer.data());

      //SetLength(Buffer, strlen(PChar(Buffer)));
      //Log('Mount point = ' + Buffer);FindFirstVolumeMountPointA

      /*Buffer := Volume + Buffer;
      List.Add(Buffer);

      SetLength(Buffer, 1024);
      if not JFindNextVolumeMountPoint(vh, PChar(Buffer), Length(Buffer)) then
      begin
         FindVolumeMountPointClose(vh);
         vh := INVALID_HANDLE_VALUE;
      end;*/
      FindVolumeMountPointClose(h);
      h = INVALID_HANDLE_VALUE;
   }
   return r;
}



/**
   Returns a list of volumes using the Win32 API including:
   - the volume name
   - the link
   - the mount point

	AllowedDriveType:		When empty, returns all drives. Otherwise, only returns drives of type (see GetDriveType for types)
	AllowedLinkName:		When empty, returns all drives. Otherwise, only returns drives whose link name contains one of the AllowedLinkNames)
								AllowedLinkNames can include e.g. Floppy, Harddisk
**/
std::vector<VOLUMEDEF> GetListOfVolumes(std::vector<int> AllowedDriveType,std::vector<std::string> AllowedLinkName)
{
   //QString Buffer;
   QByteArray Buffer;
   HANDLE h;
   DWORD   dwSysFlags;         // flags that describe the file system
   unsigned size=1024;
   char    FileSysNameBuffer[size];
   char userName[size];
   DWORD serialNumber;
   DWORD componentLength;

   QStringList DriveList;           /* List of volume names (\\?\Volume{GUID}\) that correspong to the mount points a:\...z:\ */

   std::vector<VOLUMEDEF> vvd;
   VOLUMEDEF vd;

   printf("Get list of volumes\n");

   // Enumerate the NT4 mount points
   // ------------------------------
   // Currently, only enumerates drive letters - mount points could also be directories
   for(char drive = 'a'; drive <='z'; drive++)
   {
      QByteArray DriveString=QByteArray(1,drive)+":\\";
      Buffer.reserve(size);
      if(GetVolumeNameForVolumeMountPointA(DriveString.data(),Buffer.data(),size))
      {
         FixAnsiLength(Buffer,size);
         //printf("Drive '%s' -> '%s'\n",DriveString.data(),Buffer.data());
         if(Buffer.size()>0)
            DriveList<<Buffer;
         else
            DriveList<<"";
      }
      else
         DriveList<<"";
   }

   for(unsigned i=0;i<DriveList.size();i++)
   {
      printf("Drive %c - %s\n",i+'a',DriveList[i].toStdString().c_str());
   }
   //return vvd;



   // Enumerate the volumes
   // ---------------------
   Buffer.reserve(size);
   h = FindFirstVolumeA((LPSTR)Buffer.data(),size);               // Returns the volume in Buffer (\\?\Volume{GUID}\)
   while(h!=INVALID_HANDLE_VALUE)
   {
      FixAnsiLength(Buffer,size);
      printf("--------------------------------------------------------------------------------\n");
      printf("Found Volume: %s\n",Buffer.data());

      //QStringList vmp = FindVolumeMountPoints(Buffer);    // seems not to work

      vd.volumeName=Buffer;

      int rv;
      rv = GetDriveTypeA(Buffer.data());
      vd.driveType=rv;
		//printf("Drive type: %d\n",rv);

		// Check if the drive type belongs to the drive types of interest
		if(AllowedDriveType.size()==0 || (find(AllowedDriveType.begin(),AllowedDriveType.end(),vd.driveType)!=AllowedDriveType.end()))
		{
			// Get some more infos
			//SetLength(VolumeName, strlen(PChar(VolumeName)));
			QByteArray s = QByteArray("\\??\\");
			s += Buffer.mid(4,Buffer.size()-5);    /* Strip the first \.\ and the last \ */
			printf("s: %s\n",s.data());
			QString VolumeLink;
			VolumeLink = NativeReadLink(s);
			vd.volumeLink = QByteArray(VolumeLink.toStdString().c_str());

			// Check if the link name belongs to the link names of interest
			bool foundallowedlinkname=false;
			for(unsigned i=0;i<AllowedLinkName.size();i++)
			{
				if(vd.volumeLink.indexOf(AllowedLinkName[i].c_str())!=-1)
					foundallowedlinkname=true;
			}
			if(AllowedLinkName.size()==0 || foundallowedlinkname)
			{
				// Find mount point in previously iterated NT drives
				int idx=DriveList.indexOf(vd.volumeName);
				if(idx!=-1)
					vd.mountPoint=QByteArray(1,'a'+idx)+QByteArray(":\\");
				else
					vd.mountPoint=QByteArray("");

				QByteArray Description;
				//printf("Calling TestDevice\n");
				//TestDevice(vd.volumeName,Description);

				printf("Before GetVolumeInformationA\n");
				rv = GetVolumeInformationA(Buffer.data(), userName, 1024, &serialNumber, &componentLength, &dwSysFlags, FileSysNameBuffer, 1024);
				printf("After GetVolumeInformationA rv: %d\n",rv);
				if(rv==0)
				{
					DWORD dw = GetLastError();
					printf("Get volume information failed. error code: %d\n",dw);
					//getlasterror(dw);

					vd.Name=QByteArray("");
					vd.dwSysFlags=0;
					vd.fileSystem=QByteArray("");
					vd.serialNumber=0;
				}
				else
				{
					vd.Name=QByteArray(userName);
					vd.dwSysFlags=dwSysFlags;
					vd.fileSystem=QByteArray(FileSysNameBuffer);
					vd.serialNumber=serialNumber;

					printf("printfing\n");
					printf("File system name: %s\n",vd.fileSystem.data());
					printf("volume name: %s\n",vd.Name.data());
					printf("Serial Number: %X\n",vd.serialNumber);
					printf("Flags: %lX\n",vd.dwSysFlags);
				}




				printf("Calling GetGeometryEx");

				int GMediaType,GTracksPerCylinder,GSectorsPerTrack,GBytesPerSector;
				long long GCylinders,GDiskSize,GDiskSize2;
				if(GetGeometryEx(vd.volumeLink,GMediaType,GCylinders,GTracksPerCylinder,GSectorsPerTrack,GBytesPerSector,GDiskSize))
				{
				 vd.MediaType = GMediaType;
				 vd.Cylinders = GCylinders;
				 vd.TracksPerCylinder = GTracksPerCylinder;
				 vd.SectorsPerTrack = GSectorsPerTrack;
				 vd.BytesPerSector = GBytesPerSector;
				 vd.DiskSize = GDiskSize;
				}
				else
				{
					vd.MediaType = -1;
					vd.Cylinders = -1;
					vd.TracksPerCylinder = -1;
					vd.SectorsPerTrack = -1;
					vd.BytesPerSector = -1;
					vd.DiskSize = -1;
				}

				bool ok;
				int method;
				//ok = GetSize(vd.volumeName,GDiskSize2,method);
				//printf("Getsize '%s' ok %d method %d %lld\n",vd.volumeName.data(),(int)ok,method,GDiskSize2);
				//printf("method: %d\n",method);
				ok = GetSize(vd.volumeLink,GDiskSize2,method);
				printf("Getsize '%s' ok %d method %d %lld\n",vd.volumeLink.data(),(int)ok,method,GDiskSize2);
				printf("method: %d\n",method);
				if(ok)
				{
					vd.DiskSize2 = GDiskSize2;
				}
				else
				{
					vd.DiskSize2 = -1;
				}

				/*ok = GetSize(s,GDiskSize2,method);
				printf("Getsize '%s' ok %d method %d %lld\n",s.data(),(int)ok,method,GDiskSize2);
				printf("method: %d\n",method);*/

				vvd.push_back(vd);
			}
		}

      Buffer.reserve(size);
      if(!FindNextVolumeA(h, (LPSTR)Buffer.data(),size))
      {
         FindVolumeClose(h);
         h = INVALID_HANDLE_VALUE;
      }
   }
   printf("returning from getlistofvolumes\n");
   return vvd;
}


HANDLE NTCreateFile(QByteArray FileName,DWORD dwDesiredAccess,DWORD dwShareMode,PSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
   UNICODE_STRING UName;
   QString FileNameW;
   OBJECT_ATTRIBUTES ObjectAttributes;
   IO_STATUS_BLOCK IOStatus;
   NTSTATUS Status;
   //DWORD ErrorNo;
   HANDLE handle;

   NativeInit();

   FileNameW=QString(FileName);

   RtlInitUnicodeString(&UName,(PWCHAR)FileNameW.data());


   ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
   ObjectAttributes.RootDirectory = 0;
   ObjectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
   ObjectAttributes.ObjectName = &UName;
   ObjectAttributes.SecurityDescriptor = 0;
   ObjectAttributes.SecurityQualityOfService = 0;


   Status = NtOpenFile(&handle,dwDesiredAccess | SYNCHRONIZE, &ObjectAttributes, &IOStatus, 0, FILE_SYNCHRONOUS_IO_NONALERT);

   if(Status<0)
   {
      handle = INVALID_HANDLE_VALUE;
      //printf("NtOpenFile failed '%s'\n",FileName.data());
   }
   //else
      //printf("NtOpenFile success '%s'\n",FileName.data());

   //ErrorNo := RtlNtStatusToDosError(Status);

   //SetLastError(ErrorNo);
   //GetLastError := ErrorNo;
//   Debug('Win32 Error = (' + IntToStr(GetLastError) + ') ' + SysErrorMessage(GetLastError), DebugOff);
   return handle;

}


DWORD CtlCode(DWORD DeviceType, DWORD Func, DWORD Method, DWORD Access)
{
   return (DeviceType<<16) | (Access<<14) | (Func<<2) | (Method);
}

/**
   Retrieves the physical location of a specified volume on one or more disks.

**/
bool GetDiskExtents(HANDLE hFile,QByteArray &Device,uint64_t &Offset, uint64_t &Len)
{
   QByteArray Buffer;
   PVOLUME_DISK_EXTENTS Volume;
   DWORD BytesReturned;
   int i;
   bool result;
   int size=1024;

   //printf("GetDiskExtents\n");

   Buffer.reserve(size);

   result = false;

   if(DeviceIoControl(hFile, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 0, 0,Buffer.data(),size,&BytesReturned, 0))

   {
      Buffer.resize(BytesReturned);

      Len=0;

      Volume = (PVOLUME_DISK_EXTENTS)Buffer.data();
      for(i=0; i<Volume->NumberOfDiskExtents; i++)
      {
         Device = QByteArray("\\Device\\Harddisk") + QByteArray(QString("%1").arg(Volume->Extents[i].DiskNumber).toStdString().c_str()) + QByteArray("\\Partition0");
         Offset = Volume->Extents[i].StartingOffset.QuadPart;
         Len    += Volume->Extents[i].ExtentLength.QuadPart;

         /*printf("i: %d\n",i);
         printf("\tdisknumber %d\n",Volume->Extents[i].DiskNumber);
         printf("StartingOffset %lld\n",Volume->Extents[i].StartingOffset.QuadPart);
         printf("ExtentLength %lld\n",Volume->Extents[i].ExtentLength.QuadPart);
         printf("%s %lld %lld\n",Device.data(),Offset,Len);*/

         result = true;
      }
   } 
   return result;
}

bool GetPartitionSize(HANDLE h,long long &len)
{
   QByteArray Buffer;
   DWORD BytesReturned;
   PPARTITION_INFORMATION P;
   int size=1024;

   //printf("GetPartitionSize\n");

   Buffer.reserve(size);

   if(DeviceIoControl(h,IOCTL_DISK_GET_PARTITION_INFO, 0, 0, Buffer.data(), size, &BytesReturned, 0))
   {
      P = (PPARTITION_INFORMATION)Buffer.data();
      len = P->PartitionLength.QuadPart;
      return true;
   }
   return false;
}



bool GetDiskSize(HANDLE h,long long len)
{
   QByteArray Buffer;
   DWORD BytesReturned;
   PDISK_GEOMETRY_EX G;
   LARGE_INTEGER Size;
   DWORD Error;
   DWORD Read;
   uint64_t result;
   int size=1024;

   printf("GetDiskSize\n");

   result = false;
   len = 0;
   Buffer.reserve(size);


   // IOCTL_DISK_GET_DRIVE_GEOMETRY_EX
   if(DeviceIoControl(h,CtlCode(IOCTL_DISK_BASE, 0x28, METHOD_BUFFERED, FILE_ANY_ACCESS), 0, 0, Buffer.data(),size, &BytesReturned, 0))
   {
      G = (PDISK_GEOMETRY_EX)Buffer.data();
      //Log('Disk size is ' + IntToStr(G.DiskSize.QuadPart));
      len = G->DiskSize.QuadPart;
      return true;
   }

   // the old way...  This is not accurate
   // IOCTL_DISK_GET_DRIVE_GEOMETRY   CTL_CODE(IOCTL_DISK_BASE, 0x0000, METHOD_BUFFERED, FILE_ANY_ACCESS)
   if(DeviceIoControl(h,CtlCode(IOCTL_DISK_BASE, 0x0, METHOD_BUFFERED, FILE_ANY_ACCESS), 0, 0,Buffer.data(), size, &BytesReturned, 0))
   {
      G = (PDISK_GEOMETRY_EX)Buffer.data();
      //Log(IntToStr(G.Geometry.Cylinders.QuadPart));
      //Log(IntToStr(G.Geometry.TracksPerCylinder));
      //Log(IntToStr(G.Geometry.SectorsPerTrack));
      result = G->Geometry.Cylinders.QuadPart * (G->Geometry.TracksPerCylinder /*+ 1*/) * (G->Geometry.SectorsPerTrack /*+ 1*/) * G->Geometry.BytesPerSector;
      //Log('Total = ' + IntToStr(Result));

      // Fish around for the correct value...
      // This can break USB devices, but USB is not supported on NT4
      while(1)
      {
         Size.QuadPart = result;
         Size.LowPart = SetFilePointer(h, Size.LowPart, &Size.HighPart, FILE_BEGIN);
         if(Size.LowPart == 0xFFFFFFFF)
         {
            Error = GetLastError();
            if(Error != NO_ERROR)
            {
               //Log(SysErrorMessage(GetLastError));
               break;
            }
         }
         if(ReadFile(h,Buffer.data(),512, &Read, 0))
         {
            result = result + 512;
         }
         else
         {
            break;
         }
      }
      //Log('Total = ' + IntToStr(Result));
      len = result;
      return true;
   }
   return false;
}



/*
   Generic GetSize functions that calls the right stuff according to the kind of handle.
*/
bool GetSizeHandle(HANDLE h,long long &len,int &method)
{
   QByteArray NewDevice;
   uint64_t NewOffset;
   uint64_t NewLength;
   uint64_t result;


   if(GetDiskExtents(h, NewDevice, NewOffset, NewLength))
   {
      len = NewLength;
      method=0;
      return true;
   }
   bool ok = GetPartitionSize(h,len);
   if(ok)
   {
      method=1;
      return true;
   }
   ok = GetDiskSize(h,len);
   if(ok)
   {
      method=2;
      return true;
   }
   return false;
}
bool GetSize(QByteArray DeviceName,long long &size,int &method)
{
   bool result=true;
   HANDLE h;

   size=-1;

   h = NTCreateFile(DeviceName.data(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
   if(h!=INVALID_HANDLE_VALUE)
   {
      bool ok = GetSizeHandle(h,size,method);
      CloseHandle(h);
      if(!ok)
         result = false;
   }
   else
   {
      result=false;
   }
   return result;
}


QByteArray MediaDescription(int Media)
{
   QByteArray result;
   switch(Media)
   {
      case Media_Type_F5_1Pt2_512:
         result = "5.25, 1.2MB,  512 bytes/sector";
         break;
      case Media_Type_F3_1Pt44_512:
         result = "3.5,  1.44MB, 512 bytes/sector";
         break;
      case Media_Type_F3_2Pt88_512:
         result = "3.5,  2.88MB, 512 bytes/sector";
         break;
      case Media_Type_F3_20Pt8_512:
         result = "3.5,  20.8MB, 512 bytes/sector";
         break;
      case Media_Type_F3_720_512:
         result = "3.5,  720KB,  512 bytes/sector";
         break;
      case Media_Type_F5_360_512:
         result = "5.25, 360KB,  512 bytes/sector";
         break;
      case Media_Type_F5_320_512:
         result = "5.25, 320KB,  512 bytes/sector";
         break;
      case Media_Type_F5_320_1024:
         result = "5.25, 320KB,  1024 bytes/sector";
         break;
      case Media_Type_F5_180_512:
         result = "5.25, 180KB,  512 bytes/sector";
         break;
      case Media_Type_F5_160_512:
         result = "5.25, 160KB,  512 bytes/sector";
         break;
      case Media_Type_RemovableMedia:
         result = "Removable media other than floppy";
         break;
      case Media_Type_FixedMedia:
         result = "Fixed hard disk media";
         break;
      default:
        result = "Unknown";
   }
   return result;
}


/**
   GetGeometry x: requires a device name of the form: \Device\HarddiskVolume3 or \Device\CdRom0
   or \Device\Harddisk0\Partition0

**/
bool GetGeometryEx(QByteArray DeviceName,int &MediaType,long long int &Cylinders,int &TracksPerCylinder,int &SectorsPerTrack,
                   int &BytesPerSector,long long &DiskSize)
{
   bool result=true;
   HANDLE h;
   int size=5120;
   char data[size];
   DISK_GEOMETRY_EX *GeometryEx = (DISK_GEOMETRY_EX*)data;        // Need to reserve space
   DWORD Len;

   memset(data,0,5120);    // Clear the GeometryEx structure

   MediaType=-1;
   Cylinders=-1;
   TracksPerCylinder=-1;
   SectorsPerTrack=-1;
   BytesPerSector=-1;
   DiskSize=-1;


   printf("GetGeometryEx '%s'\n",DeviceName.data());

   h = NTCreateFile(DeviceName.data(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
   if(h!=INVALID_HANDLE_VALUE)
   {
      // get the geometry...
      if(DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, 0, 0, GeometryEx, size, &Len, 0))
      {
         MediaType=GeometryEx->Geometry.MediaType;
         Cylinders=GeometryEx->Geometry.Cylinders.QuadPart;
         TracksPerCylinder=GeometryEx->Geometry.TracksPerCylinder;
         SectorsPerTrack=GeometryEx->Geometry.SectorsPerTrack;
         BytesPerSector=GeometryEx->Geometry.BytesPerSector;
         DiskSize=GeometryEx->DiskSize.QuadPart;

         /*printf("Geometry Ex:\n");
         printf("\tCylinders: %lld\n",GeometryEx->Geometry.Cylinders.QuadPart);
         printf("\tTracksPerCylinder: %d\n",GeometryEx->Geometry.TracksPerCylinder);
         printf("\tSectorsPerTrack: %d\n",GeometryEx->Geometry.SectorsPerTrack);
         printf("\tBytesPerSector: %d\n",GeometryEx->Geometry.BytesPerSector);
         printf("\tMediaType: %d\n",GeometryEx->Geometry.MediaType);
         printf("\tDiskSize: %llu\n",GeometryEx->DiskSize);*/

         PDISK_DETECTION_INFO ddi = DiskGeometryGetDetect(GeometryEx);
         PDISK_PARTITION_INFO dpi = DiskGeometryGetPartition(GeometryEx);
         //printf("\tSize of partition info: %d\n",dpi->SizeOfPartitionInfo);
         if(dpi->SizeOfPartitionInfo)
         {
            /*printf("\tPartition style: %s\n", (dpi->PartitionStyle==PARTITION_STYLE_MBR) ?
                                                   "MBR" :
                                                   ((dpi->PartitionStyle==PARTITION_STYLE_GPT)?"GPT":"RAW"));
            if(dpi->PartitionStyle==PARTITION_STYLE_MBR)
               printf("\tMBR Signature: %X\n",dpi->Mbr.Signature);
            if(dpi->PartitionStyle==PARTITION_STYLE_GPT)
               printf("\tMBR Signature: %X %X %X %X\n",dpi->Gpt.DiskId.Data1,dpi->Gpt.DiskId.Data2,dpi->Gpt.DiskId.Data3,dpi->Gpt.DiskId.Data4);

            printf("ddi: %p\n",ddi);
            printf("\tSize of detection info: %d\n",ddi->SizeOfDetectInfo);*/
         }
         if(ddi->SizeOfDetectInfo)
         {
            //printf("\tType of detection info: %s\n",(ddi->DetectionType==DetectExInt13)?"Extended Int13":((ddi->DetectionType==DetectInt13)?"Int13":((ddi->DetectionType==DetectNone)?"None":"NA")));
            /*if(ddi->DetectionType==DetectExInt13)
            {

            }
            if(ddi->DetectionType==DetectInt13)
            {
               printf("\tInt13:\n");
               printf("\t\tDrive)
            }*/
         }
      }
      else
      {
         //printf("DeviceIoControl not ok\n");
         result=false;
      }
      CloseHandle(h);
   }
   else
   {
      printf("Error\n");
      result=false;

   }
   return result;
}




bool NativeInit()
{
   FARPROC proc;;

   if(!module_handle)
   {
      module_handle = GetModuleHandleA("ntdll.dll");
      printf("module_handle: %d\n",module_handle);
   }

   if(!RtlInitUnicodeString)
   {
      proc = GetProcAddress( module_handle, "RtlInitUnicodeString");
      RtlInitUnicodeString = (void (*)(PUNICODE_STRING,PCWSTR))proc;
      printf("RtlInitUnicodeString: %p\n",RtlInitUnicodeString);
   }

   if(!NtOpenDirectoryObject)
   {
      proc = GetProcAddress( module_handle, "NtOpenDirectoryObject");
      NtOpenDirectoryObject = (NTSTATUS WINAPI(*)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES))proc;
      printf("NtOpenDirectoryObject: %p\n",NtOpenDirectoryObject);
   }

   if(!NtQueryDirectoryObject)
   {
      proc = GetProcAddress(module_handle,"NtQueryDirectoryObject");
      NtQueryDirectoryObject = (NTSTATUS WINAPI (*)(HANDLE,PVOID,ULONG,BOOLEAN,BOOLEAN,PULONG,PULONG))proc;
      printf("NtQueryDirectoryObject: %p\n",NtQueryDirectoryObject);
   }

   if(!NtOpenSymbolicLinkObject)
   {
      proc = GetProcAddress(module_handle,"NtOpenSymbolicLinkObject");
      NtOpenSymbolicLinkObject = (NTSTATUS WINAPI (*)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES))proc;
      printf("NtOpenSymbolicLinkObject: %p\n",NtOpenSymbolicLinkObject);
   }
   if(!NtQuerySymbolicLinkObject)
   {
      proc = GetProcAddress(module_handle,"NtQuerySymbolicLinkObject");
      NtQuerySymbolicLinkObject = (NTSTATUS WINAPI (*)(HANDLE,PUNICODE_STRING,PULONG))proc;
      printf("NtQuerySymbolicLinkObject: %p\n",NtQuerySymbolicLinkObject);
   }
   if(!NtOpenFile)
   {
      proc = GetProcAddress(module_handle,"NtOpenFile");
      NtOpenFile = (NTSTATUS WINAPI (*)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES,PIO_STATUS_BLOCK,ULONG,ULONG))proc;
      printf("NtOpenFile: %p\n",NtOpenFile);
   }

}

bool NativeDir(QString Dir,QStringList &List)
{
   UNICODE_STRING UName;
   OBJECT_ATTRIBUTES ObjectAttributes;
   NTSTATUS Status;
   HANDLE hObject;
   ULONG index;
   QByteArray Data;
   POBJECT_DIRECTORY_INFORMATION DirObjInformation;
   ULONG dw;
   bool result=true;


   NativeInit();

   RtlInitUnicodeString(&UName,(const WCHAR*)Dir.data());



   ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
   ObjectAttributes.RootDirectory = 0;
   ObjectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
   ObjectAttributes.ObjectName = &UName;
   ObjectAttributes.SecurityDescriptor = 0;
   ObjectAttributes.SecurityQualityOfService = 0;



   Status = NtOpenDirectoryObject(&hObject,STANDARD_RIGHTS_READ | DIRECTORY_QUERY,&ObjectAttributes);

   //printf("NtOpenDirectoryObject rv: %d\n",Status);
   if(Status>=0)
   {
      index = 0;
      while(1)
      {
         Data.resize(1024);
         memset(Data.data(),0,Data.size());
         //printf("data size: %d\n",Data.size());

         DirObjInformation = (POBJECT_DIRECTORY_INFORMATION)Data.data();
         Status = NtQueryDirectoryObject(
                     hObject,
                     DirObjInformation,
                     Data.size(),
                     TRUE,         // get next index
                     FALSE,        // don't ignore index input
                     &index,
                     &dw);         // can be NULL

         if(Status>=0)
         {

            QString name;

            unsigned l = GetUnicodeLength(DirObjInformation->Name.Buffer);
            //printf("Got length %d\n",l);
            name.reserve(l);
            memcpy(name.data(),DirObjInformation->Name.Buffer,l*2);
            name.resize(l);
            //printf("Name: %s\n",name.toStdString().c_str());

            List << name;
            }
         else
         {
            //printf("NtQueryDirectoryObject fail %lX\n",Status);
            break;
         }
      }

      //NtClose(hObj);
      CloseHandle(hObject);
      //printf("CloseHandle rv: %d\n",rv);
   }
   else
   {
      //printf("NtOpenDirectoryObject failed: %lX\n", Status);
      result = false;
   }
   return result;
}

/**
**/
QString NativeReadLink(QString Link )
{
   UNICODE_STRING UName;
   OBJECT_ATTRIBUTES ObjectAttributes;
   NTSTATUS Status;
   HANDLE hObject;
   QString Data;
   ULONG dw;

   QString Result("");

   //printf("Native read link '%s'\n",Link.toStdString().c_str());


   NativeInit();

   RtlInitUnicodeString(&UName,(const WCHAR*)Link.data());

   ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
   ObjectAttributes.RootDirectory = 0;
   ObjectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
   ObjectAttributes.ObjectName = &UName;
   ObjectAttributes.SecurityDescriptor = 0;
   ObjectAttributes.SecurityQualityOfService = 0;


   Status = NtOpenSymbolicLinkObject(&hObject,SYMBOLIC_LINK_QUERY,&ObjectAttributes);
   //printf("NtOpenSymbolicLinkObject status %d\n",Status);

   if(Status>=0)
   {
      UName.Length = 0;
      int size = 1024;
      Data.reserve(size);
      UName.MaximumLength = size*2;
      UName.Buffer = (PWCHAR)Data.data();

      Status = NtQuerySymbolicLinkObject(hObject,&UName,&dw);
      //printf("NtQuerySymbolicLinkObject status %d\n",Status);
      // UName: \Device\HarddiskVolume3, \Device\Floppy0
      if(Status>=0)
      {
         //printf("Uname.Length: %d\n",UName.Length);
         FixUnicodeLength(Data,size);
         //printf("Uname '%s'\n",Data.toStdString().c_str());
         Result = Data;


         //SetLength(Data, UName.Length div 2);
         //Result = Data;
      }

      CloseHandle(hObject);
   }
   return Result;
}

bool StartsWith(QString S, QString Start, QString &Value)
{
   int p = S.indexOf(Start);
   if(p==0)
   {
      Value = S.mid(Start.size());
      return true;
   }
   return false;
}

void ParseNativeDir(QStringList List)
{
   printf("Number of elements: %d\n",List.size());

   QString Number;
   QString DeviceName;
   QStringList Harddisks;
   int DriveNo;
   int PartNo;
   bool ok;

   for(unsigned i=0;i<List.size();i++)
   {
      DeviceName.clear();
      //printf("DeviceName. size %d\n",DeviceName.size());
      QString name = List[i];
      //printf("Name: %s\n",name.toStdString().c_str());
      if(StartsWith(name,"CdRom",Number))
      {
         DriveNo = Number.toInt(&ok);
         if(ok)
         {
            DeviceName = QString("\\Device\\CdRom%1").arg(DriveNo);
         }
      }
      if(StartsWith(name,"Floppy",Number))
      {
         DriveNo = Number.toInt(&ok);
         if(ok)
         {
            DeviceName = QString("\\Device\\Floppy%1").arg(DriveNo);
         }
      }
      if(StartsWith(name,"Harddisk",Number))
      {
         DriveNo = Number.toInt(&ok);
         if(ok)
         {
            DeviceName = QString("\\Device\\Harddisk%1").arg(DriveNo);
            Harddisks << DeviceName;
         }
      }

      if(DeviceName.size())
      {
         printf("%s\n",DeviceName.toStdString().c_str());
      }
   }
   // Do something with the hard disks
   for(unsigned i=0;i<Harddisks.size();i++)
   {
      QStringList Devices;
      printf("Harddisk %s\n",Harddisks[i].toStdString().c_str());
      NativeDir(Harddisks[i], Devices);
      for(unsigned j=0;j<Devices.size();j++)
      {
         printf("%s - %s\n",Harddisks[i].toStdString().c_str(),Devices[j].toStdString().c_str());
         if(StartsWith(Devices[j],"Partition",Number))
         {
            PartNo = Number.toInt(&ok);
            if(ok)
            {
               DeviceName = Harddisks[i] + QString("\\Partition%1").arg(PartNo);
               //printf("DeviceName")
            }
         }
      }

   }

}
/******************************************************************************
*******************************************************************************
LowLevelFile   LowLevelFile   LowLevelFile   LowLevelFile   LowLevelFile
*******************************************************************************
*******************************************************************************
Low level file access using NT undocumented functions.

All functions with bool return value indicate errors by returning false.

******************************************************************************/
LowLevelFile::LowLevelFile(QByteArray _fn)
{
	fileName=_fn;
}
LowLevelFile::~LowLevelFile()
{
	close();

}
bool LowLevelFile::open()
{
	h = NTCreateFile(fileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,0);
	if(h!=INVALID_HANDLE_VALUE)
		return true;
	return false;
}
bool LowLevelFile::close()
{
	if(h != INVALID_HANDLE_VALUE)
	{
		CloseHandle(h);
		h = INVALID_HANDLE_VALUE;
	}
}

bool LowLevelFile::read(int size,unsigned char *buffer,int &read)
{
	if(h!=INVALID_HANDLE_VALUE)
	{
		DWORD len;
		bool rv = ReadFile(h,buffer,size,&len,0);

		read = len;
		if(rv==false)
			return false;
		return true;
	}

	return false;
}

bool LowLevelFile::seek(unsigned long long pos,unsigned long long &newpos)
{
	if(h!=INVALID_HANDLE_VALUE)
	{
		LONG sh,sl;
		sl = pos;
		sh = pos>>32;
		DWORD rv = SetFilePointer(h,sl,&sh,FILE_BEGIN);
		if(rv == INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR)		// Failure only when low part is invalid, and getlasterror is indicating an error
			return false;

		newpos = sh;
		newpos <<=32;
		newpos|=(unsigned long long)sl;
		return true;
	}
	return false;
}

