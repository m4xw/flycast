#pragma once

#include <stddef.h>
#include "build.h"
#include "log/Log.h"

#ifdef HAVE_LIBNX
#include <strings.h>
#endif

#ifdef _MSC_VER
#define DECL_ALIGN(x) __declspec(align(x))
#else
#ifndef __forceinline
#define __forceinline inline
#endif
#define DECL_ALIGN(x) __attribute__((aligned(x)))
#endif


#if HOST_CPU == CPU_X86
#ifdef _MSC_VER
#define DYNACALL  __fastcall
#else
//android defines fastcall as regparm(3), it doesn't work for us
#undef fastcall
#define DYNACALL __attribute__((fastcall))
#endif
#else
#define DYNACALL
#endif

#ifdef _MSC_VER
#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#ifdef _CRT_SECURE_NO_DEPRECATE
#undef _CRT_SECURE_NO_DEPRECATE
#endif

#define _CRT_SECURE_NO_DEPRECATE 
//unnamed struncts/unions
#pragma warning( disable : 4201)

//unused parameters
#pragma warning( disable : 4100)

//SHUT UP M$ COMPILER !@#!@$#
#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#undef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#ifdef _CRT_SECURE_NO_DEPRECATE
#undef _CRT_SECURE_NO_DEPRECATE
#endif
#define _CRT_SECURE_NO_DEPRECATE

//Do not complain when i use enum::member
#pragma warning( disable : 4482)

//unnamed struncts/unions
#pragma warning( disable : 4201)

//unused parameters
#pragma warning( disable : 4100)
#endif

#include <stdint.h>
#include <stddef.h>

//basic types
typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef ptrdiff_t snat;
typedef size_t unat;

#ifdef _M_X64
#undef X86
#define X64
#endif

typedef char wchar;

#ifndef CDECL
#define CDECL __cdecl
#endif


enum page_access
{
   ACC_NONE = 0,
   ACC_READONLY,
   ACC_READWRITE,
   ACC_READWRITEEXEC
};

typedef float f32;
typedef double f64;

//intc function pointer and enums
enum HollyInterruptType
{
	holly_nrm = 0x0000,
	holly_ext = 0x0100,
	holly_err = 0x0200
};

enum HollyInterruptID
{		
		// asic9a /sh4 external holly normal [internal]
		holly_RENDER_DONE_vd = holly_nrm | 0,	//bit 0 = End of Render interrupt : Video
		holly_RENDER_DONE_isp = holly_nrm | 1,	//bit 1 = End of Render interrupt : ISP
		holly_RENDER_DONE = holly_nrm | 2,		//bit 2 = End of Render interrupt : TSP

		holly_SCANINT1 = holly_nrm | 3,			//bit 3 = V Blank-in interrupt
		holly_SCANINT2 = holly_nrm | 4,			//bit 4 = V Blank-out interrupt
		holly_HBLank = holly_nrm | 5,			//bit 5 = H Blank-in interrupt
												
		holly_YUV_DMA = holly_nrm | 6,			//bit 6 = End of Transferring interrupt : YUV
		holly_OPAQUE = holly_nrm | 7,			//bit 7 = End of Transferring interrupt : Opaque List
		holly_OPAQUEMOD = holly_nrm | 8,		//bit 8 = End of Transferring interrupt : Opaque Modifier Volume List
		
		holly_TRANS = holly_nrm | 9,			//bit 9 = End of Transferring interrupt : Translucent List
		holly_TRANSMOD = holly_nrm | 10,		//bit 10 = End of Transferring interrupt : Translucent Modifier Volume List
		holly_PVR_DMA = holly_nrm | 11,			//bit 11 = End of DMA interrupt : PVR-DMA
		holly_MAPLE_DMA = holly_nrm | 12,		//bit 12 = End of DMA interrupt : Maple-DMA

		holly_MAPLE_VBOI = holly_nrm | 13,		//bit 13 = Maple V blank over interrupt
		holly_GDROM_DMA = holly_nrm | 14,		//bit 14 = End of DMA interrupt : GD-DMA
		holly_SPU_DMA = holly_nrm | 15,			//bit 15 = End of DMA interrupt : AICA-DMA
		
