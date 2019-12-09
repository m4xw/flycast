// serialize.cpp : save states

#include "types.h"
#include "hw/aica/dsp.h"
#include "hw/aica/aica.h"
#include "hw/aica/sgc_if.h"
#include "hw/arm7/arm7.h"
#include "hw/holly/sb_mem.h"
#include "hw/flashrom/flashrom.h"
#include "hw/mem/_vmem.h"
#include "hw/gdrom/gdromv3.h"
#include "hw/maple/maple_devs.h"
#include "hw/maple/maple_cfg.h"
#include "hw/pvr/Renderer_if.h"
#include "hw/pvr/ta_structs.h"
#include "hw/sh4/sh4_interrupts.h"
#include "hw/sh4/sh4_sched.h"
#include "hw/sh4/sh4_mmr.h"
#include "hw/sh4/modules/mmu.h"
#include "imgread/common.h"
#include "reios/reios.h"
#include "reios/gdrom_hle.h"
#include <map>
#include <set>
#include "rend/gles/gles.h"
#include "hw/sh4/dyna/blockmanager.h"
#include "hw/sh4/dyna/ngen.h"
#include "hw/naomi/naomi_cart.h"
#include "hw/naomi/naomi.h"

#define LIBRETRO_SKIP(size) do { *(u8**)data += (size); *total_size += (size); } while (false)

/*
 * search for "maybe" to find items that were left out that may be needed
 */

//./core/hw/arm7/arm_mem.cpp
extern bool aica_interr;
extern u32 aica_reg_L;
extern bool e68k_out;
extern u32 e68k_reg_L;
extern u32 e68k_reg_M;



//./core/hw/arm7/arm7.cpp
extern DECL_ALIGN(8) reg_pair arm_Reg[RN_ARM_REG_COUNT];
extern bool armIrqEnable;
extern bool armFiqEnable;
extern int armMode;
extern bool Arm7Enabled;
extern u8 cpuBitsSet[256];
/*
	if AREC dynarec enabled:
	vector<ArmDPOP> ops;
	u8* icPtr;
	u8* ICache;
	u8 ARM7_TCB[ICacheSize+4096];
	void* EntryPoints[8*1024*1024/4];
	map<u32,u32> renamed_regs;
	u32 rename_reg_base;
	u32 nfb,ffb,bfb,mfb;
	static x86_block* x86e;
	x86_Label* end_lbl;
	u8* ARM::emit_opt=0;
	eReg ARM::reg_addr;
	eReg ARM::reg_dst;
	s32 ARM::imma;
*/




//./core/hw/aica/dsp.o
extern DECL_ALIGN(4096) dsp_t dsp;
//recheck dsp.cpp if FEAT_DSPREC == DYNAREC_JIT




//./core/hw/aica/aica.o
//these are all just pointers into aica_reg
//extern CommonData_struct* CommonData;
//extern DSPData_struct* DSPData;
//extern InterruptInfo* MCIEB;
//extern InterruptInfo* MCIPD;
//extern InterruptInfo* MCIRE;
//extern InterruptInfo* SCIEB;
//extern InterruptInfo* SCIPD;
//extern InterruptInfo* SCIRE;

extern AicaTimer timers[3];



//./core/hw/aica/aica_if.o
extern VArray2 aica_ram;
extern u32 VREG;//video reg =P
extern u32 ARMRST;//arm reset reg
extern u32 rtc_EN;
extern int dma_sched_id;



//./core/hw/aica/aica_mem.o
extern u8 aica_reg[0x8000];



//./core/hw/aica/sgc_if.o
struct ChannelEx;
#define AicaChannel ChannelEx
//this is just a pointer to aica_reg
//extern DSP_OUT_VOL_REG* dsp_out_vol;
//not needed - one-time init
//void(* 	STREAM_STEP_LUT [5][2][2])(ChannelEx *ch)
//void(* 	STREAM_INITAL_STEP_LUT [5])(ChannelEx *ch)
//void(* 	AEG_STEP_LUT [4])(ChannelEx *ch)
//void(* 	FEG_STEP_LUT [4])(ChannelEx *ch)
//void(* 	ALFOWS_CALC [4])(ChannelEx *ch)
//void(* 	PLFOWS_CALC [4])(ChannelEx *ch)

//special handling
//extern AicaChannel AicaChannel::Chans[64];
#define Chans AicaChannel::Chans
#define CDDA_SIZE  (2352/2)
extern s16 cdda_sector[CDDA_SIZE];
extern u32 cdda_index;



//./core/hw/holly/sb.o
extern Array<RegisterStruct> sb_regs;
extern u32 SB_ISTNRM;
extern u32 SB_FFST_rc;
extern u32 SB_FFST;



//./core/hw/holly/sb_mem.o
//unused
//static HollyInterruptID dmatmp1;
//static HollyInterruptID dmatmp2;
//static HollyInterruptID OldDmaId;

//this is one-time init, no updates - don't need to serialize
//extern RomChip sys_rom;
extern SRamChip sys_nvmem_sram;
extern DCFlashChip sys_nvmem_flash;
//this is one-time init, no updates - don't need to serialize
//extern _vmem_handler area0_handler;

//./core/hw/gdrom/gdromv3.o
extern int gdrom_sched;
extern signed int sns_asc;
extern signed int sns_ascq;
extern signed int sns_key;
extern packet_cmd_t packet_cmd;
extern u32 set_mode_offset;
extern read_params_t read_params ;
extern packet_cmd_t packet_cmd;
//Buffer for sector reads [dma]
extern read_buff_t read_buff ;
//pio buffer
extern pio_buff_t pio_buff ;
extern u32 set_mode_offset;
extern ata_cmd_t ata_cmd ;
extern cdda_t cdda ;
extern gd_states gd_state;
extern DiscType gd_disk_type;
extern u32 data_write_mode;
//Registers
extern u32 DriveSel;
extern GD_ErrRegT Error;
extern GD_InterruptReasonT IntReason;
extern GD_FeaturesT Features;
extern GD_SecCountT SecCount;
extern GD_SecNumbT SecNumber;
extern GD_StatusT GDStatus;
extern ByteCount_t ByteCount ;
extern int GDROM_TICK;





//./core/hw/maple/maple_devs.o
extern char EEPROM[0x100];
extern bool EEPROM_loaded;

//./core/hw/maple/maple_if.o
//needs special handler
extern maple_device* MapleDevices[4][6];
//one time set
extern int maple_sched;
//incremented but never read
//extern u32 dmacount;
extern bool maple_ddt_pending_reset;

#ifdef HAVE_MODEM
//./core/hw/modem/modem.cpp
extern int modem_sched;
#endif


//./core/hw/pvr/Renderer_if.o
//only written - not read
//extern u32 VertexCount;
extern u32 FrameCount;
//one-time init
//extern Renderer* renderer;
//these are just mutexes used during rendering
//extern cResetEvent rs;
//extern cResetEvent re;
//these max_?? through ovrn are written and not read
//extern int max_idx;
//extern int max_mvo;
//extern int max_op;
//extern int max_pt;
//extern int max_tr;
//extern int max_vtx;
//extern int max_modt;
//extern int ovrn;
//seems safe to omit this - gets refreshed every frame from a pool
//extern TA_context* _pvrrc;
//just a flag indiating the rendering thread is running
//extern int rend_en ;
//the renderer thread - one time set
//extern cThread rthd;
extern bool pend_rend;

