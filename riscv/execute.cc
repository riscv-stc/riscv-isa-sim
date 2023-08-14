// See LICENSE for license details.

#include "processor.h"
#include "hwsync.h"
#include "mmu.h"
#include "disasm.h"
#include "soc_apb.h"
#include <cassert>

#ifdef RISCV_ENABLE_COMMITLOG
static void commit_log_reset(processor_t* p)
{
  p->get_state()->log_reg_write.clear();
  p->get_state()->log_mem_read.clear();
  p->get_state()->log_mem_write.clear();
  p->get_state()->log_mem_stc_write.clear();  
}

static void commit_log_stash_privilege(processor_t* p)
{
  state_t* state = p->get_state();
  state->last_inst_priv = state->prv;
  state->last_inst_xlen = p->get_xlen();
  state->last_inst_flen = p->get_flen();
}

static void commit_log_print_value(FILE *log_file, int width, const void *data)
{
  assert(log_file);

  switch (width) {
    case 8:
      fprintf(log_file, "0x%01" PRIx8, *(const uint8_t *)data);
      break;
    case 16:
      fprintf(log_file, "0x%04" PRIx16, *(const uint16_t *)data);
      break;
    case 32:
      fprintf(log_file, "0x%08" PRIx32, *(const uint32_t *)data);
      break;
    case 64:
      fprintf(log_file, "0x%016" PRIx64, *(const uint64_t *)data);
      break;
    default:
      // max lengh of vector
      if (((width - 1) & width) == 0) {
        const uint64_t *arr = (const uint64_t *)data;

        fprintf(log_file, "0x");
        for (int idx = width / 64 - 1; idx >= 0; --idx) {
          fprintf(log_file, "%016" PRIx64, arr[idx]);
        }
      } else {
        abort();
      }
      break;
  }
}

static void commit_log_print_value_stc(processor_t *p, int type, const void *data)
{
  FILE *log_file = p->get_log_file(); 
  
  fprintf(log_file, " 0x%08" PRIx32, *( uint32_t *)data);

  fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
  fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
  fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
  fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
  fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);     
  fprintf(log_file, " vme_data_type: 0x%08" PRIx32, VME_DTYPE);

}