		holly_EXT_DMA1 = holly_nrm | 16,		//bit 16 = End of DMA interrupt : Ext-DMA1(External 1)
		holly_EXT_DMA2 = holly_nrm | 17,		//bit 17 = End of DMA interrupt : Ext-DMA2(External 2)
		holly_DEV_DMA = holly_nrm | 18,			//bit 18 = End of DMA interrupt : Dev-DMA(Development tool DMA)
		
		holly_CH2_DMA = holly_nrm | 19,			//bit 19 = End of DMA interrupt : ch2-DMA 
		holly_PVR_SortDMA = holly_nrm | 20,		//bit 20 = End of DMA interrupt : Sort-DMA (Transferring for alpha sorting)
		holly_PUNCHTHRU = holly_nrm | 21,		//bit 21 = End of Transferring interrupt : Punch Through List

		// asic9c/sh4 external holly external [EXTERNAL]
		holly_GDROM_CMD = holly_ext | 0x00,	//bit 0 = GD-ROM interrupt
		holly_SPU_IRQ = holly_ext | 0x01,	//bit 1 = AICA interrupt
		holly_EXP_8BIT = holly_ext | 0x02,	//bit 2 = Modem interrupt
		holly_EXP_PCI = holly_ext | 0x03,	//bit 3 = External Device interrupt

		// asic9b/sh4 external holly err only error [error]
		//missing quite a few ehh ?
		//bit 0 = RENDER : ISP out of Cache(Buffer over flow)
		//bit 1 = RENDER : Hazard Processing of Strip Buffer
		holly_PRIM_NOMEM = holly_err | 0x02,	//bit 2 = TA : ISP/TSP Parameter Overflow
		holly_MATR_NOMEM = holly_err | 0x03		//bit 3 = TA : Object List Pointer Overflow
		//bit 4 = TA : Illegal Parameter
		//bit 5 = TA : FIFO Overflow
		//bit 6 = PVRIF : Illegal Address set
		//bit 7 = PVRIF : DMA over run
		//bit 8 = MAPLE : Illegal Address set
		//bit 9 = MAPLE : DMA over run
		//bit 10 = MAPLE : Write FIFO over flow
		//bit 11 = MAPLE : Illegal command
		//bit 12 = G1 : Illegal Address set
		//bit 13 = G1 : GD-DMA over run
		//bit 14 = G1 : ROM/FLASH access at GD-DMA
		//bit 15 = G2 : AICA-DMA Illegal Address set
		//bit 16 = G2 : Ext-DMA1 Illegal Address set
		//bit 17 = G2 : Ext-DMA2 Illegal Address set
		//bit 18 = G2 : Dev-DMA Illegal Address set
		//bit 19 = G2 : AICA-DMA over run
		//bit 20 = G2 : Ext-DMA1 over run
		//bit 21 = G2 : Ext-DMA2 over run
		//bit 22 = G2 : Dev-DMA over run
		//bit 23 = G2 : AICA-DMA Time out
		//bit 24 = G2 : Ext-DMA1 Time out
		//bit 25 = G2 : Ext-DMA2 Time out
		//bit 26 = G2 : Dev-DMA Time out 
		//bit 27 = G2 : Time out in CPU accessing 	
};



struct vram_block
{
	u32 start;
	u32 end;
	u32 len;
	u32 type;
 
	void* userdata;
};

extern unsigned FLASH_SIZE;
extern unsigned BBSRAM_SIZE;
extern unsigned BIOS_SIZE;
extern unsigned RAM_SIZE;
extern unsigned RAM_MASK;
extern unsigned VRAM_SIZE;
extern unsigned ARAM_SIZE;
extern unsigned VRAM_MASK;
extern unsigned ARAM_MASK;

#define GD_CLOCK 33868800				//GDROM XTAL -- 768fs