//these will all get cleared out after a few frames - no need to serialize
//static bool render_called = false;
//u32 fb1_watch_addr_start;
//u32 fb1_watch_addr_end;
//u32 fb2_watch_addr_start;
//u32 fb2_watch_addr_end;
//bool fb_dirty;




//./core/hw/pvr/pvr_mem.o
extern u32 YUV_tempdata[512/4];//512 bytes
extern u32 YUV_dest;
extern u32 YUV_blockcount;
extern u32 YUV_x_curr;
extern u32 YUV_y_curr;
extern u32 YUV_x_size;
extern u32 YUV_y_size;




//./core/hw/pvr/pvr_regs.o
extern bool fog_needs_update;
extern u8 pvr_regs[pvr_RegSize];




//./core/hw/pvr/spg.o
extern u32 in_vblank;
extern u32 clc_pvr_scanline;
extern int render_end_sched;
extern int vblank_sched;




//./core/hw/pvr/ta.o
extern u32 ta_type_lut[256];
extern u8 ta_fsm[2049];	//[2048] stores the current state
extern u32 ta_fsm_cl;




//./core/hw/pvr/ta_ctx.o
//these frameskipping don't need to be saved
//extern int frameskip;
//extern bool FrameSkipping;		// global switch to enable/disable frameskip
//maybe need these - but hopefully not
//extern TA_context* ta_ctx;
//extern tad_context ta_tad;
//extern TA_context*  vd_ctx;
//extern rend_context vd_rc;
//extern cMutex mtx_rqueue;
//extern TA_context* rqueue;
//extern cResetEvent frame_finished;
//extern cMutex mtx_pool;
//extern vector<TA_context*> ctx_pool;
//extern vector<TA_context*> ctx_list;
//end maybe



//./core/hw/pvr/ta_vtx.o
extern bool pal_needs_update;
extern u32 _pal_rev_256[4];
extern u32 _pal_rev_16[64];
extern u32 pal_rev_256[4];
extern u32 pal_rev_16[64];
//extern u32 palette_ram[1024];
extern u32 decoded_colors[3][65536];
extern u32 tileclip_val;
extern u8 f32_su8_tbl[65536];
//written but never read
//extern ModTriangle* lmr;
//never changed
//extern PolyParam nullPP;
//maybe
//extern PolyParam* CurrentPP;
//maybe
//extern List<PolyParam>* CurrentPPlist;
//TA state vars
extern DECL_ALIGN(4) u8 FaceBaseColor[4];
extern DECL_ALIGN(4) u8 FaceOffsColor[4];
extern DECL_ALIGN(4) u8 FaceBaseColor1[4];
extern DECL_ALIGN(4) u8 FaceOffsColor1[4];
extern DECL_ALIGN(4) u32 SFaceBaseColor;
extern DECL_ALIGN(4) u32 SFaceOffsColor;
//maybe
//extern TaListFP* TaCmd;
//maybe
//extern u32 CurrentList;
//maybe
//extern TaListFP* VertexDataFP;
//written but never read
//extern bool ListIsFinished[5];
//maybe ; need special handler
//FifoSplitter<0> TAFifo0;
//counter for frameskipping - doesn't need to be saved
//extern int ta_parse_cnt;




//./core/rend/TexCache.o
//maybe
//extern u8* vq_codebook;
extern u32 palette_index;
extern bool KillTex;
extern u32 palette16_ram[1024];
extern u32 palette32_ram[1024];
extern u32 detwiddle[2][8][1024];
//maybe
//extern vector<vram_block*> VramLocks[/*VRAM_SIZE*/(16*1024*1024)/PAGE_SIZE];
//maybe - probably not - just a locking mechanism
//extern cMutex vramlist_lock;
extern VArray2 vram;




//./core/hw/sh4/sh4_mmr.o
extern Array<u8> OnChipRAM;
extern Array<RegisterStruct> CCN;  //CCN  : 14 registers
extern Array<RegisterStruct> UBC;   //UBC  : 9 registers
extern Array<RegisterStruct> BSC;  //BSC  : 18 registers
extern Array<RegisterStruct> DMAC; //DMAC : 17 registers
extern Array<RegisterStruct> CPG;   //CPG  : 5 registers
extern Array<RegisterStruct> RTC;  //RTC  : 16 registers
extern Array<RegisterStruct> INTC;  //INTC : 4 registers
extern Array<RegisterStruct> TMU;  //TMU  : 12 registers
extern Array<RegisterStruct> SCI;   //SCI  : 8 registers
extern Array<RegisterStruct> SCIF; //SCIF : 10 registers




//./core/hw/sh4/sh4_mem.o
extern VArray2 mem_b;
//one-time init
//extern _vmem_handler area1_32b;
//one-time init
//extern _vmem_handler area5_handler;




//./core/hw/sh4/sh4_interrupts.o
extern u16 IRLPriority;
//one-time init
//extern InterptSourceList_Entry InterruptSourceList[28];
extern DECL_ALIGN(64) u16 InterruptEnvId[32];
extern DECL_ALIGN(64) u32 InterruptBit[32];
extern DECL_ALIGN(64) u32 InterruptLevelBit[16];
extern u32 interrupt_vpend; // Vector of pending interrupts
extern u32 interrupt_vmask; // Vector of masked interrupts             (-1 inhibits all interrupts)
extern u32 decoded_srimask; // Vector of interrupts allowed by SR.IMSK (-1 inhibits all interrupts)




//./core/hw/sh4/sh4_core_regs.o
extern Sh4RCB* p_sh4rcb;
//just method pointers
//extern sh4_if  sh4_cpu;
//one-time set
//extern u8* sh4_dyna_rcb;
extern u32 old_rm;
extern u32 old_dn;




//./core/hw/sh4/sh4_sched.o
extern u64 sh4_sched_ffb;
extern vector<sched_list> sch_list;
//extern int sh4_sched_next_id;




//./core/hw/sh4/interpr/sh4_interpreter.o
extern int aica_sched;
extern int rtc_sched;




//./core/hw/sh4/modules/serial.o
extern SCIF_SCFSR2_type SCIF_SCFSR2;
extern u8 SCIF_SCFRDR2;
extern SCIF_SCFDR2_type SCIF_SCFDR2;




//./core/hw/sh4/modules/bsc.o
extern BSC_PDTRA_type BSC_PDTRA;




//./core/hw/sh4/modules/tmu.o
extern u32 tmu_shift[3];
extern u32 tmu_mask[3];
extern u64 tmu_mask64[3];
extern u32 old_mode[3];
extern int tmu_sched[3];
extern u32 tmu_ch_base[3];
extern u64 tmu_ch_base64[3];




//./core/hw/sh4/modules/ccn.o
extern u32 CCN_QACR_TR[2];




