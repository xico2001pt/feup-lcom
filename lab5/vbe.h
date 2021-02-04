#ifndef __VBE__
#define __VBE__
#include <machine/int86.h>

// BIOS services
#define BIOS_VID_CARD 0x10

// AH consts
#define AH_CALL_FAILED 0X01
#define AH_NOT_SUPPORTED_CONF 0X02
#define AH_NOT_SUPPORTED_MODE 0X03

#define VBE_CALL 0x4F

// Functions
#define SET_VBE_MODE 0x02 // Passed to BX register
#define GET_VBE_MODE_INFO 0x01
#define GET_VBE_CONTROLLER_INFO 0x00

// Graphics modes
#define MODE_1024x768_INDEX 0x105
#define MODE_640x480_DIRECT 0x110
#define MODE_800x600_DIRECT 0x115
#define MODE_1280x1024_DIRECT 0x11A
#define MODE_1152x864_DIRECT 0x14C
#define MODE_CGA 0x03

// Buffer model
#define LINEAR_BUFFER BIT(14)

typedef struct __attribute__((packed)){
  char VbeSignature[4];
  BCD VbeVersion[2];
  uint32_t OemStringPtr;
  uint8_t Capabilities[4];
  uint32_t VideoModePtr;
  uint16_t TotalMemory;
  uint16_t OemSoftwareRev;
  uint32_t OemVendorNamePtr;
  uint32_t OemProductNamePtr;
  uint32_t OemProductRevPtr;
  uint8_t Reserved[222];
  uint8_t OemData[256];

} VbeInfoBlock;

uint32_t far_to_virtual(phys_bytes p, void *base);

int vbe_controller_get_info(vg_vbe_contr_info_t *info_p);

int vbe_mode_get_info(uint16_t mode, vbe_mode_info_t *vmi_p);

int vbe_set_mode(uint16_t mode);

int vbe_call(struct reg86 *r);

void * vbe_map_vram(unsigned int phys_base_ptr, unsigned int vram_size);

#endif