#define AICA_CORE_CLOCK (GD_CLOCK*4/3)		//[45158400]  GD->PLL 3:4 -> AICA CORE	 -- 1024fs
#define ADAC_CLOCK (AICA_CORE_CLOCK/2)		//[11289600]  44100*256, AICA CORE -> PLL 4:1 -> ADAC -- 256fs
#define AICA_ARM_CLOCK (AICA_CORE_CLOCK/2)	//[22579200]  AICA CORE -> PLL 2:1 -> ARM
#define AICA_SDRAM_CLOCK (GD_CLOCK*2)		//[67737600]  GD-> PLL 2 -> SDRAM
#define SH4_MAIN_CLOCK (200*1000*1000)		//[200000000] XTal(13.5) -> PLL (33.3) -> PLL 1:6 (200)
#define SH4_RAM_CLOCK (100*1000*1000)		//[100000000] XTal(13.5) -> PLL (33.3) -> PLL 1:3 (100)	, also suplied to HOLLY chip
#define G2_BUS_CLOCK (25*1000*1000)			//[25000000]  from Holly, from SH4_RAM_CLOCK w/ 2 2:1 plls


////////////////////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************
//*********************** PowerVR **********************
//******************************************************
 
void libCore_vramlock_Unlock_block  (vram_block* block);
void libCore_vramlock_Unlock_block_wb  (vram_block* block);
vram_block* libCore_vramlock_Lock(u32 start_offset,u32 end_offset,void* userdata);



//******************************************************
//************************ GDRom ***********************
//******************************************************
enum DiscType
{
	CdDA=0x00,
	CdRom=0x10,
	CdRom_XA=0x20,
	CdRom_Extra=0x30,
	CdRom_CDI=0x40,
	GdRom=0x80,		

	NoDisk=0x1,			//These are a bit hacky .. but work for now ...
	Open=0x2,			//tray is open :)
	Busy=0x3			//busy -> needs to be automatically done by gdhost
};

enum DiskArea
{
	SingleDensity,
	DoubleDensity
};

//******************************************************
//************************ AICA ************************
//******************************************************
void libARM_InterruptChange(u32 bits,u32 L);
void libCore_CDDA_Sector(s16* sector);


//passed on AICA init call


//Ram/Regs are managed by plugin , exept RTC regs (managed by main emu)

//******************************************************
//******************** ARM Sound CPU *******************
//******************************************************

//******************************************************
//****************** Maple devices ******************
//******************************************************


enum MapleDeviceCreationFlags
{
	MDCF_None=0,
	MDCF_Hotplug=1
};

struct maple_subdevice_instance;
struct maple_device_instance;

//buffer_out_len and responce need to be filled w/ proper info by the plugin
//buffer_in must not be edited (its direct pointer on ram)
//output buffer must contain the frame data , the frame header is generated by the maple routing code
//typedef u32 FASTCALL MapleSubDeviceDMAFP(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len);
typedef u32 MapleDeviceDMAFP(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len);

struct maple_subdevice_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleDeviceDMAFP* dma;
	bool connected;
	u32 reserved;	//reserved for the emu , DO NOT EDIT
};
struct maple_device_instance
{
	//port
	u8 port;
	//user data
	void* data;
	//MapleDeviceDMA
	MapleDeviceDMAFP* dma;
	bool connected;

	maple_subdevice_instance subdevices[5];
};


//includes from CRT
#include <stdlib.h>
#include <stdio.h>

//includes from c++rt
#include <vector>
#include <string>
using namespace std;

//used for asm-olny functions
#if defined(X86) && defined(_MSC_VER)
#define naked   __declspec( naked )
#else
#define naked __attribute__((naked))
#endif


#ifndef INLINE
#if DEBUG
//force
#define INLINE
//sugest
#define SINLINE
#else
//force
#define INLINE __forceinline
//sugest
#define SINLINE __inline
#endif
#endif

//no inline -- fixme
#ifdef _WIN32
#define NOINLINE __declspec(noinline)
#define likely(x) x
#define unlikely(x) x
#else
#define NOINLINE __attribute__ ((noinline))
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)       __builtin_expect((x),0)
#endif

//basic includes
#include "stdclass.h"

#ifdef _ANDROID
#include <android/log.h>

#ifdef printf
#undef printf
#endif

#ifdef puts
#undef puts
#endif

#define LOG_TAG   "lr-reicast"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define puts      LOGI
#define printf    LOGI
#define putinf    LOGI
#endif

#ifndef RELEASE
#define EMUERROR(format, ...) printf("Error in %s:%s:%d: " format "\n", __FILE__,__FUNCTION__ ,__LINE__, ##__VA_ARGS__)
#else
#define EMUERROR(...)
#endif
#define EMUERROR2 EMUERROR
#define EMUERROR3 EMUERROR
#define EMUERROR4 EMUERROR