//./core/hw/sh4/modules/mmu.o
extern TLB_Entry UTLB[64];
extern TLB_Entry ITLB[4];
#if defined(NO_MMU)
extern u32 sq_remap[64];
#else
extern u32 ITLB_LRU_USE[64];
#endif



//./core/imgread/common.o
extern u32 NullDriveDiscType;
//maybe - seems to all be one-time inits ;    needs special handler
//extern Disc* disc;
extern u8 q_subchannel[96];




//./core/nullDC.o
extern unsigned FLASH_SIZE;
extern unsigned BBSRAM_SIZE;
extern unsigned BIOS_SIZE;
extern unsigned RAM_SIZE;
extern unsigned ARAM_SIZE;
extern unsigned VRAM_SIZE;
extern unsigned RAM_MASK;
extern unsigned ARAM_MASK;
extern unsigned VRAM_MASK;
//settings can be dynamic
//extern settings_t settings;




//./core/reios/reios.o
//never used
//extern u8* biosrom;
//one time init
//extern u8* flashrom;
//one time init
//extern u32 base_fad ;
//one time init
//extern bool descrambl;
//one time init
//extern bool bootfile_inited;
//one time init
//extern map<u32, hook_fp*> hooks;
//one time init
//extern map<hook_fp*, u32> hooks_rev;




//./core/reios/gdrom_hle.o
//never used in any meaningful way
//extern u32 SecMode[4];




//./core/reios/descrambl.o
//random seeds can be...random
//extern unsigned int seed;



//./core/rend/gles/gles.o
//maybe
//extern GLCache glcache;
//maybe
//extern gl_ctx gl;
//maybe
//extern struct ShaderUniforms_t ShaderUniforms;
//maybe
//extern u32 gcflip;
//maybe
//extern float fb_scale_x;
//extern float fb_scale_y;
//extern float scale_x;
//extern float scale_y;
//extern int screen_width;
//extern int screen_height;
//extern GLuint fogTextureId;
//end maybe



//./core/rend/gles/gldraw.o
//maybe
//extern PipelineShader* CurrentShader;
//written, but never used
//extern Vertex* vtx_sort_base;
//maybe
//extern vector<SortTrigDrawParam>	pidx_sort;




//./core/rend/gles/gltex.o
//maybe ; special handler
//extern map<u64,TextureCacheData> TexCache;
//maybe
//extern FBT fb_rtt;
//not used
//static int TexCacheLookups;
//static int TexCacheHits;
//static float LastTexCacheStats;
//maybe should get reset naturally if needed
//GLuint fbTextureId;




//./core/hw/naomi/naomi.o
extern u32 naomi_updates;
extern u32 GSerialBuffer;
extern u32 BSerialBuffer;
extern int GBufPos;
extern int BBufPos;
extern int GState;
extern int BState;
extern int GOldClk;
extern int BOldClk;
extern int BControl;
extern int BCmd;
extern int BLastCmd;
extern int GControl;
extern int GCmd;
extern int GLastCmd;
extern int SerStep;
extern int SerStep2;
extern unsigned char BSerial[];
extern unsigned char GSerial[];

extern bool NaomiDataRead;
extern u32 NAOMI_ROM_OFFSETH;
extern u32 NAOMI_ROM_OFFSETL;
extern u32 NAOMI_ROM_DATA;
extern u32 NAOMI_DMA_OFFSETH;
extern u32 NAOMI_DMA_OFFSETL;
extern u32 NAOMI_DMA_COUNT;
extern u32 NAOMI_BOARDID_WRITE;
extern u32 NAOMI_BOARDID_READ;
extern u32 NAOMI_COMM_OFFSET;
extern u32 NAOMI_COMM_DATA;




//./core/hw/naomi/naomi_cart.o
//all one-time loads
//u8* RomPtr;
//u32 RomSize;
//fd_t*	RomCacheMap;
//u32		RomCacheMapCount;




//./core/rec-x64/rec_x64.o
//maybe need special handler
//extern BlockCompilerx64 *compilerx64_data;




//./core/rec.o
extern int cycle_counter;
//extern int idxnxx;




//./core/hw/sh4/dyna/decoder.o
//temp storage only
//extern RuntimeBlockInfo* blk;
extern state_t state;
extern Sh4RegType div_som_reg1;
extern Sh4RegType div_som_reg2;
extern Sh4RegType div_som_reg3;




//./core/hw/sh4/dyna/driver.o
extern u8 SH4_TCB[CODE_SIZE+4096];
//one time ptr set
//extern u8* CodeCache;
extern u32 LastAddr;
extern u32 LastAddr_min;
//temp storage only
//extern u32* emit_ptr;
extern char block_hash[1024];




//./core/hw/sh4/dyna/blockmanager.o
//cleared but never read
//extern bm_List blocks_page[/*BLOCKS_IN_PAGE_LIST_COUNT*/(32*1024*1024)/4096];
//maybe - the next three seem to be list of precompiled blocks of code - but if not found will populate
//extern bm_List all_blocks;
//extern bm_List del_blocks;
//extern blkmap_t blkmap;
//these two are never referenced
//extern u32 bm_gc_luc;
//extern u32 bm_gcf_luc;
//data is never written to this
//extern u32 PAGE_STATE[(32*1024*1024)/*RAM_SIZE*//32];
//never read
//extern u32 total_saved;
//just printf output
//extern bool print_stats;




//./core/libretro/libretro.o
//extern u32 kcode[4];
//extern u8 rt[4];
//extern u8 lt[4];
//extern u32 vks[4];
//extern s8 joyx[4];
//extern s8 joyy[4];



//./core/libretro-common/glsm/glsm.o
//one time setup
//extern GLint glsm_max_textures;
//one time setup
//extern struct retro_hw_render_callback hw_render;
//maybe
//extern struct gl_cached_state gl_state;


bool ra_serialize(void *src, unsigned int src_size, void **dest, unsigned int *total_size)
{
	if ( *dest != NULL )
	{
		memcpy(*dest, src, src_size) ;
		*dest = ((unsigned char*)*dest) + src_size ;
	}

	*total_size += src_size ;
	return true ;
}

bool ra_unserialize(void *src, unsigned int src_size, void **dest, unsigned int *total_size)
{
	if ( *dest != NULL )
	{
		memcpy(src, *dest, src_size) ;
		*dest = ((unsigned char*)*dest) + src_size ;
	}

	*total_size += src_size ;
	return true ;
}

bool register_serialize(Array<RegisterStruct>& regs,void **data, unsigned int *total_size )
{
	int i = 0 ;

	for ( i = 0 ; i < regs.Size ; i++ )
	{
		LIBRETRO_S(regs.data[i].flags) ;
		LIBRETRO_S(regs.data[i].data32) ;
	}

	return true ;
}

bool register_unserialize(Array<RegisterStruct>& regs,void **data, unsigned int *total_size, u32 force_size = 0)
{
	int i = 0 ;
	u32 dummy = 0 ;

	if (force_size == 0)
	   force_size = regs.Size;
	for ( i = 0 ; i < force_size ; i++ )
	{
		LIBRETRO_US(regs.data[i].flags) ;
		if ( ! (regs.data[i].flags & REG_RF) )
			LIBRETRO_US(regs.data[i].data32) ;
		else
			LIBRETRO_US(dummy) ;
	}
	return true ;
}

