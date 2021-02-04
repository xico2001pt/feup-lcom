#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include <machine/int86.h>
#include <vbe.h>

int vbe_call(struct reg86 * r){  
  /* Make the BIOS call */
  if(sys_int86(r) != OK ) {
    printf("\tvbe_set_mode: sys_int86() failed \n");
    return 1;
  }
  switch(r->ah){
    case AH_CALL_FAILED: printf("BIOS call failed\n"); return 1;
    case AH_NOT_SUPPORTED_CONF: printf("BIOS not supported config\n"); return 1;
    case AH_NOT_SUPPORTED_MODE: printf("BIOS not supported mode\n"); return 1;
  }
  return 0;
}

uint32_t far_to_virtual(phys_bytes b, void *base) {
  return (((b >> 16) << 4) + PB2OFF(b) + (uint32_t) base);
}

int vbe_controller_get_info(vg_vbe_contr_info_t *info_p) {
  mmap_t mem;
  if(lm_alloc(sizeof(VbeInfoBlock), &mem) == NULL){
    printf("lm_alloc failed\n");
    return 1;
  }

  memcpy((VbeInfoBlock *) mem.virt, "VBE2", 4);

  struct reg86 r;
  memset(&r, 0, sizeof(r));
  r.intno = BIOS_VID_CARD;
  r.ah = VBE_CALL;
  r.al = GET_VBE_CONTROLLER_INFO;
  r.es = PB2BASE(mem.phys);
  r.di = PB2OFF(mem.phys);
  if(vbe_call(&r)){
    printf("vbe_mode_get_info failed\n");
    if(lm_free(&mem)){
      printf("lm_free failed\n");
    }
    return 1;
  }

  // Copy fields
  VbeInfoBlock * info = (VbeInfoBlock *) mem.virt;
  void * difference = (void *) ((phys_bytes) mem.virt - mem.phys);

  memcpy(info_p->VBESignature, info->VbeSignature, 4);
  memcpy(info_p->VBEVersion, info->VbeVersion, 2);
  info_p->OEMString = (char *) far_to_virtual((phys_bytes) info->OemStringPtr, difference);
  info_p->VideoModeList = (uint16_t *) far_to_virtual((phys_bytes) info->VideoModePtr, difference);
  info_p->TotalMemory = info->TotalMemory * 64;
  info_p->OEMVendorNamePtr = (char *) far_to_virtual((phys_bytes) info->OemVendorNamePtr, difference);
  info_p->OEMProductNamePtr = (char *) far_to_virtual((phys_bytes) info->OemProductNamePtr, difference);
  info_p->OEMProductRevPtr = (char *) far_to_virtual((phys_bytes) info->OemProductRevPtr, difference);
  if(!lm_free(&mem)){
    printf("lm_free failed\n");
    return 1;
  }
  return 0;
}

int vbe_mode_get_info(uint16_t mode, vbe_mode_info_t *vmi_p){
  printf("get info");
  mmap_t mem;
  if(lm_alloc(sizeof(vbe_mode_info_t), &mem) == NULL){
    printf("lm_alloc failed\n");
    return 1;
  }
  printf("before | phys:0x%x  virt:0x%x   PB2BASE:0x%x  PB2OFF:0x%x\n", mem.phys, mem.virt, PB2BASE(mem.phys), PB2OFF(mem.phys));

  struct reg86 r;
  memset(&r, 0, sizeof(r));
  r.intno = BIOS_VID_CARD;
  r.ah = VBE_CALL;
  r.al = GET_VBE_MODE_INFO;
  r.es = PB2BASE(mem.phys);
  r.di = PB2OFF(mem.phys);
  r.cx = mode;
  if(vbe_call(&r)){
    printf("vbe_mode_get_info failed\n");
    if(lm_free(&mem)){
      printf("lm_free failed\n");
    }
    return 1;
  }

  printf("after | phys:0x%x  virt:0x%x   PB2BASE:0x%x  PB2OFF:0x%x\n", mem.phys, mem.virt, PB2BASE(mem.phys), PB2OFF(mem.phys));

  vbe_mode_info_t * info = (vbe_mode_info_t *) mem.virt;
  *vmi_p = *info;

  if(!lm_free(&mem)){
    printf("lm_free failed\n");
    return 1;
  }
  
  return 0;
}

int vbe_set_mode(uint16_t mode){
  struct reg86 r;   
  memset(&r, 0, sizeof(r));  
  r.intno = BIOS_VID_CARD; /* BIOS video services */
  r.ah = VBE_CALL;    /* Set Video Mode function */
  r.al = SET_VBE_MODE;
  r.bx = LINEAR_BUFFER | mode;
  if(vbe_call(&r)){
    printf("VBE call failed\n");
    return 1;
  }
  return 0;
}

void * vbe_map_vram(unsigned int phys_base_ptr, unsigned int vram_size){
  struct minix_mem_range mr;
  unsigned int vram_base = phys_base_ptr;  /* VRAM's physical addresss */
  int r;

  /* Allow memory mapping */
  mr.mr_base = (phys_bytes) vram_base;    
  mr.mr_limit = mr.mr_base + vram_size;

  printf("mapped VRAM base:0x%x size:%d limit:0x%x\n", mr.mr_base, vram_size, mr.mr_limit); 

  if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  /* Map memory */
  void * video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

  if(video_mem == MAP_FAILED)
    panic("couldn't map video memory");
  return video_mem;
}