#define _X_x_X_MMU_VER_STR ""

#if DC_PLATFORM==DC_PLATFORM_DREAMCAST
	#define VER_EMUNAME		"reicast"
#elif DC_PLATFORM==DC_PLATFORM_DEV_UNIT
	#define VER_EMUNAME		"reicast-DevKit-SET5.21"
#elif DC_PLATFORM==DC_PLATFORM_NAOMI
	#define VER_EMUNAME		"reicast-Naomi"
#elif DC_PLATFORM==DC_PLATFORM_ATOMISWAVE
	#define VER_EMUNAME		"reicast-AtomisWave"
#else
	#error unknown target platform
#endif


#define VER_FULLNAME	VER_EMUNAME " git" _X_x_X_MMU_VER_STR " (built " __DATE__ "@" __TIME__ ")"
#define VER_SHORTNAME	VER_EMUNAME " git" _X_x_X_MMU_VER_STR


void os_DebugBreak(void);
#define dbgbreak os_DebugBreak()

#ifdef _MSC_VER
#pragma warning( disable : 4127 4996 /*4244*/)
#else
#define stricmp strcasecmp
#endif

#ifndef NO_VERIFY
#define verify(x) if((x)==false){ msgboxf("Verify Failed  : " #x "\n in %s -> %s : %d \n",MBX_ICONERROR,(__FUNCTION__),(__FILE__),__LINE__); dbgbreak;}
#else
#define verify(x) if((x)==false){ msgboxf("Verify Failed  : " #x "\n in %s -> %s : %d \n",MBX_ICONERROR,(__FUNCTION__),(__FILE__),__LINE__); }
#endif

#define die(reason) { msgboxf("Fatal error : %s\n in %s -> %s : %d \n",MBX_ICONERROR,(reason),(__FUNCTION__),(__FILE__),__LINE__); dbgbreak;}


//will be removed sometime soon
//This shit needs to be moved to proper headers
typedef u32  RegReadAddrFP(u32 addr);

typedef void RegWriteAddrFP(u32 addr, u32 data);

/*
	Read Write Const
	D    D     N      -> 0			-> RIO_DATA
	D    F     N      -> WF			-> RIO_WF
	F    F     N      -> RF|WF		-> RIO_FUNC
	D    X     N      -> RO|WF		-> RIO_RO
	F    X     N      -> RF|WF|RO	-> RIO_RO_FUNC
	D    X     Y      -> CONST|RO|WF-> RIO_CONST
	X    F     N      -> RF|WF|WO	-> RIO_WO_FUNC
*/
enum RegStructFlags
{
	//Basic :
	REG_ACCESS_8=1,
	REG_ACCESS_16=2,
	REG_ACCESS_32=4,

	REG_RF=8,
	REG_WF=16,
	REG_RO=32,
	REG_WO=64,
	REG_CONST=128,
	REG_NO_ACCESS=REG_RO|REG_WO,
};

enum RegIO
{
	RIO_DATA = 0,
	RIO_WF = REG_WF,
	RIO_FUNC = REG_WF | REG_RF,
	RIO_RO = REG_RO | REG_WF,
	RIO_RO_FUNC = REG_RO | REG_RF | REG_WF,
	RIO_CONST = REG_RO | REG_WF,
	RIO_WO_FUNC = REG_WF | REG_RF | REG_WO, 
	RIO_NO_ACCESS = REG_WF | REG_RF | REG_NO_ACCESS
};

struct RegisterStruct
{
	union
	{
		u32 data32;					//stores data of reg variable [if used] 32b
		u16 data16;					//stores data of reg variable [if used] 16b
		u8  data8;					//stores data of reg variable [if used]	8b

		RegReadAddrFP* readFunctionAddr;	//stored pointer to reg read function
	};

	RegWriteAddrFP* writeFunctionAddr;

	u32 flags;					//Access flags !

	void reset()
	{
		if (!(flags & (REG_RO | REG_RF)))
			data32 = 0;
	}
};


struct settings_t
{
   unsigned System;

	struct {
		bool UseReios;
	} bios;

