#include "types.h"
#include "hw/sh4/sh4_mem.h"

#include "sb_mem.h"
#include "sb.h"
#include "hw/pvr/pvr_regs.h"
#include "hw/pvr/pvr_mem.h"
#include "hw/gdrom/gdrom_if.h"
#include "hw/aica/aica_if.h"
#include "hw/naomi/naomi.h"
#include "hw/modem/modem.h"

#include "hw/flashrom/flashrom.h"
#include "reios/reios.h"

/*
	Dreamcast 'area 0' emulation
	Pretty much all peripheral registers are mapped here

	Routing is mostly handled here, as well as flash/SRAM emulation
*/

RomChip sys_rom;
SRamChip sys_nvmem_sram;
DCFlashChip sys_nvmem_flash;

extern bool bios_loaded;
extern char nvmem_file[PATH_MAX];
extern char nvmem_file2[PATH_MAX];	// AtomisWave

static const char *get_rom_prefix(void)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
         return "dc_";
      case DC_PLATFORM_DEV_UNIT:
         return "hkt_";
      case DC_PLATFORM_NAOMI:
         return "naomi_";
      case DC_PLATFORM_NAOMI2:
         return "n2_";
      case DC_PLATFORM_ATOMISWAVE:
    	 // Not used
         return "";
   }
   return "";
}

static const char *get_rom_names(void)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
         return "%boot.bin;%boot.bin.bin;%bios.bin;%bios.bin.bin";
      case DC_PLATFORM_NAOMI:
         return "%boot.bin;%boot.bin.bin;%bios.bin;%bios.bin.bin;epr-21576d.bin";
      case DC_PLATFORM_NAOMI2:
         return "%boot.bin;%boot.bin.bin;%bios.bin;%bios.bin.bin";
      case DC_PLATFORM_ATOMISWAVE:
    	 // Not used
         return "";
   }

   return NULL; 
}

static void add_isp_to_nvmem(DCFlashChip *flash)
{
	u8 block[64];
	if (!flash->ReadBlock(FLASH_PT_USER, FLASH_USER_INET, block))
	{
		memset(block, 0, sizeof(block));
		strcpy((char *)block + 2, "PWBrowser");
		block[12] = 0x1c;
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_INET, block);

		memset(block, 0, sizeof(block));
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_INET + 1, block);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_INET + 2, block);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_INET + 3, block);
		memset(block + 27, 0xFF, sizeof(block) - 27);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_INET + 4, block);
		memset(block, 0xFF, sizeof(block));
		for (u32 i = FLASH_USER_INET + 5; i <= 0xbf; i++)
			flash->WriteBlock(FLASH_PT_USER, i, block);

		flash_isp1_block isp1;
		memset(&isp1, 0, sizeof(isp1));
		isp1._unknown[0] = 0xFF;
		isp1._unknown[1] = 0xFE;
		isp1._unknown[2] = 0xFF;
		isp1._unknown[3] = 0xFF;
		memcpy(isp1.sega, "SEGA", 4);
		strcpy(isp1.username, "flycast1");
		strcpy(isp1.password, "password");
		strcpy(isp1.phone, "1234567");
		if (flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP1, &isp1) != 1)
			WARN_LOG(FLASHROM, "Failed to save ISP information to flash RAM");

		memset(block, 0xFF, sizeof(block));
		block[34] = 0;
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP1 + 1, block);
		memset(block, 0xFF, sizeof(block));
		block[9] = 0;
		memset(block + 49, 0, 13);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP1 + 2, block);
		memset(block, 0xFF, sizeof(block));
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP1 + 3, block);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP1 + 4, block);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP1 + 5, block);

		flash_isp2_block isp2;
		memset(&isp2, 0, sizeof(isp2));
		memcpy(isp2.sega, "SEGA", 4);
		strcpy(isp2.username, "flycast2");
		strcpy(isp2.password, "password");
		strcpy(isp2.phone, "1234567");
		if (flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP2, &isp2) != 1)
			WARN_LOG(FLASHROM, "Failed to save ISP information to flash RAM");
		u8 block[64];
		memset(block, 0xFF, sizeof(block));
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP2 + 1, block);
		block[9] = 0;
		memset(block + 49, 0, 13);
		flash->WriteBlock(FLASH_PT_USER, FLASH_USER_ISP2 + 2, block);
		for (u32 i = FLASH_USER_ISP2 + 3; i <= 0xEA; i++)
			flash->WriteBlock(FLASH_PT_USER, i, block);
	}
}