static void commit_log_print_stc_mem_info(processor_t *p)
{
  auto custom = p->get_state()->log_mem_stc_write;
  if (custom.empty())
    return;

  FILE *log_file = p->get_log_file();  
  auto item = custom.front();
  for (auto item : custom) {  
    auto addr  = std::get<0>(item);   
    auto paddr = std::get<1>(item);   
    auto type  = std::get<2>(item);   
    int  size  = 0;   

    if(CMT_LOG_VME & type) {   //switch jump to case label  
      if(CMT_LOG_VME_COM == type) { 
        fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
        fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
        fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
        fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
        fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);    
        fprintf(log_file, " vme_data_type: 0x%08" PRIx32, VME_DTYPE);
        int pad = SHAPE1_COLUMN >= STRIDE_RD ? SHAPE1_COLUMN : STRIDE_RD ; 
        int xx  = SHAPE1_COLUMN;
        int yy  = SHAPE1_ROW;        
        if (0x0 == VME_DTYPE || 0x01 == VME_DTYPE) {
          size = 2;   //float16(0x0) or bfloat16
          for(int row=0; row < yy; row++) {
            for(int col=0; col < xx; col++) {
              int idx = row*pad+col;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
            }         
          } 
        } else if (0x02 == VME_DTYPE) {
          size = 4;   //float32
          for(int row=0; row < yy; row++) {
            for(int col=0; col < xx; col++) {
              int idx = row*pad+col;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));
            }         
          }           
        } else {
          std::cout << " error! vme_data_type = " << VME_DTYPE << std::endl;
        } 
      }

      if(CMT_LOG_VME_COM_H == type) { 
        fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
        fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
        fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
        fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
        fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);    
        fprintf(log_file, " vme_data_type: 0x%08" PRIx32, VME_DTYPE);
        
        int xx  = SHAPE1_COLUMN;        
        if (0x0 == VME_DTYPE || 0x01 == VME_DTYPE) {
          size = 2;   //float16(0x0) or bfloat16
          for(int col=0; col < xx; col++)  {
            int idx = col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));       
          } 
        } else if (0x02 == VME_DTYPE) {
          size = 4;   //float32
          for(int col=0; col < xx; col++) {
            int idx = col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));       
          }           
        } else {
          std::cout << " error! vme_data_type = " << VME_DTYPE << std::endl;
        } 
      }

      if(CMT_LOG_VME_COM_W == type) { 
        fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
        fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
        fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
        fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
        fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);    
        fprintf(log_file, " vme_data_type: 0x%08" PRIx32, VME_DTYPE);

        int yy  = SHAPE1_ROW;        
        if (0x0 == VME_DTYPE || 0x01 == VME_DTYPE) {
          size = 2;   //float16(0x0) or bfloat16
          for(int row=0; row < yy; row++) {
            int idx = row;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));        
          } 
        } else if (0x02 == VME_DTYPE) {
          size = 4;   //float32
          for(int row=0; row < yy; row++) {
            int idx = row;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));        
          }           
        } else {
          std::cout << " error! vme_data_type = " << VME_DTYPE << std::endl;
        } 
      }

      if(CMT_LOG_VME_CONV == type) { 
        fprintf(log_file, " vme_FM_in_col: %d", VME_WIN); 
        fprintf(log_file, " vme_FM_in_row: %d", VME_HIN); 
        fprintf(log_file, " vme_Cin: %d", VME_CIN); 
        fprintf(log_file, " vme_Stride_Cin: %d", VME_IFM_C_STRIDE); 

        fprintf(log_file, " vme_kernel_param1_Kw: %d", VME_KW); 
        fprintf(log_file, " vme_kernel_param1_Kh: %d", VME_KH); 
        fprintf(log_file, " vme_kernel_param1_Dh: %d", VME_DILATION_H); 
        fprintf(log_file, " vme_kernel_param1_Sh: %d", VME_SH); 
        fprintf(log_file, " vme_kernel_param2_Dw: %d", VME_DILATION_W); 
        fprintf(log_file, " vme_kernel_param2_Sw: %d", VME_SW); 
        fprintf(log_file, " vme_kernel_param2_S_kernel: %d", VME_K_C_STRIDE);  
        fprintf(log_file, " vme_FM_padding_u: %d", VME_N_PAD_U);
        fprintf(log_file, " vme_FM_padding_d: %d", VME_N_PAD_D);
        fprintf(log_file, " vme_FM_padding_l: %d", VME_N_PAD_L);
        fprintf(log_file, " vme_FM_padding_r: %d", VME_N_PAD_R);
        
        fprintf(log_file, " vme_FM_out_col: %d", VME_WOUT); 
        fprintf(log_file, " vme_FM_out_row: %d", VME_HOUT); 
        fprintf(log_file, " vme_Cout: %d", VME_CIN); 
        fprintf(log_file, " vme_Stride_Cout: %d", VME_OFM_C_STRIDE); 

        int xx  = VME_WOUT;
        int yy  = VME_HOUT;
        int zz  = VME_CIN;
        if (0x0 == VME_DTYPE || 0x01 == VME_DTYPE) {
          size = 2;   //float16, float16
          for(int out=0; out < zz; out++) { 
            for(int row=0; row < yy; row++) { 
              for(int col=0; col < xx; col++) {  
                int idx = out*yy*xx + row*xx + col;
                fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
                fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
              }         
            } 
          }
        } else if (0x02 == VME_DTYPE) {
          size = 4;   //float32
          for(int out=0; out < zz; out++) { 
            for(int row=0; row < yy; row++) { 
              for(int col=0; col < xx; col++) {  
                int idx = out*yy*xx + row*xx + col;
                fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
                fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));
              }         
            } 
          }
        } else {
          std::cout << " error! mme_data_type = " << MME_DATA_TYPE << std::endl;
        } 
      }

      if(CMT_LOG_VME_DATA8 == type) { 
        fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
        fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
        fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
        fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
        fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);
            
        int pad = SHAPE1_COLUMN >= STRIDE_RD ? SHAPE1_COLUMN : STRIDE_RD ;   
        size = 1;   //int8/uint8
        for(int row=0; row < SHAPE1_ROW; row++) {
          for(int col=0; col < SHAPE1_COLUMN; col++) {
            int idx = row*pad+col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%01" PRIx8, *((uint8_t *)paddr+idx));
          }         
        }         
      }   

      if(CMT_LOG_VME_DATA16 == type) { 
        fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
        fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
        fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
        fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
        fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);    
        
        int pad = SHAPE1_COLUMN >= STRIDE_RD ? SHAPE1_COLUMN : STRIDE_RD ;    
        size = 2;   //int16/fp16/bf16
        for(int row=0; row < SHAPE1_ROW; row++) {
          for(int col=0; col < SHAPE1_COLUMN; col++) {
            int idx = row*pad+col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
          }         
        }         
      }

      if(CMT_LOG_VME_DATA32 == type) { 
        fprintf(log_file, " vme_shape_col: %d", SHAPE1_COLUMN);   
        fprintf(log_file, " vme_shape_row: %d", SHAPE1_ROW);      
        fprintf(log_file, " vme_stride_d: %d" , STRIDE_RD);       
        fprintf(log_file, " vme_stride_s1: %d", STRIDE_RS1);      
        fprintf(log_file, " vme_stride_s2: %d", STRIDE_RS2);  

        int pad = SHAPE1_COLUMN >= STRIDE_RD ? SHAPE1_COLUMN : STRIDE_RD ;     
        size = 4;   //int32/fp32
        for(int row=0; row < SHAPE1_ROW; row++) {
          for(int col=0; col < SHAPE1_COLUMN; col++) {
            int idx = row*pad+col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%04" PRIx32, *((uint32_t *)paddr+idx));
          }         
        }         
      }   

    }

    else if(CMT_LOG_MTE == type) {    
      fprintf(log_file, " mte_shape_col: %d", MTE_SHAPE_COLUMN); 
      fprintf(log_file, " mte_shape_row: %d", MTE_SHAPE_ROW); 
      fprintf(log_file, " mte_stride_d: %d" , MTE_STRIDE_RD); 
      fprintf(log_file, " mte_stride_s: %d" , MTE_STRIDE_RS1); 
      fprintf(log_file, " mte_data_type: 0x%08" PRIx32, MTE_DATA_TYPE);

      int pad = MTE_SHAPE_COLUMN >= MTE_STRIDE_RD ? MTE_SHAPE_COLUMN : MTE_STRIDE_RD ;     
      if (0x0 == MTE_DATA_TYPE || 0x101 == MTE_DATA_TYPE) { 
        size = 2;   //float16(0x0) or bfloat16
        for(int row=0; row < MTE_SHAPE_ROW; row++) {
          for(int col=0; col < MTE_SHAPE_COLUMN; col++) {
            int idx = row*pad+col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
          }         
        } 
      } else if (0x202 == MTE_DATA_TYPE) {          
        size = 4;   //float32
        for(int row=0; row < MTE_SHAPE_ROW; row++) {
          for(int col=0; col < MTE_SHAPE_COLUMN; col++) {
            int idx = row*pad+col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));
          }           
        } 
      } else if (0x303 == MTE_DATA_TYPE) {          
        size = 1;   //int8
        for(int row=0; row < MTE_SHAPE_ROW; row++) {
          for(int col=0; col < MTE_SHAPE_COLUMN; col++) {
            int idx = row*pad+col;
            fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
            fprintf(log_file, " 0x%01" PRIx8, *((uint8_t *)paddr+idx));
          }           
        } 
      } else {
        std::cout << " error! mte_data_type = " << MTE_DATA_TYPE << std::endl;
      }     
    }

    else if(CMT_LOG_DMAE == type) {   
      fprintf(log_file, " dmae_shape_x: %d", DMAE_SHAPE_X); 
      fprintf(log_file, " dmae_shape_y: %d", DMAE_SHAPE_Y); 
      fprintf(log_file, " dmae_shape_z: %d", DMAE_SHAPE_Z); 
      fprintf(log_file, " dmae_stride_s1: %d", DMAE_STRIDE_S_X); 
      fprintf(log_file, " dmae_stride_s2: %d", DMAE_STRIDE_S_Y); 
      fprintf(log_file, " dmae_stride_d1: %d", DMAE_STRIDE_D_X); 
      fprintf(log_file, " dmae_stride_d2: %d", DMAE_STRIDE_D_Y); 
      fprintf(log_file, " dmae_data_type: 0x%08" PRIx32, DMAE_DATA_TYPE);

      int pad_x = DMAE_SHAPE_X >= DMAE_STRIDE_D_X ? DMAE_SHAPE_X : DMAE_STRIDE_D_X ;  
      // int pad_y = DMAE_SHAPE_Y >= DMAE_STRIDE_D_Y ? DMAE_SHAPE_Y : DMAE_STRIDE_D_Y ; 
      int pad_y = (DMAE_SHAPE_Y*pad_x) >= DMAE_STRIDE_D_Y ? (DMAE_SHAPE_Y*pad_x) : DMAE_STRIDE_D_Y ;           
      if (0x0 == DMAE_DATA_TYPE || 0x101 == DMAE_DATA_TYPE || 0x201 == DMAE_DATA_TYPE || 0x200 == DMAE_DATA_TYPE) { 
        size = 2;   //float16(0x0, 0x200) or bfloat16
        for(int zz=0; zz < DMAE_SHAPE_Z; zz++) {
          for(int yy=0; yy < DMAE_SHAPE_Y; yy++) {
            for(int xx=0; xx < DMAE_SHAPE_X; xx++) {
              int idx = zz*pad_y + yy*pad_x + xx;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
            }
          }           
        } 
      } else if (0x202 == DMAE_DATA_TYPE || 0x2 == DMAE_DATA_TYPE || 0x102 == DMAE_DATA_TYPE) { 
        size = 4;   //float32
        for(int zz=0; zz < DMAE_SHAPE_Z; zz++) {
          for(int yy=0; yy < DMAE_SHAPE_Y; yy++) {
            for(int xx=0; xx < DMAE_SHAPE_X; xx++) {
              int idx = zz*pad_y + yy*pad_x + xx;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));
            }
          }           
        } 
      } else if (0x303 == DMAE_DATA_TYPE) {          
        size = 1;   //int8
        for(int zz=0; zz < DMAE_SHAPE_Z; zz++) {
          for(int yy=0; yy < DMAE_SHAPE_Y; yy++) {
            for(int xx=0; xx < DMAE_SHAPE_X; xx++) {
              int idx = zz*pad_y + yy*pad_x + xx;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%01" PRIx8, *((uint8_t *)paddr+idx));
            }
          }           
        } 
      } else {
        std::cout << " error! dmae_data_type = " << DMAE_DATA_TYPE << std::endl;
      }     
    }

    else if(CMT_LOG_MME & type) {   
      int pad = 0;
      int xx  = 0;
      int yy  = 0;
      int zz  = 1; 
      if(CMT_LOG_MME_METR == type) { 
        pad = BC_SHAPE1_ROW >= BC_STRIDE_RD ? BC_SHAPE1_ROW : BC_STRIDE_RD ;
        xx  = BC_SHAPE1_ROW;
        yy  = BC_SHAPE1_COLUMN;
      } else if(CMT_LOG_MME_REDUCE == type || CMT_LOG_MME_DATA16== type ) { 
        pad = 1 >= BC_STRIDE_RD ? 1 : BC_STRIDE_RD ;
        xx  = 1;
        yy  = BC_SHAPE1_ROW;
      } else if(CMT_LOG_MME_MEMUL_MM == type) { 
        pad = BC_SHAPE2_COLUMN >= BC_STRIDE_RD ? BC_SHAPE2_COLUMN : BC_STRIDE_RD ;
        xx  = BC_SHAPE2_COLUMN;
        yy  = BC_SHAPE1_ROW;
      } else if(CMT_LOG_MME_MEMUL_TS == type) { 
        pad = BC_SHAPE2_COLUMN >= BC_STRIDE_RD ? BC_SHAPE2_COLUMN : BC_STRIDE_RD ;
        xx  = BC_SHAPE2_COLUMN;
        yy  = BC_SHAPE1_COLUMN;
      } else if(CMT_LOG_MME_CONV == type) {
        pad = CONV_COUT >= CONV_OUT_STRIDE ? CONV_COUT : CONV_OUT_STRIDE ; 
        xx  = CONV_OUT_COLUMN;
        yy  = CONV_OUT_ROW;
        zz  = CONV_COUT;
      }
      
      if(CMT_LOG_MME_METR == type || CMT_LOG_MME_REDUCE == type || CMT_LOG_MME_MEMUL_MM == type || CMT_LOG_MME_MEMUL_TS == type ) {
        fprintf(log_file, " mme_shape_s1_col: %d", BC_SHAPE1_COLUMN); 
        fprintf(log_file, " mme_shape_s1_row: %d", BC_SHAPE1_ROW); 
        fprintf(log_file, " mme_shape_s2_col: %d", BC_SHAPE2_COLUMN); 
        fprintf(log_file, " mme_shape_s2_row: %d", BC_SHAPE2_ROW); 
        fprintf(log_file, " mme_stride_d: %d"  , BC_STRIDE_RD); 
        fprintf(log_file, " mme_stride_s1: %d" , BC_STRIDE_RS1); 
        fprintf(log_file, " mme_stride_s2: %d" , BC_STRIDE_RS2); 
        fprintf(log_file, " mme_data_type: 0x%08" PRIx32, MME_DATA_TYPE);

        if (0x3030b == MME_DATA_TYPE || 0x3030c == MME_DATA_TYPE || 0x3040b == MME_DATA_TYPE || 0x3040c == MME_DATA_TYPE ||\
            0x3030f == MME_DATA_TYPE || 0x30310 == MME_DATA_TYPE || 0x3040f == MME_DATA_TYPE || 0x30410 == MME_DATA_TYPE ||\
            0x3090b == MME_DATA_TYPE || 0x30a0b == MME_DATA_TYPE || 0x3090c == MME_DATA_TYPE || 0x30a0c == MME_DATA_TYPE ||\
            0x30d0f == MME_DATA_TYPE || 0x30e0f == MME_DATA_TYPE || 0x30d10 == MME_DATA_TYPE || 0x30e10 == MME_DATA_TYPE ||\
            0x00000 == MME_DATA_TYPE || 0x10101 == MME_DATA_TYPE ) { 
          size = 2;   //float16, bfloat16
          for(int row=0; row < yy; row++) { 
            for(int col=0; col < xx; col++) {  
              int idx = row*pad+col;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
            }         
          } 
        } else if(0x2 == MME_DATA_TYPE || 0x10102 == MME_DATA_TYPE || 0x20202 == MME_DATA_TYPE ) { 
          size = 4;   //float32
          for(int row=0; row < yy; row++) {
            for(int col=0; col < xx; col++) {
              int idx = row*pad+col;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));
            }         
          } 
        } else if (0x30303 == MME_DATA_TYPE) {          
        size = 1;   //int8 metr
        for(int row=0; row < yy; row++) {
            for(int col=0; col < xx; col++) {
              int idx = row*pad+col;
              fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
              fprintf(log_file, " 0x%01" PRIx8, *((uint8_t *)paddr+idx));
              }
          }           
        } else {
          std::cout << " error! mme_data_type = " << MME_DATA_TYPE << std::endl;
        } 
      }

      if(CMT_LOG_MME_CONV == type) {
        fprintf(log_file, " conv_FM_in_col: %d", CONV_IN_COLUMN); 
        fprintf(log_file, " conv_FM_in_row: %d", CONV_IN_ROW); 
        fprintf(log_file, " conv_Cin: %d", CONV_CIN); 
        fprintf(log_file, " conv_Stride_Cin: %d", CONV_IN_STRIDE); 

        fprintf(log_file, " conv_kernel_param1_Kw: %d", CONV_KW); 
        fprintf(log_file, " conv_kernel_param1_Kh: %d", CONV_KH); 
        fprintf(log_file, " conv_kernel_param1_Dh: %d", CONV_DH); 
        fprintf(log_file, " conv_kernel_param1_Sh: %d", CONV_SH); 
        fprintf(log_file, " conv_kernel_param2_Dw: %d", CONV_DW); 
        fprintf(log_file, " conv_kernel_param2_Sw: %d", CONV_SW); 
        fprintf(log_file, " conv_kernel_param2_S_kernel: %d", CONV_S_KERNEL);  
        fprintf(log_file, " conv_padding: 0x%08" PRIx32, CONV_PADDING);
        fprintf(log_file, " conv_dequant_coeff: 0x%08" PRIx32, MME_DEQUANT_COEFF);
        fprintf(log_file, " conv_quant_coeff: 0x%08" PRIx32, MME_QUANT_COEFF);
        fprintf(log_file, " mme_data_type: 0x%08" PRIx32, MME_DATA_TYPE);
        
        fprintf(log_file, " conv_FM_out_col: %d", CONV_OUT_COLUMN); 
        fprintf(log_file, " conv_FM_out_row: %d", CONV_OUT_ROW); 
        fprintf(log_file, " conv_Cout: %d", CONV_COUT); 
        fprintf(log_file, " conv_Stride_Cout: %d", CONV_OUT_STRIDE); 
    
        if (0x3030b == MME_DATA_TYPE || 0x3030c == MME_DATA_TYPE || 0x3040b == MME_DATA_TYPE || 0x3040c == MME_DATA_TYPE ||\
            0x3030f == MME_DATA_TYPE || 0x30310 == MME_DATA_TYPE || 0x3040f == MME_DATA_TYPE || 0x30410 == MME_DATA_TYPE ||\
            0x3090b == MME_DATA_TYPE || 0x30a0b == MME_DATA_TYPE || 0x3090c == MME_DATA_TYPE || 0x30a0c == MME_DATA_TYPE ||\
            0x30d0f == MME_DATA_TYPE || 0x30e0f == MME_DATA_TYPE || 0x30d10 == MME_DATA_TYPE || 0x30e10 == MME_DATA_TYPE ||\
            0x00000 == MME_DATA_TYPE || 0x10101 == MME_DATA_TYPE ) { 
          size = 2;   //float16, float16
          for(int row=0; row < yy; row++) { 
            for(int col=0; col < xx; col++) { 
              for(int out=0; out < zz; out++) {
                int idx = row*xx*pad + col*pad + out;
                fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
                fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
              }         
            } 
          }
        } else if(0x2 == MME_DATA_TYPE || 0x10102 == MME_DATA_TYPE || 0x20202 == MME_DATA_TYPE ) { 
          size = 4;   //float32
          for(int row=0; row < yy; row++) { 
            for(int col=0; col < xx; col++) { 
              for(int out=0; out < zz; out++) {
                int idx = row*xx*pad + col*pad + out;
                fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
                fprintf(log_file, " 0x%08" PRIx32, *((uint32_t *)paddr+idx));
              }         
            } 
          }
        } else {
          std::cout << " error! mme_data_type = " << MME_DATA_TYPE << std::endl;
        } 
      }   
    
      if(CMT_LOG_MME_DATA16 == type) {
        fprintf(log_file, " mme_shape_s1_col: %d", BC_SHAPE1_COLUMN); 
        fprintf(log_file, " mme_shape_s1_row: %d", BC_SHAPE1_ROW); 
        fprintf(log_file, " mme_stride_d: %d"  , BC_STRIDE_RD); 
        fprintf(log_file, " mme_stride_s1: %d" , BC_STRIDE_RS1); 
        fprintf(log_file, " mme_data_type: 0x%08" PRIx32, MME_DATA_TYPE);
    
        if (0x00000 == MME_DATA_TYPE || 0x10101 == MME_DATA_TYPE || 0x20202 == MME_DATA_TYPE) { 
          size = 2;   //uint16
          for(int out=0; out < zz; out++) { 
            for(int row=0; row < yy; row++) { 
              for(int col=0; col < xx; col++) {  
                int idx = out*yy*xx + row*xx + col;
                fprintf(log_file, " mem 0x%016" PRIx64, (addr+idx*size));
                fprintf(log_file, " 0x%04" PRIx16, *((uint16_t *)paddr+idx));
              }         
            } 
          }
        } else {
          std::cout << " error! mme_data_type = " << MME_DATA_TYPE << std::endl;
        } 
      }  


    }

  }
}