	struct {
		string ElfFile;
	} reios;

	struct
	{
		bool UseMipmaps;
		bool WideScreen;
      bool RenderToTexture;
      bool RenderToTextureBuffer;
      int RenderToTextureUpscale;
      bool TranslucentPolygonDepthMask;
      bool ModifierVolumes;
		bool Clipping;
      int TextureUpscale;
		int MaxFilteredTextureSize;
      bool AutoExtraDepthScale;
      f32 ExtraDepthScale;
		bool ThreadedRendering;
		bool CustomTextures;
		bool DumpTextures;
		bool DelayFrameSwapping; // Delay swapping frame until FB_R_SOF matches FB_W_SOF
		bool WidescreenGameHacks;
	} rend;

	struct
	{
		bool Enable;
      unsigned Type;
		bool idleskip;
		bool unstable_opt;
		bool disable_nvmem;
		bool disable_vmem32;
      bool DisableDivMatching;
      bool AutoDivMatching;
	} dynarec;
	
	struct
	{
		u32 run_counts;
	} profile;

	struct
	{
	   u32 cable;			// 0 -> VGA, 1 -> VGA, 2 -> RGB, 3 -> TV
	   u32 RTC;
	   u32 region;			// 0 -> JP, 1 -> USA, 2 -> EU, 3 -> default
	   u32 broadcast;		// 0 -> NTSC, 1 -> PAL, 2 -> PAL/M, 3 -> PAL/N, 4 -> default
	   u32 language;		// 0 -> JP, 1 -> EN, 2 -> DE, 3 -> FR, 4 -> SP, 5 -> IT, 6 -> default
	   bool FullMMU;
	   bool ForceWinCE;
	} dreamcast;

	struct
   {
      u32 LimitFPS;		//0 -> no , (1) -> limit
      u32 CDDAMute;
      u32 DSPEnabled;		//0 -> no, 1 -> yes
      u32 NoBatch;
      u32 NoSound;        //0 ->sound, 1 -> no sound
   } aica;

	struct
	{
		bool PatchRegion;
		bool LoadDefaultImage;
		char DefaultImage[512];
	} imgread;

	struct
	{
		struct
		{
			u32 ResolutionMode;
			u32 VSync;
		} Video;

		struct 
		{
			u32 MultiSampleCount;
			u32 MultiSampleQuality;
			u32 AspectRatioMode;
		} Enhancements;

		struct
		{
			u32 PaletteMode;
			u32 AlphaSortMode;
			bool ModVol;
			u32 ZBufferMode;
			u32 TexCacheMode;
         f32 zMin;
         f32 zMax;
		} Emulation;

		struct
		{
			u32 ShowFPS;
			u32 ShowStats;
		} OSD;

		u32 ta_skip;
		u32 subdivide_transp;
		u32 rend;
		
		u32 MaxThreads;
		u32 SynchronousRendering;
	} pvr;

   unsigned UpdateMode;
   unsigned UpdateModeForced;

	struct {
		bool SerialConsole;
	} debug;

	struct {
		bool OpenGlChecks;
	} validate;

	struct {
		int JammaSetup;
	} mapping;
};

extern settings_t settings;

void LoadSettings(void);
void SaveSettings(void);
u32 GetRTC_now(void);

static inline bool is_s8(u32 v) { return (s8)v==(s32)v; }
static inline bool is_u8(u32 v) { return (u8)v==(s32)v; }
static inline bool is_s16(u32 v) { return (s16)v==(s32)v; }
static inline bool is_u16(u32 v) { return (u16)v==(u32)v; }

#include "hw/sh4/sh4_if.h"

//more to come

extern sh4_if				  sh4_cpu;

//sh4 thread
s32 plugins_Init(char *s, size_t len);
void plugins_Term(void);
void plugins_Reset(bool Manual);

//PVR
s32 libPvr_Init(void);
void libPvr_Reset(bool Manual);
void libPvr_Term(void);

void libPvr_LockedBlockWrite(vram_block* block,u32 addr);	//set to 0 if not used

void* libPvr_GetRenderTarget(void);
void* libPvr_GetRenderSurface(void);

//AICA
s32 libAICA_Init(void);
void libAICA_Reset(bool Manual);
void libAICA_Term(void);


