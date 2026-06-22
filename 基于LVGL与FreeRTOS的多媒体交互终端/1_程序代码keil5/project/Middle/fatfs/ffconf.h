/*---------------------------------------------------------------------------/
/  FatFs Functional Configurations
/---------------------------------------------------------------------------*/


/******************************************************************************************************************

                                                自定义配置

******************************************************************************************************************/
#define USE_FF_UNICODE			0			// 使用系统自带的字库编码
#define USE_SELF_UNICODE		1			// 使用自己的字库编码文件
#define USE_UNCODE				USE_SELF_UNICODE



#define FFCONF_DEF	86631	/* Revision ID */
/******************************************************************************************************************

                                                功能函数配置

******************************************************************************************************************/

/* 使能或禁止与写相关函数 即配置只读 */
#define FF_FS_READONLY	0

 


#define FF_FS_MINIMIZE	0
/*	
	定义最小化级别以删除一些基本API函数
	0:基本功能已完全开启
	1:f_stat(), f_getfree(), f_unlink(), f_mkdir(), f_truncate() and f_rename()被移除
	2:除1外 f_opendir(), f_readdir() and f_closedir()也被移除
	3:除2外 f_lseek()也被移除
 */



 

#define FF_USE_FIND		0
/* 	
	是否使用f_findfirst()和f_findnext()函数
	0:不使用 
	1:使用 
	2:Enable with matching altname[] too)
 */

/* 	是否使用f_mkfs()函数
 *	0:不使用 
 *	1:使用 
 */
#define FF_USE_MKFS		1


/* 	是否使用快速查找功能
 *	0:不使用 
 *	1:使用 
 */
#define FF_USE_FASTSEEK	1

/* 	是否使用f_expand()函数
 *	0:不使用 
 *	1:使用 
 */
#define FF_USE_EXPAND	0


/* 	是否使用切换属性操作 f_chmod() and f_utime()函数
 *	0:不使用 
 *	1:使用 
 *  此外，FF_FS_READONLY需要为0才能启用该选项
 */
#define FF_USE_CHMOD	0


/* 	是否使用切换卷标功能 f_getlabel() and f_setlabel()函数
 *	0:不使用 
 *	1:使用 
 */
#define FF_USE_LABEL	1


/* 	是否使用f_forward()函数
 *	0:不使用 
 *	1:使用 
 */
#define FF_USE_FORWARD	0


/* 	FF_USE_STRFUNC是否使用f_gets() f_putc() f_puts()和 f_printf()函数
 *	0:不使用 	FF_PRINT_LLI / FF_PRINT_FLOAT / FF_STRF_ENCODE不起作用
 *	1:使用 		不进行LF-CRLF转换
 *	2:使用 		进行LF-CRLF转换
 */
#define FF_USE_STRFUNC	1
#define FF_PRINT_LLI	0
#define FF_PRINT_FLOAT	0
#define FF_STRF_ENCODE	3
/* 	FF_PRINT_LLI = 1		使得f_printf()支持long long参数
	FF_PRINT_FLOAT = 1/2	使f_printf()支持浮点参数
	FF_LFN_UNICODE >= 1且启用LFN时，字符串函数转换字符编码在其中。
	FF_STRF_ENCODE选择文件上字符编码的假设通过这些函数来读/写
   0: ANSI/OEM in current CP
   1: Unicode in UTF-16LE
   2: Unicode in UTF-16BE
   3: Unicode in UTF-8
*/



/******************************************************************************************************************

                                                命名空间和本地环境配置

******************************************************************************************************************/

/* 设置字符编码类型 */
#define FF_CODE_PAGE	936


/* 是否支持长文件名 */
#define FF_USE_LFN		3
#define FF_MAX_LFN		255
/*
/   0: Disable LFN. FF_MAX_LFN has no effect.
/   1: Enable LFN with static  working buffer on the BSS. Always NOT thread-safe.
/   2: Enable LFN with dynamic working buffer on the STACK.
/   3: Enable LFN with dynamic working buffer on the HEAP.
/
/   FF_MAX_LFN 最大的文件名长度 
*/


#define FF_LFN_UNICODE	2
/* 当启用LFN时，此选项将切换API上的字符编码
/
/   0: ANSI/OEM in current CP (TCHAR = char)
/   1: Unicode in UTF-16 (TCHAR = WCHAR)
/   2: Unicode in UTF-8 (TCHAR = char)
/   3: Unicode in UTF-32 (TCHAR = DWORD)
/
/  Also behavior of string I/O functions will be affected by this option.
/  When LFN is not enabled, this option has no effect. */


#define FF_LFN_BUF		255
#define FF_SFN_BUF		12
/*
	这组选项定义了FILINFO结构中文件名成员的大小，用于读取目录项。这些值应该足以满足要读取的文件名。
	读取文件名的最大可能长度取决于关于字符编码
*/


#define FF_FS_RPATH		0
/* 	
	该选项配置对相对路径的支持。
	0:禁用相对路径，并移除相关功能。
	1:启用相对路径。F_chdir()和f_chdrive()可用。
	2:除1外，还有f_getcwd()函数可用
 */



/******************************************************************************************************************

                                                驱动/卷配置

******************************************************************************************************************/

/*设置FATFS支持的逻辑设备数目(1-10)*/
#define FF_VOLUMES		3  



#define FF_STR_VOLUME_ID	0
#define FF_VOLUME_STRS		"RAM","NAND","CF","SD","SD2","USB","USB2","USB3"
/* 
	当FF_STR_VOLUME_ID设置为1或2时，可以使用任意字符串作为路径名中的驱动器号。
	
	FF_VOLUME_STRS定义每个逻辑驱动器的卷ID字符串。项数不能小于FF_VOLUMES。
	卷ID字符串的有效字符是A-Z、A-Z和0-9，但是它们之间的比较不区分大小写。
	
	如果FF_STR_VOLUME_ID >= 1且没有定义FF_VOLUME_STRS，则用户自定义卷字符串表需要定义为:
	const char* VolumeStr[FF_VOLUMES] = {"ram","flash","sd","usb",...
*/