static void commit_log_print_value(FILE *log_file, int width, uint64_t val)
{
  commit_log_print_value(log_file, width, &val);
}

const char* processor_t::get_symbol(uint64_t addr)
{
  return sim->get_symbol(addr);
}

static void commit_log_print_insn(processor_t *p, reg_t pc, insn_t insn)
{
  FILE *log_file = p->get_log_file();

  auto& reg = p->get_state()->log_reg_write;
  auto& load = p->get_state()->log_mem_read;
  auto& store = p->get_state()->log_mem_write;
  int priv = p->get_state()->last_inst_priv;
  int xlen = p->get_state()->last_inst_xlen;
  int flen = p->get_state()->last_inst_flen;

  // print core id on all lines so it is easy to grep
  fprintf(log_file, "core%4" PRId32 ": ", p->get_id());

  fprintf(log_file, "%1d ", priv);
  commit_log_print_value(log_file, xlen, pc);
  fprintf(log_file, " (");
  commit_log_print_value(log_file, insn.length() * 8, insn.bits());
  fprintf(log_file, ")");
  bool show_vec = false;
  bool log_stc  = false; 

  for (auto item : reg) {
    if (item.first == 0)
    {
      fprintf(log_file, " continue ");
      continue;
    }
      
    char prefix;
    int size;
    int rd = item.first >> 4;
    bool is_vec = false;
    bool is_vreg = false;
    switch (item.first & 0xf) {
    case 0:
      size = xlen;
      prefix = 'x';
      break;
    case 1:
      size = flen;
      prefix = 'f';        
      break;
    case 2:
      size = p->VU.VLEN;
      prefix = 'v';
      is_vreg = true;
      break;
    case 3:
      is_vec = true;
      break;
    case 4:
      size = xlen;
      prefix = 'c';
      break;
    case 8: 
      prefix = 'x'; 
      log_stc = true;  
      break;
    case 9: 
      prefix = 'f'; 
      log_stc = true;  
      break;
    default:
      assert("can't been here" && 0);
      break;
    }

    if (!show_vec && (is_vreg || is_vec)) {
        fprintf(log_file, " e%ld %s%ld l%ld",
                p->VU.vsew,
                p->VU.vflmul < 1 ? "mf" : "m",
                p->VU.vflmul < 1 ? (reg_t)(1 / p->VU.vflmul) : (reg_t)p->VU.vflmul,
                p->VU.vl);
        show_vec = true;
    }

    if (!is_vec) {
      if (prefix == 'c')
        fprintf(log_file, " c%d_%s ", rd, csr_name(rd));
      else
        fprintf(log_file, " %c%2d ", prefix, rd);
      if (is_vreg)
        commit_log_print_value(log_file, size, &p->VU.elt<uint8_t>(rd, 0));
      else {
        if (!log_stc)
          commit_log_print_value(log_file, size, item.second.v);
        else
          commit_log_print_value_stc(p, (item.first&0xf), item.second.v);  
      }
    }
  }

  for (auto item : load) {
    fprintf(log_file, " mem ");
    commit_log_print_value(log_file, xlen, std::get<0>(item));
  }

  for (auto item : store) {
    fprintf(log_file, " mem ");
    commit_log_print_value(log_file, xlen, std::get<0>(item));
    fprintf(log_file, " ");
    commit_log_print_value(log_file, std::get<2>(item) << 3, std::get<1>(item));
  }

  commit_log_print_stc_mem_info(p);
 
  fprintf(log_file, "\n");
}
#else
static void commit_log_reset(processor_t* p) {}
static void commit_log_stash_privilege(processor_t* p) {}
static void commit_log_print_insn(processor_t* p, reg_t pc, insn_t insn) {}
#endif