u32  libAICA_ReadReg(u32 addr,u32 size);
void libAICA_WriteReg(u32 addr,u32 data,u32 size);
void libAICA_Update(u32 cycles);				//called every ~1800 cycles, set to 0 if not used


//GDR
s32 libGDR_Init(void);
void libGDR_Reset(bool M);
void libGDR_Term(void);

void libCore_gdrom_disc_change(void);

//IO
void libGDR_ReadSector(u8 * buff,u32 StartSector,u32 SectorCount,u32 secsz);
void libGDR_ReadSubChannel(u8 * buff, u32 format, u32 len);
void libGDR_GetToc(u32* toc,u32 area);
u32 libGDR_GetDiscType(void);
void libGDR_GetSessionInfo(u8* pout,u8 session);
u32 libGDR_GetTrackNumber(u32 sector, u32& elapsed);
bool libGDR_GetTrack(u32 track_num, u32& start_fad, u32& end_fad);

//ExtDev
s32 libExtDevice_Init(void);
void libExtDevice_Reset(bool M);
void libExtDevice_Term(void);

u32  libExtDevice_ReadMem_A0_006(u32 addr,u32 size);
void libExtDevice_WriteMem_A0_006(u32 addr,u32 data,u32 size);

//Area 0 , 0x01000000- 0x01FFFFFF	[Ext. Device]
static u32 libExtDevice_ReadMem_A0_010(u32 addr,u32 size) { return 0; }
static void libExtDevice_WriteMem_A0_010(u32 addr,u32 data,u32 size) { }

//Area 5
static u32 libExtDevice_ReadMem_A5(u32 addr,u32 size){ return 0; }
static void libExtDevice_WriteMem_A5(u32 addr,u32 data,u32 size) { }

//ARM
s32 libARM_Init(void);
void libARM_Reset(bool M);
void libARM_Term(void);

void libARM_SetResetState(u32 State);
void libARM_Update(u32 cycles);

#define 	ReadMemArrRet(arr,addr,sz)				\
			{if (sz==1)								\
				return arr[addr];					\
			else if (sz==2)							\
				return *(u16*)&arr[addr];			\
			else if (sz==4)							\
				return *(u32*)&arr[addr];}	

#define WriteMemArr(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;}}	

#define WriteMemArrRet(arr,addr,data,sz)				\
			{if(sz==1)								\
				{arr[addr]=(u8)data;return;}				\
			else if (sz==2)							\
				{*(u16*)&arr[addr]=(u16)data;return;}		\
			else if (sz==4)							\
			{*(u32*)&arr[addr]=data;return;}}	


struct OnLoad
{
	typedef void OnLoadFP(void);
	OnLoad(OnLoadFP* fp) { fp(); }
};

void os_DoEvents();
double os_GetSeconds();

#ifdef _MSC_VER
#include <intrin.h>
#endif

u32 static INLINE bitscanrev(u32 v)
{
#ifdef _MSC_VER
	unsigned long rv;
	_BitScanReverse(&rv,v);
	return rv;
#else
	return 31-__builtin_clz(v);
#endif
}

void os_DebugBreak(void);

bool ra_serialize(void *src, unsigned int src_size, void **dest, unsigned int *total_size) ;
bool ra_unserialize(void *src, unsigned int src_size, void **dest, unsigned int *total_size);
bool dc_serialize(void **data, unsigned int *total_size);
bool dc_unserialize(void **data, unsigned int *total_size, size_t actual_data_size);

#define LIBRETRO_S(v) ra_serialize(&(v), sizeof(v), data, total_size)
#define LIBRETRO_US(v) ra_unserialize(&(v), sizeof(v), data, total_size)

#define LIBRETRO_SA(v_arr,num) ra_serialize((v_arr), sizeof((v_arr)[0]) * (num), data, total_size)
#define LIBRETRO_USA(v_arr,num) ra_unserialize((v_arr), sizeof((v_arr)[0]) * (num), data, total_size)


extern "C" void DYNACALL TAWriteSQ(u32 address,u8* sqb) ;

#define SAMPLE_COUNT 512

struct SoundFrame
{
   s16 l;
   s16 r;
};