void FixUpFlash()
{
	if (settings.System == DC_PLATFORM_DREAMCAST || settings.System == DC_PLATFORM_DEV_UNIT)
	{
     	sys_nvmem_flash.Validate();
     	// overwrite factory flash settings
     	if (settings.dreamcast.region <= 2)
     	{
     		sys_nvmem_flash.data[0x1a002] = '0' + settings.dreamcast.region;
     		sys_nvmem_flash.data[0x1a0a2] = '0' + settings.dreamcast.region;
     	}
     	if (settings.dreamcast.language <= 5)
     	{
     		sys_nvmem_flash.data[0x1a003] = '0' + settings.dreamcast.language;
     		sys_nvmem_flash.data[0x1a0a3] = '0' + settings.dreamcast.language;
     	}
     	if (settings.dreamcast.broadcast <= 3)
     	{
     		sys_nvmem_flash.data[0x1a004] = '0' + settings.dreamcast.broadcast;
     		sys_nvmem_flash.data[0x1a0a4] = '0' + settings.dreamcast.broadcast;
     	}
     	// overwrite user settings
     	struct flash_syscfg_block syscfg;
     	int res = sys_nvmem_flash.ReadBlock(FLASH_PT_USER, FLASH_USER_SYSCFG, &syscfg);

     	if (!res)
     	{
     		// write out default settings
     		memset(&syscfg, 0xff, sizeof(syscfg));
     		syscfg.time_lo = 0;
     		syscfg.time_hi = 0;
     		syscfg.lang = 0;
     		syscfg.mono = 0;
     		syscfg.autostart = 1;
     	}
     	u32 time = GetRTC_now();
     	syscfg.time_lo = time & 0xffff;
     	syscfg.time_hi = time >> 16;
     	if (settings.dreamcast.language <= 5)
     		syscfg.lang = settings.dreamcast.language;

     	if (sys_nvmem_flash.WriteBlock(FLASH_PT_USER, FLASH_USER_SYSCFG, &syscfg) != 1)
     		WARN_LOG(FLASHROM, "Failed to save time and language to flash RAM");

     	add_isp_to_nvmem(&sys_nvmem_flash);
	}
}

static bool nvmem_load(const string& root,
      const string& s1, const char *s2)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
         sys_nvmem_flash.Load(root, get_rom_prefix(), s1.c_str(), s2);
        	return true;
      case DC_PLATFORM_ATOMISWAVE:
         return sys_nvmem_sram.Load(nvmem_file) && sys_nvmem_flash.Load(nvmem_file2);
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
         return sys_nvmem_sram.Load(nvmem_file);
   }

   return false;
}

bool LoadRomFiles(const string& root)
{
   if (settings.System != DC_PLATFORM_ATOMISWAVE)
   {
	  if (!sys_rom.Load(root, get_rom_prefix(), get_rom_names(), "bootrom"))
	  {
		 if (settings.System == DC_PLATFORM_DREAMCAST || settings.System == DC_PLATFORM_DEV_UNIT)
		 {
			// Dreamcast absolutely needs a BIOS
			msgboxf("Unable to find bios in %s. Exiting...", MBX_ICONERROR, root.c_str());
			return false;
		 }
	  }
	  else
		 bios_loaded = true;
   }
   nvmem_load(root, "%nvmem.bin;%flash_wb.bin;%flash.bin;%flash.bin.bin", "nvram");

   return true;
}

void SaveRomFiles(const string& root)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
         sys_nvmem_flash.Save(root, get_rom_prefix(), "nvmem.bin", "nvmem");
         break;
      case DC_PLATFORM_ATOMISWAVE:
    	 sys_nvmem_sram.Save(nvmem_file);
    	 sys_nvmem_flash.Save(nvmem_file2);
         break;
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
         sys_nvmem_sram.Save(nvmem_file);
         break;
   }
}

u8 *get_nvmem_data(void)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
      case DC_PLATFORM_ATOMISWAVE:
         return sys_nvmem_flash.data;
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
         return sys_nvmem_sram.data;
   }

   return NULL;
}