inline void processor_t::update_histogram(reg_t pc)
{
#ifdef RISCV_ENABLE_HISTOGRAM
  pc_histogram[pc]++;
#endif
}

// This is expected to be inlined by the compiler so each use of execute_insn
// includes a duplicated body of the function to get separate fetch.func
// function calls.
static reg_t execute_insn(processor_t* p, reg_t pc, insn_fetch_t fetch)
{
#ifdef RISCV_ENABLE_COMMITLOG
  commit_log_reset(p);
  commit_log_stash_privilege(p);
#endif
  reg_t npc;

  try {
    npc = fetch.func(p, fetch.insn, pc);
    if (npc != PC_SERIALIZE_BEFORE) {

#ifdef RISCV_ENABLE_COMMITLOG
      if (p->get_log_commits_enabled()) {
        commit_log_print_insn(p, pc, fetch.insn);
      }
#endif

     }
#ifdef RISCV_ENABLE_COMMITLOG
  } catch(mem_trap_t& t) {
      //handle segfault in midlle of vector load/store
      if (p->get_log_commits_enabled()) {
        for (auto item : p->get_state()->log_reg_write) {
          if ((item.first & 3) == 3) {
            commit_log_print_insn(p, pc, fetch.insn);
            break;
          }
        }
      }
      throw;
#endif
  } catch(...) {
    throw;
  }
  p->update_histogram(pc);

  return npc;
}

