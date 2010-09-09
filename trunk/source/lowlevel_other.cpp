/*
std::vector<VOLUMEDEF> GetListOfMountPoints(QStringList List)
{
   //QString Buffer;
   QByteArray Buffer;
   HANDLE h;
   DWORD   dwSysFlags;         // flags that describe the file system
   char    FileSysNameBuffer[1024];
   char volumeName[1024];
   DWORD serialNumber;
   DWORD componentLength;
   unsigned size=1024;

   std::vector<VOLUMEDEF> vvd;
   VOLUMEDEF vd;


   Buffer.reserve(size);

   // Enumerate the volumes
   h = FindFirstVolumeA((LPSTR)Buffer.data(),size);
   while(h!=INVALID_HANDLE_VALUE)
   {
      FixAnsiLength(Buffer,size);
      printf("--------------------------------------------------------------------------------\n");
      printf("Volume: %s\n",Buffer.data());

      vd.volumeName=Buffer;

      int rv;
      rv = GetDriveTypeA(Buffer.data());
      vd.driveType=rv;
      printf("Drive type: %d\n",rv);


      rv = GetVolumeInformationA(Buffer.data(), volumeName, 1024, &serialNumber, &componentLength, &dwSysFlags, FileSysNameBuffer, 1024);
      if(rv==0)
      {
         DWORD dw = GetLastError();
         printf("Get volume information failed. error code: %d\n",dw);
         //getlasterror(dw);

         vd.volumeName=QByteArray("");
         vd.dwSysFlags=0;
         vd.fileSystem=QByteArray("");
         vd.serialNumber=0;
      }
      else
      {

         vd.volumeName=QByteArray(volumeName);
         vd.dwSysFlags=dwSysFlags;
         vd.fileSystem=QByteArray(FileSysNameBuffer);
         vd.serialNumber=serialNumber;


         printf("System flags: %d\n", dwSysFlags);
         printf("File system name: %s\n", FileSysNameBuffer);
         printf("volume name: %s\n",volumeName);
         printf("Serial Number: %X\n",serialNumber);
         printf("Flags: %lX\n",dwSysFlags);
         if(dwSysFlags & FILE_NAMED_STREAMS)
            printf(" The file system supports named streams.\n");
         if(dwSysFlags & FILE_READ_ONLY_VOLUME)
            printf(" The specified volume is read-only.\n");
         if(dwSysFlags & FILE_SUPPORTS_OBJECT_IDS)
            printf(" The file system supports object identifiers. \n");
         if(dwSysFlags & FILE_SUPPORTS_REPARSE_POINTS)
            printf(" The file system supports re-parse points. \n");
         if(dwSysFlags & FILE_SUPPORTS_SPARSE_FILES)
            printf(" The file system supports sparse files. \n");
         if(dwSysFlags & FILE_VOLUME_QUOTAS)
            printf(" The file system supports disk quotas. \n");
         if(dwSysFlags & FS_CASE_IS_PRESERVED)
            printf(" The file system preserves the case of file names when it places a name on disk. \n");
         if(dwSysFlags & FS_CASE_SENSITIVE)
            printf(" The file system supports case-sensitive file names. \n");
         if(dwSysFlags & FS_FILE_COMPRESSION)
            printf(" The file system supports file-based compression. \n");
         //if(dwSysFlags & FS_FILE_ENCRYPTION)
//            printf(" The file system supports the Encrypted File System (EFS). \n");
         if(dwSysFlags & FS_PERSISTENT_ACLS)
            printf(" The file system preserves and enforces access control lists (ACL).\n");
         if(dwSysFlags & FS_UNICODE_STORED_ON_DISK)
            printf(" The file system supports Unicode in file names as they appear on disk. \n");
         if(dwSysFlags & FS_VOL_IS_COMPRESSED)
            printf(" The specified volume is a compressed volume.\n");
         if(!(dwSysFlags & FILE_SUPPORTS_REPARSE_POINTS))
         {
            printf("\nThis file system does not support volume mount points.\n");
         }
         else
         {
            printf("\nThis file system supports volume mount points.\n");
            //EnumerateVolumeMountPoints(Buffer);
            FindVolumeMountPoints(Buffer);
         }

         // Get some more infos
         //SetLength(VolumeName, strlen(PChar(VolumeName)));
         QByteArray s = QByteArray("\\??\\");
         s += Buffer.mid(4,Buffer.size()-5);    // Strip the first \.\ and the last \
         printf("s: %s\n",s.data());
         QString VolumeLink;
         VolumeLink = NativeReadLink(s);
         vd.volumeLink = QByteArray(VolumeLink.toStdString().c_str());

      }



      vvd.push_back(vd);

      Buffer.reserve(size);
      if(!FindNextVolumeA(h, (LPSTR)Buffer.data(),size))
      {
         FindVolumeClose(h);
         h = INVALID_HANDLE_VALUE;
      }
   }
   return vvd;
}
*/