bool LoadHle(const string& root) {
	if (!nvmem_load(root, "%nvmem.bin;%flash_wb.bin;%flash.bin;%flash.bin.bin", "nvram")) {
		WARN_LOG(FLASHROM, "No nvmem loaded\n");
	}

   reios_reset(sys_rom.data, &sys_nvmem_flash);

   return true;
}

u32 ReadFlash(u32 addr,u32 sz)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
         return sys_nvmem_flash.Read(addr,sz);
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
      case DC_PLATFORM_ATOMISWAVE:
         return sys_nvmem_sram.Read(addr,sz);
   }

   return 0;
}

void WriteFlash(u32 addr,u32 data,u32 sz)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
         sys_nvmem_flash.Write(addr,data,sz);
         break;
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
      case DC_PLATFORM_ATOMISWAVE:
         sys_nvmem_sram.Write(addr,data,sz);
         break;
   }
}

u32 ReadBios(u32 addr,u32 sz)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
         return sys_rom.Read(addr,sz);
      case DC_PLATFORM_ATOMISWAVE:
         return sys_nvmem_flash.Read(addr,sz);
   }
   return 0;
}

void WriteBios(u32 addr,u32 data,u32 sz)
{
   switch (settings.System)
   {
      case DC_PLATFORM_DREAMCAST:
      case DC_PLATFORM_DEV_UNIT:
      case DC_PLATFORM_NAOMI:
      case DC_PLATFORM_NAOMI2:
         INFO_LOG(MEMORY, "Write to [Boot ROM] is not possible, addr=%x, data=%x, size=%d", addr, data, sz);
         break;
      case DC_PLATFORM_ATOMISWAVE:
    	 if (sz != 1)
    	 {
 			INFO_LOG(MEMORY, "Invalid access size @%08x data %x sz %d", addr, data, sz);
 			return;
    	 }
    	 sys_nvmem_flash.Write(addr, data, sz);
         break;
   }
}

//Area 0 mem map
//0x00000000- 0x001FFFFF	:MPX	System/Boot ROM
//0x00200000- 0x0021FFFF	:Flash Memory
//0x00400000- 0x005F67FF	:Unassigned
//0x005F6800- 0x005F69FF	:System Control Reg.
//0x005F6C00- 0x005F6CFF	:Maple i/f Control Reg.
//0x005F7000- 0x005F70FF	:GD-ROM / NAOMI BD Reg.
//0x005F7400- 0x005F74FF	:G1 i/f Control Reg.
//0x005F7800- 0x005F78FF	:G2 i/f Control Reg.
//0x005F7C00- 0x005F7CFF	:PVR i/f Control Reg.
//0x005F8000- 0x005F9FFF	:TA / PVR Core Reg.
//0x00600000- 0x006007FF	:MODEM
//0x00600800- 0x006FFFFF	:G2 (Reserved)
//0x00700000- 0x00707FFF	:AICA- Sound Cntr. Reg.
//0x00710000- 0x0071000B	:AICA- RTC Cntr. Reg.
//0x00800000- 0x00FFFFFF	:AICA- Wave Memory
//0x01000000- 0x01FFFFFF	:Ext. Device
//0x02000000- 0x03FFFFFF*	:Image Area*	2MB