bool processor_t::slow_path()
{
  return debug || state.single_step != state.STEP_NONE || state.debug_mode;
}

// fetch/decode/execute loop
void processor_t::step(size_t n)
{
  if (!state.debug_mode) {
    if (halt_request == HR_REGULAR) {
      enter_debug_mode(DCSR_CAUSE_DEBUGINT);
    } else if (halt_request == HR_GROUP) {
      enter_debug_mode(DCSR_CAUSE_GROUP);
	   if (unlikely(state.wfi_flag && is_sync_idle()))
        state.wfi_flag = 0;
    } // !!!The halt bit in DCSR is deprecated.
    else if (state.dcsr.halt) {
      enter_debug_mode(DCSR_CAUSE_HALT);
      if (unlikely(state.wfi_flag && is_sync_idle()))
        state.wfi_flag = 0;
    }
  }

  /* 处理a53发给spike的消息 */
  get_sim()->a53_step();
  /* 处理driver发过来的消息 */
  get_sim()->pcie_step();

  // while (n > 0 && !this->suspend) {
  while (n > 0) {
    size_t instret = 0;
    reg_t pc = state.wfi_flag ? PC_SERIALIZE_WFI : state.pc;
    mmu_t* _mmu = mmu;

    #define advance_pc() \
     if (unlikely(invalid_pc(pc))) { \
       switch (pc) { \
         case PC_SERIALIZE_BEFORE: state.serialized = true; break; \
         case PC_SERIALIZE_AFTER: ++instret; break; \
         case PC_SERIALIZE_WFI: n = ++instret; break; \
         default: abort(); \
       } \
       pc = state.pc; \
       break; \
     } else { \
       state.pc = pc; \
       instret++; \
     }

    try
    {
      if (unlikely(sim->reset_signal(id))) {
		    /*hart reset. */
        reset();
        /* clear rst signal. */
        sim->clear_reset_signal(id);
        break;
      } else if (unlikely(this->suspend)) {
        break;
      }
      /* check current core sync state, if current core is sync,
       * and wait for other core to sync, swap current core to idle,
       * and let other core to execute insn. */
      if (async_done()) {
        pc = state.pc;
        state.wfi_flag = 0;
        if (async_trap != nullptr) {
          pc -= 4;
          std::rethrow_exception(async_trap);
        }
      #if 0
        printf("npc%d sync done %lx req_stat: %x \n", id, pc, ~get_hwsync_status());
        fflush(NULL);
      #endif
      }

      /* if sync is started, let other core that not in sync to execute.
       * the core will not ack interrupt if it is in sync. */
      if (unlikely(is_sync_started() || !is_pld_idle()))
	      break;
      /* check interrupt status, if there is any interrupt occur,
       * deal with interrupt and clear wfi_flag if it is set, and wakeup current core. */
      reg_t interrupts = state.mip;
      if (unlikely(interrupts & state.mie)) {
        if (unlikely(state.wfi_flag)) {
          pc = state.pc;
          state.wfi_flag = 0;
        }
      }
      take_pending_interrupt(interrupts);

      if (unlikely(state.wfi_flag))
        break;
      if (unlikely(slow_path()))
      {
        while (instret < n)
        {
          if (unlikely(!state.serialized && state.single_step == state.STEP_STEPPED)) {
            state.single_step = state.STEP_NONE;
            if (!state.debug_mode) {
              enter_debug_mode(DCSR_CAUSE_STEP);
              // enter_debug_mode changed state.pc, so we can't just continue.
              break;
            }
          }

          if (unlikely(state.single_step == state.STEP_STEPPING)) {
            state.single_step = state.STEP_STEPPED;
          }

          insn_fetch_t fetch = mmu->load_insn(pc);
          if (debug && !state.serialized)
            disasm(fetch.insn);

          uint64_t start_rdtsc = 0;
          uint64_t endl_rdtsc = 0;
          uint64_t riscv_clks = 0;

          /* core所在的grp在sync，该核运行时 hs_sync_timer_cnts 计数器累加 */
          if (hwsync->is_hs_group_sync(id) && (!state.pld)) {
            start_rdtsc = get_host_clks();
            misc_dev->inst_cnt(fetch.insn.bits());
            pc = execute_insn(this, pc, fetch);
            endl_rdtsc = get_host_clks();

            if (likely(endl_rdtsc > start_rdtsc)) {
              riscv_clks = host_clks_2_npc(endl_rdtsc-start_rdtsc, fetch.insn.bits());
            } else {
              riscv_clks = host_clks_2_npc(endl_rdtsc+(~(uint64_t)(0))-start_rdtsc, fetch.insn.bits());
            }
            hwsync->hwsync_timer_cnts_add(id, riscv_clks);
            if (hwsync->is_hwsync_timeout(id)) {
              //hwsync->hwsync_timer_clear(id);
              hwsync->hwsync_clear();
              throw trap_sync_timeout_trigger();
            }
          } else {
            misc_dev->inst_cnt(fetch.insn.bits());
            pc = execute_insn(this, pc, fetch);
          }

          advance_pc();
        }
      }
      else while (instret < n)
      {
        // This code uses a modified Duff's Device to improve the performance
        // of executing instructions. While typical Duff's Devices are used
        // for software pipelining, the switch statement below primarily
        // benefits from separate call points for the fetch.func function call
        // found in each execute_insn. This function call is an indirect jump
        // that depends on the current instruction. By having an indirect jump
        // dedicated for each icache entry, you improve the performance of the
        // host's next address predictor. Each case in the switch statement
        // allows for the program flow to contine to the next case if it
        // corresponds to the next instruction in the program and instret is
        // still less than n.
        //
        // According to Andrew Waterman's recollection, this optimization
        // resulted in approximately a 2x performance increase.

        // This figures out where to jump to in the switch statement
        size_t idx = _mmu->icache_index(pc);

        // This gets the cached decoded instruction from the MMU. If the MMU
        // does not have the current pc cached, it will refill the MMU and
        // return the correct entry. ic_entry->data.func is the C++ function
        // corresponding to the instruction.
        auto ic_entry = _mmu->access_icache(pc);

        // This macro is included in "icache.h" included within the switch
        // statement below. The indirect jump corresponding to the instruction
        // is located within the execute_insn() function call.
        #define ICACHE_ACCESS(i) { \
          insn_fetch_t fetch = ic_entry->data; \
          uint64_t start_rdtsc = 0;   \
          uint64_t endl_rdtsc = 0;    \
          uint64_t riscv_clks = 0;    \
          if (hwsync->is_hs_group_sync(id) && (!state.pld)) {   \
            start_rdtsc = get_host_clks();   \
            misc_dev->inst_cnt(fetch.insn.bits());     \
            pc = execute_insn(this, pc, fetch); \
            endl_rdtsc = get_host_clks();    \
            if (likely(endl_rdtsc > start_rdtsc)) {   \
              riscv_clks = host_clks_2_npc(endl_rdtsc-start_rdtsc, fetch.insn.bits());    \
            } else {    \
              riscv_clks = host_clks_2_npc(endl_rdtsc+(~(uint64_t)(0))-start_rdtsc, fetch.insn.bits());   \
            }   \
            hwsync->hwsync_timer_cnts_add(id, riscv_clks);    \
            if (hwsync->is_hwsync_timeout(id)) {   \
              /* hwsync->hwsync_timer_clear(id); */   \
              hwsync->hwsync_clear();   \
              throw trap_sync_timeout_trigger();  \
            }   \
          }else {   \
            misc_dev->inst_cnt(fetch.insn.bits());     \
            pc = execute_insn(this, pc, fetch); \
          }   \
          ic_entry = ic_entry->next; \
          if (i == mmu_t::ICACHE_ENTRIES-1) break; \
          if (unlikely(ic_entry->tag != pc)) break; \
          if (unlikely(instret+1 == n)) break; \
          instret++; \
          state.pc = pc; \
        }

        // This switch statement implements the modified Duff's device as
        // explained above.
        switch (idx) {
          // "icache.h" is generated by the gen_icache script
          #include "icache.h"
        }

        advance_pc();
      }
    }
    catch(trap_t& t)
    {
      take_trap(t, pc);
      n = instret;

      if (unlikely(state.single_step == state.STEP_STEPPED)) {
        state.single_step = state.STEP_NONE;
        enter_debug_mode(DCSR_CAUSE_STEP);
      }
    }
    catch (trigger_matched_t& t)
    {
      if (mmu->matched_trigger) {
        // This exception came from the MMU. That means the instruction hasn't
        // fully executed yet. We start it again, but this time it won't throw
        // an exception because matched_trigger is already set. (All memory
        // instructions are idempotent so restarting is safe.)

        insn_fetch_t fetch = mmu->load_insn(pc);
        misc_dev->inst_cnt(fetch.insn.bits());
        pc = execute_insn(this, pc, fetch);
        advance_pc();

        delete mmu->matched_trigger;
        mmu->matched_trigger = NULL;
      }
      switch (state.mcontrol[t.index].action) {
        case ACTION_DEBUG_MODE:
          enter_debug_mode(DCSR_CAUSE_HWBP);
          break;
        case ACTION_DEBUG_EXCEPTION: {
          insn_trap_t trap(CAUSE_BREAKPOINT, t.address);
          take_trap(trap, pc);
          break;
        }
        default:
          abort();
      }
    }
    catch (wait_for_interrupt_t &t)
    {
      // Return to the outer simulation loop, which gives other devices/harts a
      // chance to generate interrupts.
      //
      // In the debug ROM this prevents us from wasting time looping, but also
      // allows us to switch to other threads only once per idle loop in case
      // there is activity.
      n = instret;
      if (!unlikely(IS_EXECUTE_IN_DEBUGROM(pc)))
        state.wfi_flag = 1;
    }

    state.minstret += instret;
    n -= instret;

    /* 处理a53发给spike的消息 */
    get_sim()->a53_step();
    /* 处理driver发过来的消息 */
    get_sim()->pcie_step();
  }
}