enum vmu_screen_position_enum
{
	UPPER_LEFT = 0,
	UPPER_RIGHT,
	LOWER_LEFT,
	LOWER_RIGHT
};

#define VMU_SCREEN_WIDTH 48
#define VMU_SCREEN_HEIGHT 32
#define DEFAULT_VMU_PIXEL_ON_R 50
#define DEFAULT_VMU_PIXEL_ON_G 54
#define DEFAULT_VMU_PIXEL_ON_B 93
#define DEFAULT_VMU_PIXEL_OFF_R 135
#define DEFAULT_VMU_PIXEL_OFF_G 161
#define DEFAULT_VMU_PIXEL_OFF_B 134
#define VMU_NUM_COLORS 29

enum VMU_SCREEN_COLORS {
	VMU_DEFAULT_ON,
	VMU_DEFAULT_OFF,
	VMU_BLACK,
	VMU_BLUE,
	VMU_LIGHT_BLUE,
	VMU_GREEN,
	VMU_GREEN_BLUE,
	VMU_GREEN_LIGHT_BLUE,
	VMU_LIGHT_GREEN,
	VMU_LIGHT_GREEN_BLUE,
	VMU_LIGHT_GREEN_LIGHT_BLUE,
	VMU_RED,
	VMU_RED_BLUE,
	VMU_RED_LIGHT_BLUE,
	VMU_RED_GREEN,
	VMU_RED_GREEN_BLUE,
	VMU_RED_GREEN_LIGHT_BLUE,
	VMU_RED_LIGHT_GREEN,
	VMU_RED_LIGHT_GREEN_BLUE,
	VMU_RED_LIGHT_GREEN_LIGHT_BLUE,
	VMU_LIGHT_RED,
	VMU_LIGHT_RED_BLUE,
	VMU_LIGHT_RED_LIGHT_BLUE,
	VMU_LIGHT_RED_GREEN,
	VMU_LIGHT_RED_GREEN_BLUE,
	VMU_LIGHT_RED_GREEN_LIGHT_BLUE,
	VMU_LIGHT_RED_LIGHT_GREEN,
	VMU_LIGHT_RED_LIGHT_GREEN_BLUE,
	VMU_WHITE
};

struct rgb_t {
	u8 r ;
	u8 g ;
	u8 b ;
};

struct vmu_screen_params_t {
	u8* vmu_lcd_screen = NULL ;
	bool vmu_screen_display = false ;
	u8 vmu_pixel_on_R = 128 ;
	u8 vmu_pixel_on_G = 0 ;
	u8 vmu_pixel_on_B = 0 ;
	u8 vmu_pixel_off_R = 0;
	u8 vmu_pixel_off_G = 0;
	u8 vmu_pixel_off_B = 0;
	u8 vmu_screen_size_mult = 2 ;
	u8 vmu_screen_opacity = 0xBF ;
	vmu_screen_position_enum vmu_screen_position = LOWER_RIGHT ;
	bool vmu_screen_needs_update = true ;
};

extern const rgb_t VMU_SCREEN_COLOR_MAP[VMU_NUM_COLORS] ;
extern const char* VMU_SCREEN_COLOR_NAMES[VMU_NUM_COLORS] ;
extern vmu_screen_params_t vmu_screen_params[4] ;

#define LIGHTGUN_CROSSHAIR_SIZE 16

enum LIGHTGUN_COLORS {
	LIGHTGUN_COLOR_OFF,
	LIGHTGUN_COLOR_WHITE,
	LIGHTGUN_COLOR_RED,
	LIGHTGUN_COLOR_GREEN,
	LIGHTGUN_COLOR_BLUE,
	LIGHTGUN_COLORS_COUNT
};

struct lightgun_params_t {
	bool offscreen;
	float x;
	float y;
	bool dirty;
	int colour;
};

extern u8 lightgun_palette[LIGHTGUN_COLORS_COUNT*3];
extern u8 lightgun_img_crosshair[LIGHTGUN_CROSSHAIR_SIZE*LIGHTGUN_CROSSHAIR_SIZE];
extern lightgun_params_t lightgun_params[4] ;

enum serialize_version_enum {
	V1,
	V2,
	V3,
	V4,
	V5,
	V6,
	V7,
	V8
};