//use unified size handler for registers
//it really makes no sense to use different size handlers on em -> especially when we can use templates :p
template<u32 sz, class T>
T DYNACALL ReadMem_area0(u32 addr)
{
	addr &= 0x01FFFFFF;//to get rid of non needed bits
	const u32 base=(addr>>16);
	//map 0x0000 to 0x01FF to Default handler
	//mirror 0x0200 to 0x03FF , from 0x0000 to 0x03FFF
	//map 0x0000 to 0x001F
	if ((settings.System != DC_PLATFORM_ATOMISWAVE && base<=0x001F)
		  || (settings.System == DC_PLATFORM_ATOMISWAVE && base <= 0x0001))//	:MPX	System/Boot ROM
	{
		return ReadBios(addr,sz);
	}
	//map 0x0020 to 0x0021
	else if ((base>= 0x0020) && (base<= 0x0021)) // :Flash Memory
	{
		return ReadFlash(addr&0x1FFFF,sz);
	}
	//map 0x005F to 0x005F
	else if (likely(base==0x005F))
	{
		if ( /*&& (addr>= 0x00400000)*/ (addr<= 0x005F67FF)) // :Unassigned
		{
			INFO_LOG(MEMORY, "Read from area0_32 not implemented [Unassigned], addr=%x", addr);
		}
		else if ((addr>= 0x005F7000) && (addr<= 0x005F70FF)) // GD-ROM
		{
         if (settings.System == DC_PLATFORM_NAOMI || settings.System == DC_PLATFORM_ATOMISWAVE)
            return (T)ReadMem_naomi(addr,sz);
         return (T)ReadMem_gdrom(addr,sz);
		}
		else if (likely((addr>= 0x005F6800) && (addr<=0x005F7CFF))) //	/*:PVR i/f Control Reg.*/ -> ALL SB registers now
		{
			return (T)sb_ReadMem(addr,sz);
		}
		else if (likely((addr>= 0x005F8000) && (addr<=0x005F9FFF))) //	:TA / PVR Core Reg.
		{
         if (sz != 4) return 0;		// House of the Dead 2
			return (T)PvrReg(addr, u32);
		}
	}
	//map 0x0060 to 0x0060
	else if ((base ==0x0060) /*&& (addr>= 0x00600000)*/ && (addr<= 0x006007FF)) //	:MODEM
	{
	   if (settings.System == DC_PLATFORM_DREAMCAST)
#ifdef HAVE_MODEM
		  return (T)ModemReadMem_A0_006(addr, sz);
#else
		  return (T)0;
#endif
	   else
		  return (T)libExtDevice_ReadMem_A0_006(addr,sz);
	}
	//map 0x0060 to 0x006F
	else if ((base >=0x0060) && (base <=0x006F) && (addr>= 0x00600800) && (addr<= 0x006FFFFF)) //	:G2 (Reserved)
	{
		EMUERROR2("Read from area0_32 not implemented [G2 (Reserved)], addr=%x",addr);
	}
	//map 0x0070 to 0x0070
	else if ((base ==0x0070) /*&& (addr>= 0x00700000)*/ && (addr<=0x00707FFF)) //	:AICA- Sound Cntr. Reg.
	{
		return (T) ReadMem_aica_reg(addr,sz);//libAICA_ReadReg(addr,sz);
	}
	//map 0x0071 to 0x0071
	else if ((base ==0x0071) /*&& (addr>= 0x00710000)*/ && (addr<= 0x0071000B)) //	:AICA- RTC Cntr. Reg.
	{
      return (T)ReadMem_aica_rtc(addr,sz);
	}
	//map 0x0080 to 0x00FF
	else if ((base >=0x0080) && (base <=0x00FF) /*&& (addr>= 0x00800000) && (addr<=0x00FFFFFF)*/) //	:AICA- Wave Memory
	{
      switch (sz)
      {
         case 1:
            return aica_ram.data[addr & ARAM_MASK];
         case 2:
            return *(u16*)&aica_ram.data[addr & ARAM_MASK];
         case 4:
            return *(u32*)&aica_ram.data[addr & ARAM_MASK];
      }
	}
	//map 0x0100 to 0x01FF
	else if ((base >=0x0100) && (base <=0x01FF) /*&& (addr>= 0x01000000) && (addr<= 0x01FFFFFF)*/) //	:Ext. Device
	{
		return (T)libExtDevice_ReadMem_A0_010(addr,sz);
	}
	return 0;
}