bool dc_serialize(void **data, unsigned int *total_size)
{
	int i = 0;
	int j = 0;
	serialize_version_enum version = V8;

	*total_size = 0 ;

	//dc not initialized yet
	if ( p_sh4rcb == NULL )
		return false ;

	LIBRETRO_S(version) ;
	LIBRETRO_S(aica_interr) ;
	LIBRETRO_S(aica_reg_L) ;
	LIBRETRO_S(e68k_out) ;
	LIBRETRO_S(e68k_reg_L) ;
	LIBRETRO_S(e68k_reg_M) ;

	LIBRETRO_SA(arm_Reg,RN_ARM_REG_COUNT);
	LIBRETRO_S(armIrqEnable);
	LIBRETRO_S(armFiqEnable);
	LIBRETRO_S(armMode);
	LIBRETRO_S(Arm7Enabled);
	LIBRETRO_SA(cpuBitsSet,256);
	bool dummybool;
	LIBRETRO_S(dummybool); // intState
	LIBRETRO_S(dummybool); // stopState
	LIBRETRO_S(dummybool); // holdState

	LIBRETRO_S(dsp);

	for ( i = 0 ; i < 3 ; i++)
	{
		LIBRETRO_S(timers[i].c_step);
		LIBRETRO_S(timers[i].m_step);
	}


	LIBRETRO_SA(aica_ram.data,aica_ram.size) ;
	LIBRETRO_S(VREG);
	LIBRETRO_S(ARMRST);
	LIBRETRO_S(rtc_EN);

	LIBRETRO_SA(aica_reg,0x8000);

	channel_serialize(data, total_size) ;

	LIBRETRO_SA(cdda_sector,CDDA_SIZE);
	LIBRETRO_S(cdda_index);
	for (int i = 0; i < 64; i++)
		LIBRETRO_S(i);	// mxlr
	LIBRETRO_S(i);		// samples_gen


	register_serialize(sb_regs, data, total_size) ;
	LIBRETRO_S(SB_ISTNRM);
	LIBRETRO_S(SB_FFST_rc);
	LIBRETRO_S(SB_FFST);



	//this is one-time init, no updates - don't need to serialize
	//extern RomChip sys_rom;

	LIBRETRO_S(sys_nvmem_sram.size);
	LIBRETRO_S(sys_nvmem_sram.mask);
	LIBRETRO_SA(sys_nvmem_sram.data,sys_nvmem_sram.size);

	LIBRETRO_S(sys_nvmem_flash.size);
	LIBRETRO_S(sys_nvmem_flash.mask);
	LIBRETRO_S(sys_nvmem_flash.state);
	LIBRETRO_SA(sys_nvmem_flash.data,sys_nvmem_flash.size);

	//this is one-time init, no updates - don't need to serialize
	//extern _vmem_handler area0_handler;




	LIBRETRO_S(GD_HardwareInfo) ;



	LIBRETRO_S(sns_asc);
	LIBRETRO_S(sns_ascq);
	LIBRETRO_S(sns_key);

	LIBRETRO_S(packet_cmd);
	LIBRETRO_S(set_mode_offset);
	LIBRETRO_S(read_params);
	LIBRETRO_S(packet_cmd);
	LIBRETRO_S(read_buff);
	LIBRETRO_S(pio_buff);
	LIBRETRO_S(set_mode_offset);
	LIBRETRO_S(ata_cmd);
	LIBRETRO_S(cdda);
	LIBRETRO_S(gd_state);
	LIBRETRO_S(gd_disk_type);
	LIBRETRO_S(data_write_mode);
	LIBRETRO_S(DriveSel);
	LIBRETRO_S(Error);
	LIBRETRO_S(IntReason);
	LIBRETRO_S(Features);
	LIBRETRO_S(SecCount);
	LIBRETRO_S(SecNumber);
	LIBRETRO_S(GDStatus);
	LIBRETRO_S(ByteCount);
	LIBRETRO_S(GDROM_TICK);


	LIBRETRO_SA(EEPROM,0x100);
	LIBRETRO_S(EEPROM_loaded);


	LIBRETRO_S(maple_ddt_pending_reset);

	mcfg_SerializeDevices(data, total_size);


	LIBRETRO_S(FrameCount);
	LIBRETRO_S(pend_rend);


	LIBRETRO_SA(YUV_tempdata,512/4);
	LIBRETRO_S(YUV_dest);
	LIBRETRO_S(YUV_blockcount);
	LIBRETRO_S(YUV_x_curr);
	LIBRETRO_S(YUV_y_curr);
	LIBRETRO_S(YUV_x_size);
	LIBRETRO_S(YUV_y_size);




	LIBRETRO_S(fog_needs_update);
	LIBRETRO_SA(pvr_regs,pvr_RegSize);




	LIBRETRO_S(in_vblank);
	LIBRETRO_S(clc_pvr_scanline);
	LIBRETRO_S(i); 	// pvr_numscanlines
	LIBRETRO_S(i); 	// prv_cur_scanline
	LIBRETRO_S(i); 	// vblk_cnt
	LIBRETRO_S(i); 	// Line_Cycles
	LIBRETRO_S(i); 	// Frame_Cycles
	LIBRETRO_S(i);		// speed_load_mspdf
	LIBRETRO_S(i);		// ...
	LIBRETRO_S(i); 	// mips_counter
	LIBRETRO_S(i);		// full_rps
	LIBRETRO_S(i);		// ...


	LIBRETRO_SA(ta_type_lut,256);
	LIBRETRO_SA(ta_fsm,2049);
	LIBRETRO_S(ta_fsm_cl);


	LIBRETRO_S(pal_needs_update);
	LIBRETRO_SA(_pal_rev_256,4);
	LIBRETRO_SA(_pal_rev_16,64);
	LIBRETRO_SA(pal_rev_256,4);
	LIBRETRO_SA(pal_rev_16,64);
	for ( i = 0 ; i < 3 ; i++ )
	{
		u32 *ptr = decoded_colors[i] ;
		LIBRETRO_SA(ptr,65536);
	}
	LIBRETRO_S(tileclip_val);
	LIBRETRO_SA(f32_su8_tbl,65536);
	LIBRETRO_SA(FaceBaseColor,4);
	LIBRETRO_SA(FaceOffsColor,4);
	LIBRETRO_S(SFaceBaseColor);
	LIBRETRO_S(SFaceOffsColor);


	LIBRETRO_S(palette_index);
	LIBRETRO_S(KillTex);
	LIBRETRO_SA(palette16_ram,1024);
	LIBRETRO_SA(palette32_ram,1024);
	for (i = 0 ; i < 2 ; i++)
		for (j = 0 ; j < 8 ; j++)
		{
			u32 *ptr = detwiddle[i][j] ;
			LIBRETRO_SA(ptr,1024);
		}
	LIBRETRO_SA(vram.data, vram.size);




	LIBRETRO_SA(OnChipRAM.data,OnChipRAM_SIZE);

	register_serialize(CCN, data, total_size) ;
	register_serialize(UBC, data, total_size) ;
	register_serialize(BSC, data, total_size) ;
	register_serialize(DMAC, data, total_size) ;
	register_serialize(CPG, data, total_size) ;
	register_serialize(RTC, data, total_size) ;
	register_serialize(INTC, data, total_size) ;
	register_serialize(TMU, data, total_size) ;
	register_serialize(SCI, data, total_size) ;
	register_serialize(SCIF, data, total_size) ;

	LIBRETRO_SA(mem_b.data, mem_b.size);



	LIBRETRO_S(IRLPriority);
	LIBRETRO_SA(InterruptEnvId,32);
	LIBRETRO_SA(InterruptBit,32);
	LIBRETRO_SA(InterruptLevelBit,16);
	LIBRETRO_S(interrupt_vpend);
	LIBRETRO_S(interrupt_vmask);
	LIBRETRO_S(decoded_srimask);




	//default to nommu_full
	i = 3 ;
	if ( do_sqw_nommu == &do_sqw_nommu_area_3)
		i = 0 ;
	else if (do_sqw_nommu == &do_sqw_nommu_area_3_nonvmem)
		i = 1 ;
	else if (do_sqw_nommu==(sqw_fp*)&TAWriteSQ)
		i = 2 ;
	else if (do_sqw_nommu==&do_sqw_nommu_full)
		i = 3 ;

	LIBRETRO_S(i) ;

	LIBRETRO_SA((*p_sh4rcb).sq_buffer,64/8);

	//store these before unserializing and then restore after
	//void *getptr = &((*p_sh4rcb).cntx.sr.GetFull) ;
	//void *setptr = &((*p_sh4rcb).cntx.sr.SetFull) ;
	LIBRETRO_S((*p_sh4rcb).cntx);

	LIBRETRO_S(old_rm);
	LIBRETRO_S(old_dn);




	LIBRETRO_S(sh4_sched_ffb);
	LIBRETRO_S(i); // sh4_sched_intr

	//extern vector<sched_list> list;

	LIBRETRO_S(sch_list[aica_sched].tag) ;
	LIBRETRO_S(sch_list[aica_sched].start) ;
	LIBRETRO_S(sch_list[aica_sched].end) ;

	LIBRETRO_S(sch_list[rtc_sched].tag) ;
	LIBRETRO_S(sch_list[rtc_sched].start) ;
	LIBRETRO_S(sch_list[rtc_sched].end) ;

	LIBRETRO_S(sch_list[gdrom_sched].tag) ;
	LIBRETRO_S(sch_list[gdrom_sched].start) ;
	LIBRETRO_S(sch_list[gdrom_sched].end) ;

	LIBRETRO_S(sch_list[maple_sched].tag) ;
	LIBRETRO_S(sch_list[maple_sched].start) ;
	LIBRETRO_S(sch_list[maple_sched].end) ;

	LIBRETRO_S(sch_list[dma_sched_id].tag) ;
	LIBRETRO_S(sch_list[dma_sched_id].start) ;
	LIBRETRO_S(sch_list[dma_sched_id].end) ;

	for (int i = 0; i < 3; i++)
	{
	   LIBRETRO_S(sch_list[tmu_sched[i]].tag) ;
	   LIBRETRO_S(sch_list[tmu_sched[i]].start) ;
	   LIBRETRO_S(sch_list[tmu_sched[i]].end) ;
	}

	LIBRETRO_S(sch_list[render_end_sched].tag) ;
	LIBRETRO_S(sch_list[render_end_sched].start) ;
	LIBRETRO_S(sch_list[render_end_sched].end) ;

	LIBRETRO_S(sch_list[vblank_sched].tag) ;
	LIBRETRO_S(sch_list[vblank_sched].start) ;
	LIBRETRO_S(sch_list[vblank_sched].end) ;

	LIBRETRO_S(i); // sch_list[time_sync].tag
	LIBRETRO_S(i); // sch_list[time_sync].start
	LIBRETRO_S(i); // sch_list[time_sync].end

#ifdef HAVE_MODEM
    LIBRETRO_S(sch_list[modem_sched].tag) ;
    LIBRETRO_S(sch_list[modem_sched].start) ;
    LIBRETRO_S(sch_list[modem_sched].end) ;
#endif

	LIBRETRO_S(SCIF_SCFSR2);
	LIBRETRO_S(SCIF_SCFRDR2);
	LIBRETRO_S(SCIF_SCFDR2);


	LIBRETRO_S(BSC_PDTRA);




	LIBRETRO_SA(tmu_shift,3);
	LIBRETRO_SA(tmu_mask,3);
	LIBRETRO_SA(tmu_mask64,3);
	LIBRETRO_SA(old_mode,3);
	LIBRETRO_SA(tmu_ch_base,3);
	LIBRETRO_SA(tmu_ch_base64,3);




	LIBRETRO_SA(CCN_QACR_TR,2);




	LIBRETRO_SA(UTLB,64);
	LIBRETRO_SA(ITLB,4);
#if defined(NO_MMU)
	LIBRETRO_SA(sq_remap,64);
#else
	LIBRETRO_SA(ITLB_LRU_USE,64);
#endif



	LIBRETRO_S(NullDriveDiscType);
	LIBRETRO_SA(q_subchannel,96);


	LIBRETRO_S(FLASH_SIZE);
	LIBRETRO_S(BBSRAM_SIZE);
	LIBRETRO_S(BIOS_SIZE);
	LIBRETRO_S(RAM_SIZE);
	LIBRETRO_S(ARAM_SIZE);
	LIBRETRO_S(VRAM_SIZE);
	LIBRETRO_S(RAM_MASK);
	LIBRETRO_S(ARAM_MASK);
	LIBRETRO_S(VRAM_MASK);



	LIBRETRO_S(naomi_updates);
	LIBRETRO_S(i); // BoardID
	LIBRETRO_S(GSerialBuffer);
	LIBRETRO_S(BSerialBuffer);
	LIBRETRO_S(GBufPos);
	LIBRETRO_S(BBufPos);
	LIBRETRO_S(GState);
	LIBRETRO_S(BState);
	LIBRETRO_S(GOldClk);
	LIBRETRO_S(BOldClk);
	LIBRETRO_S(BControl);
	LIBRETRO_S(BCmd);
	LIBRETRO_S(BLastCmd);
	LIBRETRO_S(GControl);
	LIBRETRO_S(GCmd);
	LIBRETRO_S(GLastCmd);
	LIBRETRO_S(SerStep);
	LIBRETRO_S(SerStep2);
	LIBRETRO_SA(BSerial,69);
	LIBRETRO_SA(GSerial,69);
	LIBRETRO_S(reg_dimm_command);
	LIBRETRO_S(reg_dimm_offsetl);
	LIBRETRO_S(reg_dimm_parameterl);
	LIBRETRO_S(reg_dimm_parameterh);
	LIBRETRO_S(reg_dimm_status);
	LIBRETRO_S(NaomiDataRead);

	LIBRETRO_S(cycle_counter);
	LIBRETRO_S(i);	// idxnxx

#if FEAT_SHREC != DYNAREC_NONE
	LIBRETRO_S(state);
	LIBRETRO_S(div_som_reg1);
	LIBRETRO_S(div_som_reg2);
	LIBRETRO_S(div_som_reg3);


	//LIBRETRO_SA(CodeCache,CODE_SIZE) ;
	//LIBRETRO_SA(SH4_TCB,CODE_SIZE+4096);
	LIBRETRO_S(LastAddr);
	LIBRETRO_S(LastAddr_min);
	LIBRETRO_SA(block_hash,1024);
#endif

	// RegisterWrite, RegisterRead
	for (int i = 0; i < sh4_reg_count; i++)
	{
		LIBRETRO_S(i);
		LIBRETRO_S(i);
	}
	LIBRETRO_S(i); // fallback_blocks
	LIBRETRO_S(i); // total_blocks
	LIBRETRO_S(i);	// REMOVED_OPS


	LIBRETRO_S(settings.dreamcast.broadcast);
	LIBRETRO_S(settings.dreamcast.cable);
	LIBRETRO_S(settings.dreamcast.region);

	if (CurrentCartridge != NULL)
	   CurrentCartridge->Serialize(data, total_size);
	gd_hle_state.Serialize(data, total_size);

	return true ;
}

