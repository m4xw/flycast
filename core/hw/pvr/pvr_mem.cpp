/*
	PowerVR interface to plugins
	Handles YUV conversion (slow and ugly -- but hey it works ...)

	Most of this was hacked together when i needed support for YUV-dma for thps2 ;)
*/

#include "types.h"
#include "pvr_mem.h"
#include "spg.h"
#include "ta.h"
#include "Renderer_if.h"
#include "hw/mem/_vmem.h"

//TODO : move code later to a plugin
//TODO : Fix registers arrays , they must be smaller now doe to the way SB registers are handled
#include "hw/holly/holly_intc.h"
#include "hw/holly/sb.h"

//YUV converter code :)
//inits the YUV converter
u32 YUV_tempdata[512/4];//512 bytes

u32 YUV_dest=0;

u32 YUV_blockcount;

u32 YUV_x_curr;
u32 YUV_y_curr;

u32 YUV_x_size;
u32 YUV_y_size;

static u32 YUV_index = 0;

void YUV_init(void)
{
   YUV_x_curr     = 0;
   YUV_y_curr     = 0;
   YUV_dest       = TA_YUV_TEX_BASE&VRAM_MASK;//TODO : add the masking needed
   TA_YUV_TEX_CNT = 0;
   YUV_blockcount = (TA_YUV_TEX_CTRL.yuv_u_size + 1) * (TA_YUV_TEX_CTRL.yuv_v_size + 1);

   if (TA_YUV_TEX_CTRL.yuv_tex != 0)
   {
      die ("YUV: Not supported configuration\n");
      YUV_x_size     = 16;
      YUV_y_size     = 16;
   }
   else // yesh!!!
   {
      YUV_x_size = (TA_YUV_TEX_CTRL.yuv_u_size + 1) * 16;
      YUV_y_size = (TA_YUV_TEX_CTRL.yuv_v_size + 1) * 16;
   }
   YUV_index = 0;
}


static INLINE u8 GetY420(int x, int y,u8* base)
{
	//u32 base=0;
	if (x > 7)
	{
		x -= 8;
		base += 64;
	}

	if (y > 7)
	{
		y -= 8;
		base += 128;
	}
	
	return base[x+y*8];
}

static INLINE u8 GetUV420(int x, int y,u8* base)
{
	int realx=x>>1;
	int realy=y>>1;

	return base[realx+realy*8];
}

#pragma GCC push_options
#pragma GCC optimize ("-O2")
void YUV_Block8x8(u8* inuv,u8* iny, u8* out)
{
	u8* line_out_0=out+0;
	u8* line_out_1=out+YUV_x_size*2;

	for (int y=0;y<8;y+=2)
	{
		for (int x=0;x<8;x+=2)
		{
			u8 u=inuv[0];
			u8 v=inuv[64];

			line_out_0[0]=u;
			line_out_0[1]=iny[0];
			line_out_0[2]=v;
			line_out_0[3]=iny[1];

			line_out_1[0]=u;
			line_out_1[1]=iny[8+0];
			line_out_1[2]=v;
			line_out_1[3]=iny[8+1];

			inuv+=1;
			iny+=2;

			line_out_0+=4;
			line_out_1+=4;
		}
		iny+=8;
		inuv+=4;

		line_out_0+=YUV_x_size*4-8*2;
		line_out_1+=YUV_x_size*4-8*2;
	}
}
#pragma GCC pop_options

static INLINE void YUV_Block384(u8* in, u8* out)
{
	u8* inuv=in;
	u8* iny=in+128;
	u8* p_out=out;

	YUV_Block8x8(inuv+ 0,iny+  0,p_out);                    //(0,0)
	YUV_Block8x8(inuv+ 4,iny+64,p_out+8*2);                 //(8,0)
	YUV_Block8x8(inuv+32,iny+128,p_out+YUV_x_size*8*2);     //(0,8)
	YUV_Block8x8(inuv+36,iny+192,p_out+YUV_x_size*8*2+8*2); //(8,8)
}

static INLINE void YUV_ConvertMacroBlock(u8* datap)
{
	//do shit
	TA_YUV_TEX_CNT++;

	YUV_Block384((u8*)datap,vram.data + YUV_dest);

	YUV_dest+=32;

	YUV_x_curr+=16;
	if (YUV_x_curr==YUV_x_size)
	{
		YUV_dest+=15*YUV_x_size*2;
		YUV_x_curr=0;
		YUV_y_curr+=16;
		if (YUV_y_curr==YUV_y_size)
		{
			YUV_y_curr=0;
		}
	}

	if (YUV_blockcount==TA_YUV_TEX_CNT)
	{
		YUV_init();
		
		asic_RaiseInterrupt(holly_YUV_DMA);
	}
}