#define FF_MULTI_PARTITION	0
/*	
	该选项切换对物理驱动器上多个卷的支持。
	默认情况下(0)，每个逻辑驱动器号绑定到同一个物理驱动器 只有在物理驱动器上找到的FAT卷才会被挂载。
	当启用该功能时(1)，每个逻辑驱动器号可以绑定到VolToPart[]中列出的任意物理驱动器和分区。f_fdisk()函数也可用。
*/

#define FF_MIN_SS		512
#define FF_MAX_SS		512
/*	
	配置支持扇区大小的范围。(512、1024、2048或4096)
	对于大多数系统、通用存储卡和硬盘，始终设置512，但对于板载闪存和某些类型的光学介质，可能需要更大的值。
	当FF_MAX_SS大于FF_MIN_SS时，fatf配置为可变扇区大小模式，disk_ioctl()函数需要执行GET_SECTOR_SIZE命令。
 */
 
 
#define FF_LBA64		0
/*	
	该选项切换对64位LBA的支持。(0:Disable或1:Enable)
	要启用64位LBA，还需要启用exFAT。(ff_fs_exfat == 1) 
 */


#define FF_MIN_GPT		0x10000000
/* Minimum number of sectors to switch GPT as partitioning format in f_mkfs and
/  f_fdisk function. 0x100000000 max. This option has no effect when FF_LBA64 == 0. */


#define FF_USE_TRIM		0
/*
	配置使用FATFS为正常模式还是Tiny模式	(0:正常模式或1:Tiny模式)
	要启用Tiny模式，还需要对disk_ioctl()函数。
*/


/******************************************************************************************************************

                                                系统配置设置

******************************************************************************************************************/

#define FF_FS_TINY		0
/* This option switches tiny buffer configuration. (0:Normal or 1:Tiny)
/  At the tiny configuration, size of file object (FIL) is shrinked FF_MAX_SS bytes.
/  Instead of private sector buffer eliminated from the file object, common sector
/  buffer in the filesystem object (FATFS) is used for the file data transfer. */


#define FF_FS_EXFAT		1
/* This option switches support for exFAT filesystem. (0:Disable or 1:Enable)
/  To enable exFAT, also LFN needs to be enabled. (FF_USE_LFN >= 1)
/  Note that enabling exFAT discards ANSI C (C89) compatibility. */


#define FF_FS_NORTC		0
#define FF_NORTC_MON	1
#define FF_NORTC_MDAY	1
#define FF_NORTC_YEAR	2020
/* The option FF_FS_NORTC switches timestamp functiton. If the system does not have
/  any RTC function or valid timestamp is not needed, set FF_FS_NORTC = 1 to disable
/  the timestamp function. Every object modified by FatFs will have a fixed timestamp
/  defined by FF_NORTC_MON, FF_NORTC_MDAY and FF_NORTC_YEAR in local time.
/  To enable timestamp function (FF_FS_NORTC = 0), get_fattime() function need to be
/  added to the project to read current time form real-time clock. FF_NORTC_MON,
/  FF_NORTC_MDAY and FF_NORTC_YEAR have no effect.
/  These options have no effect in read-only configuration (FF_FS_READONLY = 1). */


#define FF_FS_NOFSINFO	0
/* If you need to know correct free space on the FAT32 volume, set bit 0 of this
/  option, and f_getfree() function at first time after volume mount will force
/  a full FAT scan. Bit 1 controls the use of last allocated cluster number.
/
/  bit0=0: Use free cluster count in the FSINFO if available.
/  bit0=1: Do not trust free cluster count in the FSINFO.
/  bit1=0: Use last allocated cluster number in the FSINFO if available.
/  bit1=1: Do not trust last allocated cluster number in the FSINFO.
*/


#define FF_FS_LOCK		0
/* The option FF_FS_LOCK switches file lock function to control duplicated file open
/  and illegal operation to open objects. This option must be 0 when FF_FS_READONLY
/  is 1.
/
/  0:  Disable file lock function. To avoid volume corruption, application program
/      should avoid illegal open, remove and rename to the open objects.
/  >0: Enable file lock function. The value defines how many files/sub-directories
/      can be opened simultaneously under file lock control. Note that the file
/      lock control is independent of re-entrancy. */


/* #include <somertos.h>	// O/S definitions */
#define FF_FS_REENTRANT	0
#define FF_FS_TIMEOUT	1000
#define FF_SYNC_t		HANDLE
/* The option FF_FS_REENTRANT switches the re-entrancy (thread safe) of the FatFs
/  module itself. Note that regardless of this option, file access to different
/  volume is always re-entrant and volume control functions, f_mount(), f_mkfs()
/  and f_fdisk() function, are always not re-entrant. Only file/directory access
/  to the same volume is under control of this function.
/
/   0: Disable re-entrancy. FF_FS_TIMEOUT and FF_SYNC_t have no effect.
/   1: Enable re-entrancy. Also user provided synchronization handlers,
/      ff_req_grant(), ff_rel_grant(), ff_del_syncobj() and ff_cre_syncobj()
/      function, must be added to the project. Samples are available in
/      option/syscall.c.
/
/  The FF_FS_TIMEOUT defines timeout period in unit of time tick.
/  The FF_SYNC_t defines O/S dependent sync object type. e.g. HANDLE, ID, OS_EVENT*,
/  SemaphoreHandle_t and etc. A header file for O/S definitions needs to be
/  included somewhere in the scope of ff.h. */