template<u32 sz, class T>
void  DYNACALL WriteMem_area0(u32 addr,T data)
{
	addr &= 0x01FFFFFF;//to get rid of non needed bits

	const u32 base=(addr>>16);

	//map 0x0000 to 0x001F
	if ((settings.System != DC_PLATFORM_ATOMISWAVE && base<=0x001F)
		  || (settings.System == DC_PLATFORM_ATOMISWAVE && base <= 0x0001))// :MPX System/Boot ROM
	{
		WriteBios(addr,data,sz);
	}
	//map 0x0020 to 0x0021
	else if ((base >=0x0020) && (base <=0x0021) /*&& (addr>= 0x00200000) && (addr<= 0x0021FFFF)*/) // Flash Memory
	{
		WriteFlash(addr,data,sz);
	}
	//map 0x0040 to 0x005F -> actually, I'll only map 0x005F to 0x005F, b/c the rest of it is unspammed (left to default handler)
	//map 0x005F to 0x005F
	else if ( likely(base==0x005F) )
	{
		if (/*&& (addr>= 0x00400000) */ (addr<= 0x005F67FF)) // Unassigned
		{
			EMUERROR4("Write to area0_32 not implemented [Unassigned], addr=%x,data=%x,size=%d",addr,data,sz);
		}
		else if ((addr>= 0x005F7000) && (addr<= 0x005F70FF)) // GD-ROM
		{
         if   (settings.System == DC_PLATFORM_NAOMI ||
               settings.System == DC_PLATFORM_ATOMISWAVE)
            WriteMem_naomi(addr,data,sz);
         else
            WriteMem_gdrom(addr,data,sz);
		}
		else if ( likely((addr>= 0x005F6800) && (addr<=0x005F7CFF)) ) // /*:PVR i/f Control Reg.*/ -> ALL SB registers
		{
			sb_WriteMem(addr,data,sz);
		}
		else if ( likely((addr>= 0x005F8000) && (addr<=0x005F9FFF)) ) // TA / PVR Core Reg.
		{
			verify(sz==4);
			pvr_WriteReg(addr,data);
		}
	}
	//map 0x0060 to 0x0060
	else if ((base ==0x0060) /*&& (addr>= 0x00600000)*/ && (addr<= 0x006007FF)) // MODEM
	{
#ifdef HAVE_MODEM
	   if (settings.System == DC_PLATFORM_DREAMCAST)
		  ModemWriteMem_A0_006(addr, data, sz);
	   else
#else
	   if (settings.System != DC_PLATFORM_DREAMCAST)
#endif
		  libExtDevice_WriteMem_A0_006(addr,data,sz);
	}
	//map 0x0060 to 0x006F
	else if ((base >=0x0060) && (base <=0x006F) && (addr>= 0x00600800) && (addr<= 0x006FFFFF)) // G2 (Reserved)
	{
		EMUERROR4("Write to area0_32 not implemented [G2 (Reserved)], addr=%x,data=%x,size=%d",addr,data,sz);
	}
	//map 0x0070 to 0x0070
	else if ((base >=0x0070) && (base <=0x0070) /*&& (addr>= 0x00700000)*/ && (addr<=0x00707FFF)) // AICA- Sound Cntr. Reg.
	{
		WriteMem_aica_reg(addr,data,sz);
		return;
	}
	//map 0x0071 to 0x0071
	else if ((base >=0x0071) && (base <=0x0071) /*&& (addr>= 0x00710000)*/ && (addr<= 0x0071000B)) // AICA- RTC Cntr. Reg.
	{
      WriteMem_aica_rtc(addr,data,sz);
		return;
	}
	//map 0x0080 to 0x00FF
	else if ((base >=0x0080) && (base <=0x00FF) /*&& (addr>= 0x00800000) && (addr<=0x00FFFFFF)*/) // AICA- Wave Memory
	{

      switch (sz)
      {
         case 1:
            aica_ram.data[addr & ARAM_MASK]         = (u8)data;
            break;
         case 2:
            *(u16*)&aica_ram.data[addr & ARAM_MASK] = (u16)data;
            break;
         case 4:
            *(u32*)&aica_ram.data[addr & ARAM_MASK] = data;
            break;
      }
		return;
	}
	//map 0x0100 to 0x01FF
	else if ((base >=0x0100) && (base <=0x01FF) /*&& (addr>= 0x01000000) && (addr<= 0x01FFFFFF)*/) // Ext. Device
	{
		libExtDevice_WriteMem_A0_010(addr,data,sz);
	}
	return;
}

//Init/Res/Term
void sh4_area0_Init(void)
{
	sb_Init();
}

void sh4_area0_Reset(bool Manual)
{
	sb_Reset(Manual);
	sys_rom.Reset();
	sys_nvmem_sram.Reset();
	sys_nvmem_flash.Reset();
}

void sh4_area0_Term(void)
{
	sb_Term();
}


//AREA 0
_vmem_handler area0_handler;


void map_area0_init()
{

	area0_handler = _vmem_register_handler_Template(ReadMem_area0,WriteMem_area0);
}
void map_area0(u32 base)
{
	verify(base<0xE0);

	_vmem_map_handler(area0_handler,0x00|base,0x01|base);

	//0x0240 to 0x03FF mirrors 0x0040 to 0x01FF (no flashrom or bios)
	//0x0200 to 0x023F are unused
	_vmem_mirror_mapping(0x02|base,0x00|base,0x02);
}