/**
   GetGeometry : requires a device name of the form: \Device\HarddiskVolume3 or \Device\CdRom0

**/
bool GetGeometry(QByteArray DeviceName,QByteArray Description)
{
   HANDLE h;
   bool result=false;
   int64_t size;
   NTSTATUS Status;
   DISK_GEOMETRY Geometry;
   DWORD Len;

   size = 0;





   printf("TestDevice '%s'\n",DeviceName.data());

   IO_STATUS_BLOCK IoStatusBlock;

   h = NTCreateFile(DeviceName.data(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
   printf("Handle: %d\n",h);

   if(h!=INVALID_HANDLE_VALUE)
   {
      printf("NTCreateFileA ok\n");
      //try
         //Log('Opened ' + DeviceName);
         result = true;

         // get the geometry...
         //if(DeviceIoControl(h, CtlCode(IOCTL_DISK_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS), 0, 0, &Geometry, sizeof(Geometry), &Len, 0))
         if(DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_GEOMETRY, 0, 0, &Geometry, sizeof(Geometry), &Len, 0))

         {
            printf("Geometry:\n");
            printf("\tCylinders: %d\n",Geometry.Cylinders);
            printf("\tTracksPerCylinder: %d\n",Geometry.TracksPerCylinder);
            printf("\tSectorsPerTrack: %d\n",Geometry.SectorsPerTrack);
            printf("\tBytesPerSector: %d\n",Geometry.BytesPerSector);
            printf("\tMediaType: %d\n",Geometry.MediaType);
            //QByteArray einfoa = einfo.toAscii();
            Description = MediaDescription(Geometry.MediaType)+QString("\n Bytes per sector: %1").arg(Geometry.BytesPerSector).toAscii();
            printf("desc: %s\n",Description.data());
            //Description = MediaDescription(Geometry.MediaType) + "Hello";

                          //+ QByteArray(". Block size = ") + QByteArray(QString("%1").arg(Geometry.BytesPerSector).toStdString().c_str());
            printf("DeviceIoControl ok - after add\n");
//               Size.QuadPart := Geometry.Cylinders.QuadPart * Geometry.TracksPerCylinder * Geometry.SectorsPerTrack * Geometry.BytesPerSector;
//               Log('size = ' + IntToStr(Size.QuadPart));

         }
         else
         {
            printf("DeviceIoControl not ok\n");
            Description="";
         }
         /*else
         {
            Geometry.MediaType = Media_Type_Unknown;
            //ShowError('reading geometry');
         }
         if(Filter != "")
         {
            result = FilterMatch(DeviceName, Geometry.MediaType, Filter);
         }*/
         //printf("Try getsize\n");
         //size = GetSizeHandle(h);
      //finally
         CloseHandle(h);
      //end;
   }
   else
   {
      printf("Error\n");
   }
   return false;
}