bool dc_unserialize(void **data, unsigned int *total_size, size_t actual_data_size)
{
	int i = 0;
	int j = 0;
	u8 dummy ;
	int dummy_int ;
	serialize_version_enum version = V1 ;

	*total_size = 0 ;

	LIBRETRO_US(version) ;

	//This normally isn't necessary - but we need some way to differentiate between save states
	//that were created after the format change and before the new format had a new version saved in it
	if (version == V1 && actual_data_size != 48324799 && actual_data_size != 48855967)
	   version = V2 ;

	LIBRETRO_US(aica_interr) ;
	LIBRETRO_US(aica_reg_L) ;
	LIBRETRO_US(e68k_out) ;
	LIBRETRO_US(e68k_reg_L) ;
	LIBRETRO_US(e68k_reg_M) ;

	LIBRETRO_USA(arm_Reg,RN_ARM_REG_COUNT);
	LIBRETRO_US(armIrqEnable);
	LIBRETRO_US(armFiqEnable);
	LIBRETRO_US(armMode);
	LIBRETRO_US(Arm7Enabled);
	LIBRETRO_USA(cpuBitsSet,256);
	LIBRETRO_SKIP(1); // intState
	LIBRETRO_SKIP(1); // stopState
	LIBRETRO_SKIP(1); // holdState

	LIBRETRO_US(dsp);

	for ( i = 0 ; i < 3 ; i++)
	{
		LIBRETRO_US(timers[i].c_step);
		LIBRETRO_US(timers[i].m_step);
	}


	LIBRETRO_USA(aica_ram.data,aica_ram.size) ;
	LIBRETRO_US(VREG);
	LIBRETRO_US(ARMRST);
	LIBRETRO_US(rtc_EN);
	if (version < V3)
	   LIBRETRO_US(dummy_int);	//dma_sched_id

	LIBRETRO_USA(aica_reg,0x8000);


	if (version < V7)
	{
		LIBRETRO_SKIP(4 * 16); 			// volume_lut
		LIBRETRO_SKIP(4 * 256 + 768);	// tl_lut. Due to a previous bug this is not 4 * (256 + 768)
		LIBRETRO_SKIP(4 * 64);			// AEG_ATT_SPS
		LIBRETRO_SKIP(4 * 64);			// AEG_DSR_SPS
		LIBRETRO_SKIP(2);					// pl
		LIBRETRO_SKIP(2);					// pr
	}
	channel_unserialize(data, total_size, version);

	LIBRETRO_USA(cdda_sector,CDDA_SIZE);
	LIBRETRO_US(cdda_index);
	LIBRETRO_SKIP(4 * 64); 	// mxlr
	LIBRETRO_SKIP(4);			// samples_gen


	register_unserialize(sb_regs, data, total_size) ;
	LIBRETRO_US(SB_ISTNRM);
	LIBRETRO_US(SB_FFST_rc);
	LIBRETRO_US(SB_FFST);



	//this is one-time init, no updates - don't need to serialize
	//extern RomChip sys_rom;

	LIBRETRO_US(sys_nvmem_sram.size);
	LIBRETRO_US(sys_nvmem_sram.mask);
	LIBRETRO_USA(sys_nvmem_sram.data,sys_nvmem_sram.size);

	LIBRETRO_US(sys_nvmem_flash.size);
	LIBRETRO_US(sys_nvmem_flash.mask);
	LIBRETRO_US(sys_nvmem_flash.state);
	LIBRETRO_USA(sys_nvmem_flash.data,sys_nvmem_flash.size);

	//this is one-time init, no updates - don't need to serialize
	//extern _vmem_handler area0_handler;




	LIBRETRO_US(GD_HardwareInfo) ;

	if (version < V3)
	   LIBRETRO_US(dummy_int);	//gdrom_sched
	LIBRETRO_US(sns_asc);
	LIBRETRO_US(sns_ascq);
	LIBRETRO_US(sns_key);

	LIBRETRO_US(packet_cmd);
	LIBRETRO_US(set_mode_offset);
	LIBRETRO_US(read_params);
	LIBRETRO_US(packet_cmd);
	LIBRETRO_US(read_buff);
	LIBRETRO_US(pio_buff);
	LIBRETRO_US(set_mode_offset);
	LIBRETRO_US(ata_cmd);
	LIBRETRO_US(cdda);
	LIBRETRO_US(gd_state);
	LIBRETRO_US(gd_disk_type);
	LIBRETRO_US(data_write_mode);
	LIBRETRO_US(DriveSel);
	LIBRETRO_US(Error);
	LIBRETRO_US(IntReason);
	LIBRETRO_US(Features);
	LIBRETRO_US(SecCount);
	LIBRETRO_US(SecNumber);
	LIBRETRO_US(GDStatus);
	LIBRETRO_US(ByteCount);
	LIBRETRO_US(GDROM_TICK);


	LIBRETRO_USA(EEPROM,0x100);
	LIBRETRO_US(EEPROM_loaded);

	if ( version == V1 )
	{
		//V1 saved NaomiState which was a struct of 3 u8 variables
		LIBRETRO_US(dummy);
		LIBRETRO_US(dummy);
		LIBRETRO_US(dummy);
	}

	LIBRETRO_US(maple_ddt_pending_reset);

	if ( version == V1 )
	{
		for (i = 0 ; i < 4 ; i++)
			for (j = 0 ; j < 6 ; j++)
				if ( MapleDevices[i][j] != 0 )
				{
					MapleDeviceType devtype = MapleDevices[i][j]->get_device_type() ;
					mcfg_DestroyDevice(i,j) ;
					mcfg_Create(devtype, i, j);
					MapleDevices[i][j]->maple_unserialize(data, total_size) ;
				}
	}
	else
		mcfg_UnserializeDevices(data, total_size);


	LIBRETRO_US(FrameCount);
	LIBRETRO_US(pend_rend);


	LIBRETRO_USA(YUV_tempdata,512/4);
	LIBRETRO_US(YUV_dest);
	LIBRETRO_US(YUV_blockcount);
	LIBRETRO_US(YUV_x_curr);
	LIBRETRO_US(YUV_y_curr);
	LIBRETRO_US(YUV_x_size);
	LIBRETRO_US(YUV_y_size);




	LIBRETRO_US(fog_needs_update);
	LIBRETRO_USA(pvr_regs,pvr_RegSize);
	fog_needs_update = true ;


	LIBRETRO_US(in_vblank);
	LIBRETRO_US(clc_pvr_scanline);
	LIBRETRO_US(i); 			// pvr_numscanlines
	LIBRETRO_US(i); 			// prv_cur_scanline
	LIBRETRO_US(i); 			// vblk_cnt
	LIBRETRO_US(i); 			// Line_Cycles
	LIBRETRO_US(i); 			// Frame_Cycles
	if (version < V3)
	{
	   LIBRETRO_US(dummy_int);	//render_end_sched
	   LIBRETRO_US(dummy_int);	//vblank_sched
	   LIBRETRO_US(dummy_int);	//time_sync
	}
	LIBRETRO_SKIP(8); 		// speed_load_mspdf
	LIBRETRO_US(dummy_int); // mips_counter
	LIBRETRO_SKIP(8); 		// full_rps



	LIBRETRO_USA(ta_type_lut,256);
	LIBRETRO_USA(ta_fsm,2049);
	LIBRETRO_US(ta_fsm_cl);


	LIBRETRO_US(pal_needs_update);
	LIBRETRO_USA(_pal_rev_256,4);
	LIBRETRO_USA(_pal_rev_16,64);
	LIBRETRO_USA(pal_rev_256,4);
	LIBRETRO_USA(pal_rev_16,64);
	for ( i = 0 ; i < 3 ; i++ )
	{
		u32 *ptr = decoded_colors[i] ;
		LIBRETRO_USA(ptr,65536);
	}
	LIBRETRO_US(tileclip_val);
	LIBRETRO_USA(f32_su8_tbl,65536);
	LIBRETRO_USA(FaceBaseColor,4);
	LIBRETRO_USA(FaceOffsColor,4);
	LIBRETRO_US(SFaceBaseColor);
	LIBRETRO_US(SFaceOffsColor);


	LIBRETRO_US(palette_index);
	LIBRETRO_US(KillTex);
	LIBRETRO_USA(palette16_ram,1024);
	LIBRETRO_USA(palette32_ram,1024);
	for (i = 0 ; i < 2 ; i++)
		for (j = 0 ; j < 8 ; j++)
		{
			u32 *ptr = detwiddle[i][j] ;
			LIBRETRO_USA(ptr,1024);
		}
	LIBRETRO_USA(vram.data, vram.size);



	LIBRETRO_USA(OnChipRAM.data,OnChipRAM_SIZE);

	register_unserialize(CCN, data, total_size, version < V4 ? 16 : 0) ;
	register_unserialize(UBC, data, total_size) ;
	register_unserialize(BSC, data, total_size) ;
	register_unserialize(DMAC, data, total_size) ;
	register_unserialize(CPG, data, total_size) ;
	register_unserialize(RTC, data, total_size) ;
	register_unserialize(INTC, data, total_size, version < V4 ? 4 : 0) ;
	register_unserialize(TMU, data, total_size) ;
	register_unserialize(SCI, data, total_size) ;
	register_unserialize(SCIF, data, total_size) ;

	LIBRETRO_USA(mem_b.data, mem_b.size);



	LIBRETRO_US(IRLPriority);
	LIBRETRO_USA(InterruptEnvId,32);
	LIBRETRO_USA(InterruptBit,32);
	LIBRETRO_USA(InterruptLevelBit,16);
	LIBRETRO_US(interrupt_vpend);
	LIBRETRO_US(interrupt_vmask);
	LIBRETRO_US(decoded_srimask);




	LIBRETRO_US(i) ;
	if ( i == 0 )
		do_sqw_nommu = &do_sqw_nommu_area_3 ;
	else if ( i == 1 )
		do_sqw_nommu = &do_sqw_nommu_area_3_nonvmem ;
	else if ( i == 2 )
		do_sqw_nommu = (sqw_fp*)&TAWriteSQ ;
	else if ( i == 3 )
		do_sqw_nommu = &do_sqw_nommu_full ;



	LIBRETRO_USA((*p_sh4rcb).sq_buffer,64/8);

	//store these before unserializing and then restore after
	//void *getptr = &((*p_sh4rcb).cntx.sr.GetFull) ;
	//void *setptr = &((*p_sh4rcb).cntx.sr.SetFull) ;
	LIBRETRO_US((*p_sh4rcb).cntx);
	//(*p_sh4rcb).cntx.sr.GetFull = getptr ;
	//(*p_sh4rcb).cntx.sr.SetFull = setptr ;

	LIBRETRO_US(old_rm);
	LIBRETRO_US(old_dn);


	LIBRETRO_US(sh4_sched_ffb);
	LIBRETRO_SKIP(4); // sh4_sched_intr
	if (version < V3)
	   LIBRETRO_US(dummy_int);	// sh4_sched_next_id

	//extern vector<sched_list> list;

	LIBRETRO_US(sch_list[aica_sched].tag) ;
	LIBRETRO_US(sch_list[aica_sched].start) ;
	LIBRETRO_US(sch_list[aica_sched].end) ;

	LIBRETRO_US(sch_list[rtc_sched].tag) ;
	LIBRETRO_US(sch_list[rtc_sched].start) ;
	LIBRETRO_US(sch_list[rtc_sched].end) ;

	LIBRETRO_US(sch_list[gdrom_sched].tag) ;
	LIBRETRO_US(sch_list[gdrom_sched].start) ;
	LIBRETRO_US(sch_list[gdrom_sched].end) ;

	LIBRETRO_US(sch_list[maple_sched].tag) ;
	LIBRETRO_US(sch_list[maple_sched].start) ;
	LIBRETRO_US(sch_list[maple_sched].end) ;

	LIBRETRO_US(sch_list[dma_sched_id].tag) ;
	LIBRETRO_US(sch_list[dma_sched_id].start) ;
	LIBRETRO_US(sch_list[dma_sched_id].end) ;

	for (int i = 0; i < 3; i++)
	{
	   LIBRETRO_US(sch_list[tmu_sched[i]].tag) ;
	   LIBRETRO_US(sch_list[tmu_sched[i]].start) ;
	   LIBRETRO_US(sch_list[tmu_sched[i]].end) ;
	}

	LIBRETRO_US(sch_list[render_end_sched].tag) ;
	LIBRETRO_US(sch_list[render_end_sched].start) ;
	LIBRETRO_US(sch_list[render_end_sched].end) ;

	LIBRETRO_US(sch_list[vblank_sched].tag) ;
	LIBRETRO_US(sch_list[vblank_sched].start) ;
	LIBRETRO_US(sch_list[vblank_sched].end) ;

	LIBRETRO_US(dummy_int); // sch_list[time_sync].tag
	LIBRETRO_US(dummy_int); // sch_list[time_sync].start
	LIBRETRO_US(dummy_int); // sch_list[time_sync].end

#ifdef HAVE_MODEM
	if ( version >= V2 )
	{
		LIBRETRO_US(sch_list[modem_sched].tag) ;
		LIBRETRO_US(sch_list[modem_sched].start) ;
		LIBRETRO_US(sch_list[modem_sched].end) ;
	}
#endif

	if (version < V3)
	{
	   LIBRETRO_US(dummy_int);	//aica_sched
	   LIBRETRO_US(dummy_int);	//rtc_sched
	}


	LIBRETRO_US(SCIF_SCFSR2);
	LIBRETRO_US(SCIF_SCFRDR2);
	LIBRETRO_US(SCIF_SCFDR2);


	LIBRETRO_US(BSC_PDTRA);




	LIBRETRO_USA(tmu_shift,3);
	LIBRETRO_USA(tmu_mask,3);
	LIBRETRO_USA(tmu_mask64,3);
	LIBRETRO_USA(old_mode,3);
	if (version < V3)
	{
	   //tmu_sched*3
	   LIBRETRO_US(dummy_int);
	   LIBRETRO_US(dummy_int);
	   LIBRETRO_US(dummy_int);
	}
	LIBRETRO_USA(tmu_ch_base,3);
	LIBRETRO_USA(tmu_ch_base64,3);




	LIBRETRO_USA(CCN_QACR_TR,2);


	if (version < V6)
	{
		for (int i = 0; i < 64; i++)
		{
			LIBRETRO_US(UTLB[i].Address);
			LIBRETRO_US(UTLB[i].Data);
		}
		for (int i = 0; i < 4; i++)
		{
			LIBRETRO_US(ITLB[i].Address);
			LIBRETRO_US(ITLB[i].Data);
		}
	}
	else
	{
		LIBRETRO_USA(UTLB,64);
		LIBRETRO_USA(ITLB,4);
	}
#if defined(NO_MMU)
	LIBRETRO_USA(sq_remap,64);
#else
	LIBRETRO_USA(ITLB_LRU_USE,64);
#endif




	LIBRETRO_US(NullDriveDiscType);
	LIBRETRO_USA(q_subchannel,96);


	LIBRETRO_US(FLASH_SIZE);
	LIBRETRO_US(BBSRAM_SIZE);
	LIBRETRO_US(BIOS_SIZE);
	LIBRETRO_US(RAM_SIZE);
	LIBRETRO_US(ARAM_SIZE);
	LIBRETRO_US(VRAM_SIZE);
	LIBRETRO_US(RAM_MASK);
	LIBRETRO_US(ARAM_MASK);
	LIBRETRO_US(VRAM_MASK);



	LIBRETRO_US(naomi_updates);
	if (version < V4)
	{
	   LIBRETRO_US(dummy_int);			// RomPioOffset
	   LIBRETRO_US(dummy_int);			// DmaOffset
	   LIBRETRO_US(dummy_int);			// DmaCount
	}
	LIBRETRO_US(dummy_int);		// BoardID
	LIBRETRO_US(GSerialBuffer);
	LIBRETRO_US(BSerialBuffer);
	LIBRETRO_US(GBufPos);
	LIBRETRO_US(BBufPos);
	LIBRETRO_US(GState);
	LIBRETRO_US(BState);
	LIBRETRO_US(GOldClk);
	LIBRETRO_US(BOldClk);
	LIBRETRO_US(BControl);
	LIBRETRO_US(BCmd);
	LIBRETRO_US(BLastCmd);
	LIBRETRO_US(GControl);
	LIBRETRO_US(GCmd);
	LIBRETRO_US(GLastCmd);
	LIBRETRO_US(SerStep);
	LIBRETRO_US(SerStep2);
	LIBRETRO_USA(BSerial,69);
	LIBRETRO_USA(GSerial,69);
	LIBRETRO_US(reg_dimm_command);
	LIBRETRO_US(reg_dimm_offsetl);
	LIBRETRO_US(reg_dimm_parameterl);
	LIBRETRO_US(reg_dimm_parameterh);
	LIBRETRO_US(reg_dimm_status);
	LIBRETRO_US(NaomiDataRead);
	if (version < V4)
	{
	   LIBRETRO_US(dummy_int);		// NAOMI_ROM_OFFSETH
	   LIBRETRO_US(dummy_int);		// NAOMI_ROM_OFFSETL
	   LIBRETRO_US(dummy_int);		// NAOMI_ROM_DATA
	   LIBRETRO_US(dummy_int);		// NAOMI_DMA_OFFSETH
	   LIBRETRO_US(dummy_int);		// NAOMI_DMA_OFFSETL
	   LIBRETRO_US(dummy_int);		// NAOMI_DMA_COUNT
	   LIBRETRO_US(dummy_int);		// NAOMI_BOARDID_WRITE
	   LIBRETRO_US(dummy_int);		// NAOMI_BOARDID_READ
	   LIBRETRO_US(dummy_int);		// NAOMI_COMM_OFFSET
	   LIBRETRO_US(dummy_int);		// NAOMI_COMM_DATA
	}

	LIBRETRO_US(cycle_counter);
	LIBRETRO_US(dummy_int);	// idxnxx

#if FEAT_SHREC != DYNAREC_NONE
	LIBRETRO_US(state);
	LIBRETRO_US(div_som_reg1);
	LIBRETRO_US(div_som_reg2);
	LIBRETRO_US(div_som_reg3);



	//LIBRETRO_USA(CodeCache,CODE_SIZE) ;
	//LIBRETRO_USA(SH4_TCB,CODE_SIZE+4096);
	LIBRETRO_US(LastAddr);
	LIBRETRO_US(LastAddr_min);
	LIBRETRO_USA(block_hash,1024);
#endif

	// RegisterRead, RegisterWrite
	for (int i = 0; i < sh4_reg_count; i++)
	{
		LIBRETRO_US(dummy_int);
		LIBRETRO_US(dummy_int);
	}
	LIBRETRO_US(dummy_int); // fallback_blocks
	LIBRETRO_US(dummy_int); // total_blocks
	LIBRETRO_US(dummy_int); // REMOVED_OPS

	if (version < V5)
	{
	   LIBRETRO_US(dummy_int);	// u16 kcode[4]
	   LIBRETRO_US(dummy_int);
	   LIBRETRO_US(dummy_int);	// u8 rt[4]
	   LIBRETRO_US(dummy_int);	// u8 lt[4]
	   LIBRETRO_US(dummy_int);	// u32 vks[4]
	   LIBRETRO_US(dummy_int);
	   LIBRETRO_US(dummy_int);
	   LIBRETRO_US(dummy_int);
	   LIBRETRO_US(dummy_int);	// s8 joyx[4]
	   LIBRETRO_US(dummy_int);	// s8 joyy[4]
	}

	if (version >= V3)
	{
	   LIBRETRO_US(settings.dreamcast.broadcast);
	   LIBRETRO_US(settings.dreamcast.cable);
	   LIBRETRO_US(settings.dreamcast.region);
	}
	if (version >= V4 && CurrentCartridge != NULL)
	{
	   CurrentCartridge->Unserialize(data, total_size);
	}
	if (version >= V7)
		gd_hle_state.Unserialize(data, total_size);

	return true ;
}