void YUV_data(u32* data , u32 count)
{
	if (YUV_blockcount==0)
	{
		die("YUV_data : YUV decoder not inited , *WATCH*\n");
		//wtf ? not inited
		YUV_init();
	}

   u32 block_size = TA_YUV_TEX_CTRL.yuv_form == 0 ? 384 : 512;

	verify(block_size==384); //no support for 512

	
	count*=32;

	while (count > 0)
	{
		if (YUV_index + count >= block_size)
		{
			//more or exactly one block remaining
			u32 dr = block_size - YUV_index;				//remaining bytes til block end
			if (YUV_index == 0)
			{
				// Avoid copy
				YUV_ConvertMacroBlock((u8 *)data);				//convert block
			}
			else
			{
				memcpy(&YUV_tempdata[YUV_index >> 2], data, dr);//copy em
				YUV_ConvertMacroBlock((u8 *)&YUV_tempdata[0]);	//convert block
				YUV_index = 0;
			}
			data += dr >> 2;									//count em
			count -= dr;
		}
		else
		{	//less that a whole block remaining
			memcpy(&YUV_tempdata[YUV_index >> 2], data, count);	//append it
			YUV_index += count;
			count = 0;
		}
	}
	verify(count==0);
}

//Regs

//vram 32-64b

//read
u8 DYNACALL pvr_read_area1_8(u32 addr)
{
   INFO_LOG(MEMORY, "%08x: 8-bit VRAM reads are not possible", addr);
   return 0;
}

u16 DYNACALL pvr_read_area1_16(u32 addr)
{
   return *(u16*)&vram.data[pvr_map32(addr)];
}
u32 DYNACALL pvr_read_area1_32(u32 addr)
{
   return *(u32*)&vram.data[pvr_map32(addr)];
}

//write
void DYNACALL pvr_write_area1_8(u32 addr,u8 data)
{
   INFO_LOG(MEMORY, "%08x: 8-bit VRAM writes are not possible", addr);
}

void DYNACALL pvr_write_area1_16(u32 addr,u16 data)
{
   u32 vaddr = addr & VRAM_MASK;
   if (vaddr >= fb_watch_addr_start && vaddr < fb_watch_addr_end)
   {
      fb_dirty = true;
   }
   *(u16*)&vram.data[pvr_map32(addr)]=data;
}

void DYNACALL pvr_write_area1_32(u32 addr,u32 data)
{
   u32 vaddr = addr & VRAM_MASK;
   if (vaddr >= fb_watch_addr_start && vaddr < fb_watch_addr_end)
   {
      fb_dirty = true;
   }
   *(u32*)&vram.data[pvr_map32(addr)] = data;
}

void TAWrite(u32 address,u32* data,u32 count)
{
   u32 address_w=address&0x1FFFFFF;//correct ?
   if (address_w<0x800000)//TA poly
   {
      ta_vtx_data(data,count);
   }
   else if(address_w<0x1000000) //Yuv Converter
   {
      YUV_data(data,count);
   }
   else //Vram Writef
   {
		//shouldn't really get here (?) -> works on dc :D need to handle lmmodes
		DEBUG_LOG(MEMORY, "Vram TAWrite 0x%X , bkls %d\n", address, count);
		verify(SB_LMMODE0 == 0);
		memcpy(&vram.data[address & VRAM_MASK],data,count * 32);
   }
}

#include "hw/sh4/sh4_mmr.h"

void NOINLINE MemWrite32(void* dst, void* src)
{
	memcpy((u64*)dst,(u64*)src,32);
}

#if HOST_CPU!=CPU_ARM
extern "C" void DYNACALL TAWriteSQ(u32 address,u8* sqb)
{
   u32 address_w=address&0x1FFFFFF;//correct ?
   u8* sq=&sqb[address&0x20];

   if (likely(address_w<0x800000))//TA poly
   {
      ta_vtx_data32(sq);
   }
   else if(likely(address_w<0x1000000)) //Yuv Converter
   {
      YUV_data((u32*)sq,1);
   }
   else //Vram Writef
   {
		// Used by WinCE
		DEBUG_LOG(MEMORY, "Vram TAWriteSQ 0x%X SB_LMMODE0 %d", address, SB_LMMODE0);
		if (SB_LMMODE0 == 0)
		{
			// 64b path
			MemWrite32(&vram[address_w&(VRAM_MASK-0x1F)],sq);
		}
		else
		{
			// 32b path
			for (int i = 0; i < 8; i++, address_w += 4)
			{
				pvr_write_area1_32(address_w, ((u32 *)sq)[i]);
			}
		}
   }
}
#endif

//Misc interface

//Reset -> Reset - Initialise to default values
void pvr_Reset(bool Manual)
{
   if (!Manual)
      vram.Zero();
}

#define VRAM_BANK_BIT 0x400000

u32 pvr_map32(u32 offset32)
{
   //64b wide bus is achieved by interleaving the banks every 32 bits
   const u32 bank_bit = VRAM_BANK_BIT;
   const u32 static_bits = (VRAM_MASK - (VRAM_BANK_BIT * 2 - 1)) | 3;
   const u32 offset_bits = (VRAM_BANK_BIT - 1) & ~3;
   u32 bank = (offset32 & VRAM_BANK_BIT) / VRAM_BANK_BIT;
   u32 rv = offset32 & static_bits;
   rv |= (offset32 & offset_bits) * 2;
   rv |= bank * 4;

   return rv;
}

f32 vrf(u32 addr)
{
	return *(f32*)&vram.data[pvr_map32(addr)];
}
u32 vri(u32 addr)
{
	return *(u32*)&vram.data[pvr_map32(addr)];
}
