/*
 * fsproto.h - prototypes of the FS_xxx entry points of this driver
 *
 * Generated from the definitions in ifs/*.c (the netlabs FAT32 sources), so that the
 * driver's own functions can call each other before their definitions. The calling
 * convention is far pascal with the data segment loaded (_loadds).
 *
 * Copyright (c) 2026 OS2World, LGPL 2.1 or later; see doc/PROVENANCE.md.
 */

#ifndef FSEXFAT_FSPROTO_H_INCLUDED
#define FSEXFAT_FSPROTO_H_INCLUDED

int far pascal _loadds FS_ALLOCATEPAGESPACE(struct sffsi far *psffsi, struct sffsd far *psffsd, unsigned long ulSize, unsigned long ulWantContig);
int far pascal _loadds FS_ATTACH(unsigned short usFlag, char far * pDev, void far * pvpfsd, void far * pdevfsd, char far * pParm, unsigned short far * pLen);
int far pascal _loadds FS_CANCELLOCKREQUEST(struct sffsi far * psffsi, struct sffsd far * psffsd, void far * pLockRang);
int far pascal _loadds FS_CANCELLOCKREQUESTL(struct sffsi far * psffsi, struct sffsd far * psffsd, void far * pLockRang);
int far pascal _loadds FS_CHDIR(unsigned short usFlag, struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pDir, unsigned short usCurDirEnd);
int far pascal _loadds FS_CHGFILEPTR(struct sffsi far * psffsi, struct sffsd far * psffsd, long lOffset, unsigned short usType, unsigned short IOFlag);
int far pascal _loadds FS_CHGFILEPTRL(struct sffsi far * psffsi, struct sffsd far * psffsd, LONGLONG llOffset, unsigned short usType, unsigned short IOFlag);
int far pascal _loadds FS_CLOSE(unsigned short usType, unsigned short IOFlag, struct sffsi far * psffsi, struct sffsd far * psffsd);
int far pascal _loadds FS_COMMIT(unsigned short usType, unsigned short usIOFlag, struct sffsi far * psffsi, struct sffsd far * psffsd);
int far pascal _loadds FS_COPY(unsigned short usMode, struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pSrc, unsigned short usSrcCurDirEnd, char far * pDst, unsigned short usDstCurDirEnd, unsigned short usNameType);
int far pascal _loadds FS_DELETE(struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pFile, unsigned short usCurDirEnd);
int far pascal _loadds FS_DOPAGEIO(struct sffsi far * psffsi, struct sffsd far * psffsd, struct PageCmdHeader far * pPageCmdList);
void far pascal _loadds FS_EXIT(unsigned short usUid, unsigned short usPid, unsigned short usPdb);
int far pascal _loadds FS_FILEATTRIBUTE(unsigned short usFlag, struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pName, unsigned short usCurDirEnd, unsigned short far * pAttr);
int far pascal _loadds FS_FILEINFO(unsigned short usFlag, struct sffsi far * psffsi, struct sffsd far * psffsd, unsigned short usLevel, char far * pData, unsigned short cbData, unsigned short IOFlag);
int far pascal _loadds FS_FILEIO(struct sffsi far * psffsi, struct sffsd far * psffsd, char far * pCmdList, unsigned short cbCmdList, unsigned short far * poError, unsigned short IOFlag);
int far pascal _loadds FS_FILELOCKS(struct sffsi far * psffsi, struct sffsd far * psffsd, void far * pUnlockRange, void far * pLockRange, unsigned long ulTimeOut, unsigned long ulFlags);
int far pascal _loadds FS_FILELOCKSL(struct sffsi far * psffsi, struct sffsd far * psffsd, void far * pUnlockRange, void far * pLockRange, unsigned long ulTimeOut, unsigned long ulFlags);
int far pascal _loadds FS_FINDCLOSE(struct fsfsi far * pfsfsi, struct fsfsd far * pfsfsd);
int far pascal _loadds FS_FINDFIRST(struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pName, unsigned short usCurDirEnd, unsigned short usAttr, struct fsfsi far * pfsfsi, struct fsfsd far * pfsfsd, char far * pData, unsigned short cbData, unsigned short far * pcMatch, unsigned short usLevel, unsigned short usFlags);
int far pascal _loadds FS_FINDFROMNAME(struct fsfsi far * pfsfsi, struct fsfsd far * pfsfsd, char far * pData, unsigned short cbData, unsigned short far * pcMatch, unsigned short usLevel, unsigned long ulPosition, char far * pName, unsigned short usFlags);
int far pascal _loadds FS_FINDNEXT(struct fsfsi far * pfsfsi, struct fsfsd far * pfsfsd, char far * pData, unsigned short cbData, unsigned short far * pcMatch, unsigned short usLevel, unsigned short usFlags);
int far pascal _loadds FS_FINDNOTIFYCLOSE(unsigned short usHandle);
int far pascal _loadds FS_FINDNOTIFYFIRST(struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pName, unsigned short usCurDirEnd, unsigned short usAttr, unsigned short far * pHandle, char far * pData, unsigned short cbData, unsigned short far * pcMatch, unsigned short usLevel, unsigned long ulTimeOut);
int far pascal _loadds FS_FINDNOTIFYNEXT(unsigned short usHandle, char far * pData, unsigned short cbData, unsigned short far * pcMatch, unsigned short usInfoLevel, unsigned long ulTimeOut);
int far pascal _loadds FS_FLUSHBUF(unsigned short hVPB, unsigned short usFlag);
int far pascal _loadds FS_FSCTL(union argdat far * pArgDat, unsigned short usArgType, unsigned short usFunc, char far * pParm, unsigned short cbParm, unsigned short far * pcbParm, char far * pData, unsigned short cbData, unsigned short far * pcbData);
int far pascal _loadds FS_FSINFO(unsigned short usFlag, unsigned short hVBP, char far * pData, unsigned short cbData, unsigned short usLevel);
int far pascal _loadds FS_INIT(char far * pszParm, unsigned long pDevHlp, unsigned long far * pMiniFSD);
int far pascal _loadds FS_IOCTL(struct sffsi far * psffsi, struct sffsd far * psffsd, unsigned short usCat, unsigned short usFunc, char far * pParm, unsigned short cbParm, unsigned far * pcbParm, char far * pData, unsigned short cbData, unsigned far * pcbData);
int far pascal _loadds FS_MKDIR(struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pName, unsigned short usCurDirEnd, char far * pEABuf, unsigned short usFlags);
int far pascal _loadds FS_MOUNT(unsigned short usFlag, struct vpfsi far * pvpfsi, struct vpfsd far * pvpfsd, unsigned short hVBP, char far * pBoot);
int far pascal _loadds FS_MOVE(struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pSrc, unsigned short usSrcCurDirEnd, char far * pDst, unsigned short usDstCurDirEnd, unsigned short usFlags);
int far pascal _loadds FS_NEWSIZE(struct sffsi far * psffsi, struct sffsd far * psffsd, unsigned long ulLen, unsigned short usIOFlag);
int far pascal _loadds FS_NEWSIZEL(struct sffsi far * psffsi, struct sffsd far * psffsd, ULONGLONG ullLen, unsigned short usIOFlag);
int far pascal _loadds FS_NMPIPE(struct sffsi far * psffsi, struct sffsd far * psffsd, unsigned short usOpType, union npoper far * pOpRec, char far * pData, char far * pName);
int far pascal _loadds FS_OPENCREATE(struct cdfsi far * pcdfsi, void far * pcdfsd, char far * pName, unsigned short usCurDirEnd, struct sffsi far * psffsi, struct sffsd far * psffsd, unsigned long ulOpenMode, unsigned short usOpenFlag, unsigned short far * pAction, unsigned short usAttr, char far * pEABuf, unsigned short far * pfGenFlag);
int far pascal _loadds FS_OPENPAGEFILE(unsigned long far *pFlags, unsigned long far *pcMaxReq, char far * pName, struct sffsi far * psffsi, struct sffsd far * psffsd, unsigned short OpenMode, unsigned short OpenFlag, unsigned short Attr, unsigned long Reserved);
int far pascal _loadds FS_PATHINFO(unsigned short usFlag, struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pName, unsigned short usCurDirEnd, unsigned short usLevel, char far * pData, unsigned short cbData);
int far pascal _loadds FS_PROCESSNAME(char far * pNameBuf);
int far pascal _loadds FS_READ(struct sffsi far * psffsi, struct sffsd far * psffsd, char far * pData, unsigned short far * pLen, unsigned short usIOFlag);
int far pascal _loadds FS_RMDIR(struct cdfsi far * pcdfsi, struct cdfsd far * pcdfsd, char far * pName, unsigned short usCurDirEnd);
int far pascal _loadds FS_SETSWAP(struct sffsi far * psffsi, struct sffsd far * psffsd);
int far pascal _loadds FS_SHUTDOWN(unsigned short usType, unsigned long ulReserved);
int far pascal _loadds FS_VERIFYUNCNAME(unsigned short usFlag, char far * pName);
int far pascal _loadds FS_WRITE(struct sffsi far * psffsi, struct sffsd far * psffsd, char far * pData, unsigned short far * pLen, unsigned short usIOFlag);

#endif /* FSEXFAT_FSPROTO_H_INCLUDED */
