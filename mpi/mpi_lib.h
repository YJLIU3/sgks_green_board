
/********************************************************************************
* File                : mpi_lib.h
* Intro               : mpi Interface
* Author              : 00003
* Version             : v1.0
* Notes               :
* Change History 	  :
Date              Version        Changed By          Changes
* --------------------------------------------------------------------------------
2017-11-20        v0.1           00003               create

* Copyright(c) 2017-2020, by SGK Technology Co. Ltd., All rights reserved
*******************************************************************************/


#ifndef _MPI_LIB_H_
#define _MPI_LIB_H_

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "mpi_common.h"

#ifdef  __cplusplus
extern "C"
{
#endif

/*************************************************************************
*                               macro                                    *
**************************************************************************/
#define MAX_FW_FILE_NUM  			(3)
#define MAX_VI_NUM       			(4)
#define MAX_VO_NUM       			(2)
#define MAX_SOURCE_BUFFER_NUM  	(4)
#define MAX_PREVIEW_NUM 			(3)
#define MAX_MPI_MODULE_NUM 		(6)
#define MAX_VDEC_RESCALE_SUB_NUM 		(3)
#define MAX_VDEC_WIDTH 			(4800)
#define MAX_VDEC_HEIGHT 			(2700)

//#define ENBLE_DEBUG_PRINT

#ifdef ENBLE_DEBUG_PRINT
#define Printf(string...)	\
do{\
	printf("%s(%d)[%s]: ", __FILE__, __LINE__, __FUNCTION__);\
	printf(string);\
} while(0)
#else
#define Printf(string...)	\
do{\
} while(0)
#endif

#define ERR_PRINT(string...)   \
do{\
printf("%s(%d)[%s] error: %s\n", __FILE__, __LINE__, __FUNCTION__,(string));\
} while(0)

#define CHECK_POINT_NULL_RETURN(p, rt_value) \
if (p == NULL) \
{ \
	Printf("NULL POINT\n"); \
	return rt_value; \
}

#define MPI_DRIVER_MEDIA_DEV          "/dev/sgks_mdi"

typedef struct _DBGPARAM
{
        unsigned char   lpszName[32];           // @field Name of module
        unsigned char   rglpszZones[16][32];    // @field names of zones for first 16 bits
        unsigned long   ulZoneMask;             // @field Current zone Mask
} DBGPARAM, *LPDBGPARAM;

extern  DBGPARAM    dpCurSettings;
#define DEBUGZONE(n)  (dpCurSettings.ulZoneMask&(0x00000001<<(n)))

//#define DEBUG

#ifdef DEBUG
#define ZONE_ERROR           DEBUGZONE(0)
#define ZONE_WARN            DEBUGZONE(1)
#define ZONE_INIT            DEBUGZONE(2)
#define ZONE_SYS             DEBUGZONE(3)
#define ZONE_INFO            DEBUGZONE(4)
#define ZONE_VENC            DEBUGZONE(5)
#define ZONE_OSD             DEBUGZONE(6)
#define ZONE_CAP             DEBUGZONE(7)
#define ZONE_VDEC            DEBUGZONE(8)
#define ZONE_VOUT            DEBUGZONE(9)
#define ZONE_VIN             DEBUGZONE(10)
#define ZONE_CMD             DEBUGZONE(11)
#define ZONE_ISP             DEBUGZONE(12)
#endif

#ifdef SHIP_BUILD
#define ERRORMSG(cond,printf_exp) ((void)0)
#define RETAILMSG(cond,printf_exp) ((void)0)
#else // SHIP_BUILD
#ifdef DEBUG
#define DEBUGMSG(cond,printf_exp)   \
   ((void)((cond)?(printf printf_exp),1:0))
#define RETAILMSG(cond,printf_exp)   \
   ((cond)?(printf printf_exp),1:0)
#else // RELEASE
#define DEBUGMSG(cond,printf_exp) ((void)0)
#define RETAILMSG(cond,printf_exp)   \
   ((cond)?(printf printf_exp),1:0)
#endif // DEBUG
#endif

typedef unsigned char       u8;         /*!< 8 bit unsigned integer. */
typedef unsigned short      u16;        /*!< 16 bit unsigned integer. */
typedef unsigned int        u32;        /*!< 32 bit unsigned integer. */
typedef unsigned long long  u64;        /*!< 64 bit unsigned integer. */
typedef signed char         s8;         /*!< 8 bit signed integer. */
typedef signed short        s16;        /*!< 16 bit signed integer. */
typedef signed int          s32;        /*!< 32 bit signed integer. */
typedef signed long long    s64;        /*!< 64 bit unsigned integer. */


#define SGKS_VIDEO_RATIO(ratio)               (ratio)
#define SGKS_VIDEO_RATIO_AUTO                 SGKS_VIDEO_RATIO(0)
#define SGKS_VIDEO_RATIO_4_3                  SGKS_VIDEO_RATIO(1)
#define SGKS_VIDEO_RATIO_16_9                 SGKS_VIDEO_RATIO(2)
#define SGKS_VIDEO_RATIO_1_1                  SGKS_VIDEO_RATIO(4)

#define SGKS_VIDEO_BITS(bits)                 (bits)
#define SGKS_VIDEO_BITS_AUTO                  SGKS_VIDEO_BITS(0)
#define SGKS_VIDEO_BITS_8                     SGKS_VIDEO_BITS(8)
#define SGKS_VIDEO_BITS_10                    SGKS_VIDEO_BITS(10)
#define SGKS_VIDEO_BITS_12                    SGKS_VIDEO_BITS(12)
#define SGKS_VIDEO_BITS_14                    SGKS_VIDEO_BITS(14)
#define SGKS_VIDEO_BITS_16                    SGKS_VIDEO_BITS(16)

#define SGKS_VIDEO_FORMAT(format)             (format)
#define SGKS_VIDEO_FORMAT_AUTO                SGKS_VIDEO_FORMAT(0)
#define SGKS_VIDEO_FORMAT_INTERLACE           SGKS_VIDEO_FORMAT(1)
#define SGKS_VIDEO_FORMAT_PROGRESSIVE         SGKS_VIDEO_FORMAT(2)

#define SGKSVOUTSW(bit)                 (1 << (bit))
#define SGKS_VIDEO_VOUT_RESET           SGKSVOUTSW(0)
#define SGKS_VIDEO_VOUT_MIXER           SGKSVOUTSW(1)
#define SGKS_VIDEO_VOUT_SETUP           SGKSVOUTSW(2)
#define SGKS_VIDEO_VOUT_DISPLAY         SGKSVOUTSW(3)
#define SGKS_VIDEO_VOUT_OSD             SGKSVOUTSW(4)
#define SGKS_VIDEO_VOUT_IMG             SGKSVOUTSW(5)
#define SGKS_VIDEO_VOUT_PIP             SGKSVOUTSW(6)
#define SGKS_VIDEO_VOUT_SW              (SGKS_VIDEO_VOUT_RESET|SGKS_VIDEO_VOUT_MIXER|SGKS_VIDEO_VOUT_SETUP \
										|SGKS_VIDEO_VOUT_DISPLAY|SGKS_VIDEO_VOUT_OSD|SGKS_VIDEO_VOUT_IMG \
										|SGKS_VIDEO_VOUT_PIP)

#define ADDR_AUTO    0xFFFFFFFF

typedef enum
{
    //fw init
    dsp_init_param = 0,
    dsp_log,
    fw_orce,
    fw_me,
    fw_binary,

    //dsp config
    dsp2_info,
    dsp_chip,
    dsp_default_cmd,
    dsp_normal_cmd,
    dsp_result_queue,
    bsb_desc_addr,  
    msg_addr_arm_osd,    
    display_vouta_addr,
    display_voutb_addr,
    osd_dram_addr,
    osd_clut_dram_addr,
    tv_dram_addr,
    disp_bank_addr,

    //bsb
    bsb_buff,
    //dsp mem
    dsp_buff,

    isp_aaa_fifo1_base,
    isp_aaa_fifo2_base,
    isp_input_lookup_table,
    isp_matrix_dram_address,
    isp_output_lookup_table,
    isp_chroma_gain_curve,
    isp_hot_pixel_thd_table,
    isp_dark_pixel_thd_table,
    isp_mctf_cfg_addr,
    isp_k0123_table,
    isp_exposure_gain_curve,
    isp_luma_sharpening_alpha_table,
    isp_coeff_fir1_addr,
    isp_coeff_fir2_addr,
    isp_coring_table,
    isp_vignette_r_gain,
    isp_vignette_go_gain,
    isp_vignette_ge_gain,
    isp_vignette_b_gain,
    isp_pixel_map_addr,
    isp_fpn_reg_addr,
    isp_hor_still,
    isp_ver_still,
    isp_eis_enhance_turbo_buf,

	
} sgks_mem_map_id_e;

typedef enum mpi_dram_ptr_type
{
    BUF_CAPTURE = 0,		   /*vin output buffer*/
    BUF_PREVIEW_A_OUT,	   /* PREVIEW_A outputbuffer*/
    BUF_PREVIEW_B_OUT,	  /* PREVIEW_B output buffer*/
    BUF_PREVIEW_C_OUT,	  /* PREVIEW_C output buffer*/
    BUF_ME1_CAPTURE,	  /*Motion estimation, width and height == 1/4 BUF_CAPTURE */
    BUF_ME1_PREVIEW_A,	  /*Motion estimation, width and height == 1/4 BUF_PREVIEW_A_OUT*/
    BUF_ME1_PREVIEW_B,	  /*Motion estimation, width and height == 1/4 BUF_PREVIEW_B_OUT*/
    BUF_ME1_PREVIEW_C,	  /*Motion estimation, width and height == 1/4 BUF_PREVIEW_C_OUT*/

    BUF_PREVIEW_A_SRC, 	  /* PREVIEW_A input buffer*/
    BUF_PREVIEW_B_SRC,    /* PREVIEW_B input buffer*/
    BUF_PREVIEW_C_SRC, 	  /* PREVIEW_C input buffer*/ //10
	
    BUF_CVBS_DMA,		/* CVBS_DMA buffer*/
    
    BUF_STREAM_0_RECON,	/*stream 0 encode buffer,  width and height == encode size*/
    BUF_STREAM_1_RECON, 	/*stream 1 encode buffer,  width and height == encode size*/
    BUF_STREAM_2_RECON,	/*stream 2 encode buffer,  width and height == encode size*/
    BUF_STREAM_3_RECON,	/*stream 3 encode buffer,  width and height == encode size*/
    BUF_STREAM_0_COL, 		/*tream 0 encode buffer,  width=((encdoe width)>>4)*8, height=(encode height)>>4*/
    BUF_STREAM_1_COL, 		/*tream 1 encode buffer,  width=((encdoe width)>>4)*8, height=(encode height)>>4*/
    BUF_STREAM_2_COL,		/*tream 2 encode buffer,  width=((encdoe width)>>4)*8, height=(encode height)>>4*/
    BUF_STREAM_3_COL, 		/*tream 3 encode buffer,  width=((encdoe width)>>4)*8, height=(encode height)>>4*/

    BUF_ENCODE_RELATED,     //20
    
    BUF_STREAM_0_SRC, 		/*stream 0 encode buffer input buffer*/
    BUF_STREAM_1_SRC, 		/*stream 1 encode buffer input buffer*/
    BUF_STREAM_2_SRC, 		/*stream 2 encode buffer input buffer*/
    BUF_STREAM_3_SRC, 		/*stream 3 encode buffer input buffer*/

    BUF_VOUT_SRC_YUV422,   /*vout input buffer*/

    BUF_SCREENNAIL,      //26
    BUF_THUMBNAIL,
    BUF_EXTRA_FRAME,
    BUF_H264_DEC_FRAME,

    BUF_JEPG_DEC_FRAME,  //30
    BUF_JEPG_SPECIAL_VOUT0,
    BUF_JEPG_SPECIAL_VOUT1,
    BUF_JEPG_VOUT0_FRAME,
    BUF_JEPG_VOUT1_FRAME,
    BUF_JEPG_RESCALE_FRAME,
	
    BUF_MAX, //36
} mpi_dram_ptr_type_e;

typedef struct mpi_mem_map_table
{
    sgks_mem_map_id_e   mem_id;
    u32   				phy_addr;
//    u32   				vir_addr;
    u32   				size;
} mpi_mem_map_table_s;



typedef enum __sgks_dsp_op_mode_e {
	SGKS_DSP_ENCODE_MODE		= 0x00, /*--see sgks_encode_state_e---*/
	SGKS_DSP_DECODE_MODE		= 0x01, /*--see sgks_decode_state_e---*/
	SGKS_DSP_RESET_MODE		= 0x02,
	SGKS_DSP_UNKNOWN_MODE		= 0x03
} sgks_dsp_op_mode_e;

typedef enum __sgks_encode_mode_e
{
	SGKS_DSP_STOP_V_ENC        = 0,    /* Nothing (not preview) is running in this mode. */
	SGKS_DSP_ENC_PRE_MODE      = 1,    /* H264 encoding + preview. */
	SGKS_DSP_SJPEG_PRE_MODE    = 2,    /* JPEG capture + preview. */
	SGKS_DSP_MJPEG_PRE_MODE    = 3,    /* Real-time Motion JPEG + preview. */
	SGKS_DSP_3A_DATA_MODE      = 4,    /* Gather 3A data. No preview. */
	SGKS_DSP_ROW_DATA_MODE     = 5,    /* capture raw data from sensor or memory, generate preview, and jpeg encode the main picture and thumbnail. */
	SGKS_DSP_TIMER_MODE         = 6,
	SGKS_DSP_ENC_UNKNOWN_MODE  = 7,
} sgks_encode_mode_e;

typedef enum __sgks_encode_state_e
{
	SGKS_ENC_IDLE_STATE               = 0x00,
	SGKS_ENC_BUSY_STATE               = 0x01,
	SGKS_ENC_PAUSE_STATE              = 0x02,
	SGKS_ENC_FLUSH_STATE              = 0x03,
	SGKS_ENC_RJPEG_STOP_STATE         = 0x04,
	SGKS_ENC_RJPEG_RESUMABLE_STATE    = 0x05,
	SGKS_ENC_UNKNOWN_STATE            = 0xFF,
} sgks_encode_state_e;


typedef enum __sgks_decode_state_e
{
	SGKS_DEC_IDLE_STATE               = 0x00,
	SGKS_DEC_H264DEC_STATE            = 0x01,
	SGKS_DEC_H264DEC_IDLE_STATE       = 0x02,
	SGKS_DEC_TRANSITION_1_TO_0_STATE  = 0x03,
	SGKS_DEC_TRANSITION_1_TO_2_STATE  = 0x04,
	SGKS_DEC_JPEGSTILL_STATE          = 0x05,
	SGKS_DEC_TRANSITION_5_TO_0        = 0x06,
	SGKS_DEC_MULTISCENE_STATE         = 0x07,
	SGKS_DEC_TRANSITION_7_TO_0_STATE  = 0x08,
	SGKS_DEC_UNKNOWN_STATE            = 0x09
} sgks_decode_state_e;

typedef struct __sgks_mpi_dsp_status_s
{
   sgks_dsp_op_mode_e  op_mode;       /*sgks_dsp_op_mode_e*/
   sgks_encode_mode_e  op_sub_mode;  /*sgks_encode_mode_e*/
   u32  state;         /*sgks_encode_state_e, sgks_decode_state_e */
   u16  stream_encode_state[4];
} sgks_mpi_dsp_status_s;

typedef struct __sgks_mpi_mem_s 
{
    u8 *user_addr;
    u8 *phy_addr;
    u32 size;
} sgks_mpi_mem_s;

/*************************************************************************
*                          enum                  *
**************************************************************************/

typedef enum __sgks_stop_dec_type_e
{
	SGKS_STOP_DEC_CLEAN_SCREEN  = 0,
	SGKS_STOP_DEC_NOT_CLEAN_SCREEN
} sgks_stop_dec_type_e;

typedef enum __sgks_vdec_file_type_e
{
	SGKS_VDEC_TYPE_H264   = 0,
	SGKS_VDEC_TYPE_MJPEG
}sgks_vdec_file_type_e;

typedef enum
{
	SGKS_VDEC_PLAY_CONTROL_PAUSE   = 0,
	SGKS_VDEC_PLAY_CONTROL_RESUME,
	SGKS_VDEC_PLAY_CONTROL_STEP,
	SGKS_VDEC_PLAY_CONTROL_TRICK
} SGKS_VDEC_PLAY_CONTROL_E;


typedef enum __sgks_vdec_fw_buffer
{
      VDEC_BUF_SCREENNAIL = 0, 
      VDEC_BUF_THUMBNAIL,
      VDEC_BUF_EXTRA_FRAME,
      VDEC_BUF_H264_DEC_FRAME,

      VDEC_BUF_JEPG_DEC_FRAME, 
      VDEC_BUF_JEPG_SPECIAL_VOUT0,
      VDEC_BUF_JEPG_SPECIAL_VOUT1,
      VDEC_BUF_JEPG_VOUT0_FRAME,
      VDEC_BUF_JEPG_VOUT1_FRAME,
      VDEC_BUF_JEPG_RESCALE_FRAME,
      VDEC_BUF_MAX,
} sgks_vdec_fw_buffer_e;


typedef struct __sgks_mpi_vdec_fw_buffer_info
{
      sgks_mpi_mem_s  fw_dec_mem_info[VDEC_BUF_MAX];
} sgks_mpi_vdec_fw_buffer_info;

typedef struct __sgks_mpi_vdec_file_s
{

	sgks_vdec_file_type_e 	type;
    u32 					width;
	u32 					height;
	
} sgks_mpi_vdec_file_s;

typedef struct __sgks_mpi_vdec_buff_s
{
	u32 buff_ptr_phy;
	u8* buff_ptr_vir;
    u32 size;
}sgks_mpi_vdec_buff_s;

typedef struct __sgks_mpi_vdec_rescale_s
{
	u32 stream_id;
    u32 width;
	u32 height;
} sgks_mpi_vdec_rescale_s;

typedef struct __sgks_mpi_vdec_init_Param_s
{
    sgks_mpi_vdec_file_s		file_info;
	sgks_mpi_vdec_buff_s		vdec_buff;
	u32							rescale_sub_num;
	sgks_mpi_vdec_rescale_s  	rescale_sub[MAX_VDEC_RESCALE_SUB_NUM];
	sgks_mpi_vdec_fw_buffer_info  fw_buff_info;
} sgks_mpi_vdec_init_Param_s;

typedef struct __sgks_mpi_vdec_stop_Param_s
{
    sgks_stop_dec_type_e screen;
} sgks_mpi_vdec_stop_Param_s;

typedef struct __sgks_mpi_vdec_info_s
{
	u32 curr_pts;
	u32 curr_pts_high;
	u32 decoded_frames;
}sgks_mpi_vdec_info_s;


typedef struct dsp_init2_data
{
	u32 prevA_daddr; 
	u32 prevA_size;
	u32 prevB_daddr;
	u32 prevB_size;
	u32 prevC_daddr;
	u32 prevC_size;
	u32 cvbs_buff_daddr;
	u32 cvbs_buff_size;
//
	u32 capture_buff_daddr;
	u32 capture_buff_size;
	u32 me1_buff_daddr;
	u32 me1_buff_size;
	u32 recon_buff_daddr;
	u32 recon_buff_size;
	u32 col_buff_daddr;
	u32 col_buff_size;
//	
	u32 sec_capture_buff_daddr;
	u32 sec_capture_buff_size;
	u32 sec_me1_buff_daddr;
	u32 sec_me1_buff_size;
	u32 sec_recon_buff_daddr;
	u32 sec_recon_buff_size;
	u32 sec_col_buff_daddr;
	u32 sec_col_buff_size;
} dsp_init2_data_s;

typedef struct __sgks_fw_init_param_s
{
    //
    u32 *default_binary_data_ptr_phy;
    u32 *cmd_data_ptr_phy;
    u32 cmd_data_size;
    u32 *result_queue_ptr_phy;
    u32 result_queue_size;
    u32 dram_print_buf_addr_phy;
    u32 cvbs_dma_reg_info;
    u32 msg_addr_arm_osd_phy;
    sgks_dsp_op_mode_e operation_mode;
    u32 *default_config_ptr_phy;
    u32 default_config_size;
    u32 *DSP_buf_ptr_phy;
    u32 DSP_buf_size;
    u32 *dsp2_info_ptr_phy;
    u32 *pjpeg_buf_ptr;
    u32 pjpeg_buf_size;
    u32 *chip_id_ptr_phy;
    u32 reserved_2;
    u32 dram_printf_buf_size;
    u32 mctf_enable:1;
	u32 block_vin:1;
	u32 dsp_osd_enable:1;
	u32 sec2_out_sec5_off:1;
	u32 reserved[12];
	#ifdef MEM_FW_DEBUG
	dsp_init2_data_s dsp_init2_data;
	#endif
	//init param addr

    u32 *dsp_init_data_phy;
    u32 dsp_init_data_size;
    u32 *dsp_init_data_vir;
    u32 *default_binary_data_ptr_vir;
    u32 *cmd_data_ptr_vir;
    u32 *result_queue_ptr_vir;
    u32 *dram_print_buf_addr_vir;
    u32 *msg_addr_arm_osd_vir;
    u32 msg_addr_arm_osd_size;
    u32 *default_config_ptr_vir;
    u32 *DSP_buf_ptr_vir;
    u32 *dsp2_info_ptr_vir;
    u32 *chip_id_ptr_vir;
    u32 *bsb_phy;
    u32 bsb_size;
    u32 bsb_desc_phy;
    u32 bsb_desc_size;    
} sgks_fw_init_param_s;

typedef struct __sgks_fw_init_dram_config_s
{
    u32 buf_start_addr_phy;
    u32 buf_size;
    u8  buf_type;
    u8 buf_num;
    u8 buf_format;
    u16 buf_width;
    u16 buf_height;
}sgks_fw_init_dram_config_s;

typedef struct __dsp_code_addr
{
    u32 *dsp_code_start;
    u32 dsp_code_size;
} dsp_code_addr_s;

typedef struct sgks_fw_version
{
    u8 edition_num;
    u8 edition_subnum;
    u8 edition_thirdnum;
    u8 edition_extraverion;
	u8 year[2];
	u8  month;
	u8  day;
} sgks_fw_version_s;


typedef enum __sgks_mpi_fw_type_e
{
    SGKS_MPI_FW_ENCRYPTION   = 0,
    SGKS_MPI_FW_UNENCRYPTION,
} sgks_mpi_fw_type_e;

typedef struct __sgks_fw_info_s
{
    //fw file
    sgks_mpi_fw_type_e	 fw_type;  /*0:unencrypted 1:encryption*/
    int  				 fw_file_num;
    char 				 fw_path[MAX_FW_FILE_NUM][64];//orce + me + binary

    //fw file load mem addr
    dsp_code_addr_s code_addr[MAX_FW_FILE_NUM];
    u32 reserved[8];
} sgks_fw_info_s;



typedef enum __sgks_mpi_vi_device_interface_e
{
    SGKS_VI_INTERFACE_MIPI     = 0,
    SGKS_VI_INTERFACE_DVP,
    SGKS_VI_INTERFACE_CVBS,

    SGKS_VI_INTERFACE_AUTO = 0xff,
} sgks_mpi_vi_device_interface_e;


typedef enum
{
    SGKS_VO_INTERFACE_MIPI_LCD     = 0x10001,
    SGKS_VO_INTERFACE_RGB_LCD,
    SGKS_VO_INTERFACE_CVBS,
} mpi_vo_device_interface_e;


typedef enum __sgks_mpi_vi_type_e
{
    SGKS_VI_TYPE_VIN     = 0,
    SGKS_VI_TYPE_DMA,

    SGKS_VI_TYPE_AUTO    = 0xff,
} sgks_mpi_vi_type_e;

typedef enum __sgks_mpi_vi_MirrorMode_e
{
    SGKS_MPI_VI_MIRROR_HORRIZONTALLY_VERTICALLY   = 0x00,
    SGKS_MPI_VI_MIRROR_HORRIZONTALLY,
    SGKS_MPI_VI_MIRROR_VERTICALLY,
    SGKS_MPI_VI_MIRROR_NONE,
    SGKS_MPI_VI_MIRROR_AUTO                       = 0xFF,
} sgks_mpi_vi_MirrorMode_e;

typedef enum __sgks_mpi_vi_BayerPattern_e
{
    SGKS_MPI_VI_BAYER_PATTERN_RG       = 0,
    SGKS_MPI_VI_BAYER_PATTERN_BG,
    SGKS_MPI_VI_BAYER_PATTERN_GR,
    SGKS_MPI_VI_BAYER_PATTERN_GB,
    SGKS_MPI_VI_BAYER_PATTERN_AUTO     = 255,
} sgks_mpi_vi_BayerPattern_e;

typedef enum __sgks_mpi_vi_Input_format_e
{
    SGKS_MPI_VI_INPUT_FORMAT_RGB_RAW                  = 0x00,
    SGKS_MPI_VI_INPUT_FORMAT_YUV_422_INTLC,
    SGKS_MPI_VI_INPUT_FORMAT_YUV_422_PROG,
    SGKS_MPI_VI_INPUT_FORMAT_MEM_DIR_420I,
    SGKS_MPI_VI_INPUT_FORMAT_MEM_DIR_420P,
    SGKS_MPI_VI_INPUT_FORMAT_YUV_422_NOISP,

    SGKS_MPI_VI_INPUT_FORMAT_AUTO = 255,
} sgks_mpi_vi_Input_format_e;


typedef struct __sgks_mpi_vi_resolution_data_s
{
    u32 width;
    u32 height;
    u32 fps;
    u32 video_mode;
} sgks_mpi_vi_resolution_data_s;

typedef struct __sgks_mpi_vi_resolution_s
{
    u32 num;
    sgks_mpi_vi_resolution_data_s list[20];
} sgks_mpi_vi_resolution_s;


/*************************************************************************
*                          Struct                                        *
**************************************************************************/

typedef struct __sgks_mpi_vi_deviceInfo_s
{
    u32							    enable;
    u32    						    device_id;
    sgks_mpi_vi_device_interface_e	interface_type;
    sgks_mpi_vi_type_e		        vi_type;
    u32   						    vi_width;
    u32   						    vi_height;
    u32							    vi_fps;    
    u32   						    cap_start_x;
    u32   						    cap_start_y;
    u32   						    cap_width;
    u32   						    cap_height;
    sgks_mpi_vi_MirrorMode_e        mirror_mode;
    sgks_mpi_vi_BayerPattern_e      bayer_pattern;
    u32                             video_mode;
    u32                             dma_prv_en;
    u32                             input_format;
    u8							    device_node[32];
    u32 						    reserved[8];
} sgks_mpi_vi_deviceInfo_s;

typedef struct __sgks_mpi_vi_device_s
{
    u32 vi_device_num;
    sgks_mpi_vi_deviceInfo_s vi_deviceinfo[MAX_VI_NUM];
} sgks_mpi_vi_device_s;

typedef struct __sgks_vo_bg_color_info_s
{
    u8 y;
    u8 cb;
    u8 cr;
}sgks_vo_bg_color_info_s;

typedef enum __sgks_vo_display_input_e
{
    SGKS_VO_INPUT_FROM_MIXER = 0,
    SGKS_VO_INPUT_FROM_SMEM,
}sgks_vo_display_input_e;



#define SGKS_VO_SOURCE_TYPE_TV        (1 << 0)
#define SGKS_VO_SOURCE_TYPE_DIGITAL   (1 << 1)
#define SGKS_VO_SOURCE_TYPE_HDMI      (1 << 2)
#define SGKS_VO_SOURCE_TYPE_MIPI      (1 << 3)
#define SGKS_VO_SOURCE_TYPE_BT1120      (1 << 4)

typedef enum
{
    /*video output channel A.*/
    SGKS_VOUT_A = 0,
    /*video output channel B.*/
    SGKS_VOUT_B = 1,
    /*video output channel number.*/
    SGKS_VOUT_NUMBER,
} sgks_VOUT_ChannelEnumT;

typedef enum __sgks_vo_sink_type_e
{
    SGKS_VO_SINK_TYPE_AUTO    = 0,
    SGKS_VO_SINK_TYPE_CVBS    = ((0 << 16) | SGKS_VO_SOURCE_TYPE_TV),
    SGKS_VO_SINK_TYPE_SVIDEO  = ((1 << 16) | SGKS_VO_SOURCE_TYPE_TV),
    SGKS_VO_SINK_TYPE_YPBPR   = ((2 << 16) | SGKS_VO_SOURCE_TYPE_TV),
    SGKS_VO_SINK_TYPE_HDMI    = ((0 << 16) | SGKS_VO_SOURCE_TYPE_HDMI),
    SGKS_VO_SINK_TYPE_RGB_800_480 = ((0 << 16) | SGKS_VO_SOURCE_TYPE_DIGITAL),
    SGKS_VO_SINK_TYPE_RGB_1280_272 = ((1 << 16) | SGKS_VO_SOURCE_TYPE_DIGITAL),
    SGKS_VO_SINK_TYPE_RGB_320_240_FY23001B = ((2 << 16) | SGKS_VO_SOURCE_TYPE_DIGITAL),
        
    SGKS_VO_SINK_TYPE_MIPI_480_854         = ((0 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_MIPI_1920_480        = ((1 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_MIPI_1280_320        = ((2 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_MIPI_480_1280        = ((3 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_MIPI_400_1280        = ((4 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_MIPI_400_1600        = ((5 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_MIPI_ST7701S_480x640 = ((6 << 16) | SGKS_VO_SOURCE_TYPE_MIPI),
    SGKS_VO_SINK_TYPE_BT1120               = ((0 << 16) | SGKS_VO_SOURCE_TYPE_BT1120),
} sgks_vo_sink_type_e;


typedef struct __sgks_vo_video_size_s
{
    u32 specified;
    u16 vo_width;       //VO width
    u16 vo_height;      //VO height
    u16 video_width;    //Video width
    u16 video_height;   //Video height
}sgks_vo_video_size_s;

typedef struct __sgks_vo_video_offset_s
{
    u32 specified;
    s16 offset_x;
    s16 offset_y;
}sgks_vo_video_offset_s;




typedef enum __sgks_vo_flip_info_e
{
    SGKS_VO_FLIP_NORMAL = 0,
    SGKS_VO_FLIP_HV,
    SGKS_VO_FLIP_HORIZONTAL,
    SGKS_VO_FLIP_VERTICAL,
}sgks_vo_flip_info_e;

typedef enum __sgks_vo_rotate_info_e
{
    SGKS_VO_ROTATE_NORMAL,
    SGKS_VO_ROTATE_90,
}sgks_vo_rotate_info_e;

typedef enum __sgks_vo_flip_rotate_ctrl_e
{
    SGKS_VO_NORMAL_DISPLAY = 0,          /* flip = 0 、 rotate = 0   正常显示  */
    SGKS_VO_RIGHT_ROTATE_90,             /* flip = 0 、 rotate = 1  右旋转90度*/
    SGKS_VO_LEVEL_ROTATE_180,            /* flip = 1 、 rotate = 0  水平旋转180度*/
    SGKS_VO_LEFT_ROTATE_90,              /* flip = 1 、 rotate = 1  左旋转90度*/    
    SGKS_VO_LEVEL_FLIP_180,              /* flip = 2 、 rotate = 0  水平翻转180度*/    
    SGKS_VO_LEFT_ROTATE_90_FLIP_180,     /* flip = 2 、 rotate = 1   左旋转90度+翻转180度*/
    SGKS_VO_LEFT_RIGHT_LEFT_180,         /* flip = 3 、 rotate = 0  左右翻转180度*/
    SGKS_VO_RIGHT_ROTATE_90_FLIP_180,    /* flip = 3 、 rotate = 1   右旋转90度+翻转180度*/
}sgks_vo_flip_rotate_ctrl_e;

typedef struct __sgks_vo_osd_size_s
{
    u16 width;
    u16 height;
}sgks_vo_osd_size_s;

typedef struct __sgks_vo_osd_rescale_s
{
    u32 enable;
    u16 width;
    u16 height;
}sgks_vo_osd_rescale_s;

typedef struct __sgks_vo_osd_offset_s
{
    u32 specified;
    s16 offset_x;
    s16 offset_y;
}sgks_vo_osd_offset_s;

typedef enum __sgks_vo_tailored_info_e
{
    SGKS_VO_OSD_NO_CSC    = 0x01,            //No Software CSC
    SGKS_VO_OSD_AUTO_COPY = 0x02,            //Auto copy to other fb
}sgks_vo_tailored_info_e;


typedef enum __sgks_vo_lcd_mode_info_e
{
    SGKS_VO_LCD_MODE_DISABLE = 0,
    SGKS_VO_LCD_MODE_1COLOR_PER_DOT,
    SGKS_VO_LCD_MODE_3COLORS_PER_DOT,
    SGKS_VO_LCD_MODE_RGB565,
    SGKS_VO_LCD_MODE_3COLORS_DUMMY_PER_DOT,
    SGKS_VO_LCD_MODE_RGB888,
}sgks_vo_lcd_mode_info_e;

typedef enum __sgks_vo_lcd_seq_info_e
{
    SGKS_VO_LCD_SEQ_R0_G1_B2 = 0,
    SGKS_VO_LCD_SEQ_R0_B1_G2,
    SGKS_VO_LCD_SEQ_G0_R1_B2,
    SGKS_VO_LCD_SEQ_G0_B1_R2,
    SGKS_VO_LCD_SEQ_B0_R1_G2,
    SGKS_VO_LCD_SEQ_B0_G1_R2,
}sgks_vo_lcd_seq_info_e;

typedef enum __sgks_vo_lcd_clk_edge_info_e
{
    SGKS_VO_LCD_CLK_RISING_EDGE    = 0,
    SGKS_VO_LCD_CLK_FALLING_EDGE,
}sgks_vo_lcd_clk_edge_info_e;

typedef enum __sgks_vo_lcd_model_e
{
    SGKS_VO_LCD_MODEL_DIGITAL    = 0,
    SGKS_VO_LCD_MODEL_AUO27,
    SGKS_VO_LCD_MODEL_P28K,
    SGKS_VO_LCD_MODEL_TPO489,
    SGKS_VO_LCD_MODEL_TPO648,
    SGKS_VO_LCD_MODEL_TD043,
    SGKS_VO_LCD_MODEL_WDF2440,
    SGKS_VO_LCD_MODEL_1P3831,
    SGKS_VO_LCD_MODEL_1P3828,
    SGKS_VO_LCD_MODEL_EJ080NA,
    SGKS_VO_LCD_MODEL_AT070TNA2,
    SGKS_VO_LCD_MODEL_E330QHD,
    SGKS_VO_LCD_MODEL_PPGA3,
}sgks_vo_lcd_model_e;


typedef struct __sgks_vo_lcd_info_s
{
    sgks_vo_lcd_mode_info_e        mode;
    sgks_vo_lcd_seq_info_e         seqt;
    sgks_vo_lcd_seq_info_e         seqb;
    sgks_vo_lcd_clk_edge_info_e    dclk_edge;
    u32                             dclk_freq_hz;    /* PLL_CLK_XXX */
    sgks_vo_lcd_model_e           model;
}sgks_vo_lcd_info_s;


typedef enum __sgks_vo_hdmi_color_space_e
{
    SGKS_VO_HDMI_CS_AUTO = 0,
    SGKS_VO_HDMI_CS_RGB,
    SGKS_VO_HDMI_CS_YCBCR_444,
    SGKS_VO_HDMI_CS_YCBCR_422,
}sgks_vo_hdmi_color_space_e;

typedef enum __sgks_vo_ddd_structure_e
{
    DDD_FRAME_PACKING           = 0,
    DDD_FIELD_ALTERNATIVE       = 1,
    DDD_LINE_ALTERNATIVE        = 2,
    DDD_SIDE_BY_SIDE_FULL       = 3,
    DDD_L_DEPTH                 = 4,
    DDD_L_DEPTH_GRAPHICS_DEPTH  = 5,
    DDD_TOP_AND_BOTTOM          = 6,
    DDD_RESERVED                = 7,

    DDD_SIDE_BY_SIDE_HALF       = 8,

    DDD_UNSUPPORTED             = 16,
} sgks_vo_ddd_structure_e;

typedef enum __sgks_vo_hdmi_overscan_e
{
    SGKS_VO_HDMI_OVERSCAN_AUTO = 0,
    SGKS_VO_HDMI_NON_FORCE_OVERSCAN,
    SGKS_VO_HDMI_FORCE_OVERSCAN,
} sgks_vo_hdmi_overscan_e;

typedef enum __sgks_vo_src_e
{
    SGKS_VO_SRC_DEFAULT_IMG   = 0,
    SGKS_VO_SRC_BACKGROUND    = 1,
    SGKS_VO_SRC_ENC           = 2,
    SGKS_VO_SRC_DEC           = 3,
    SGKS_VO_SRC_H264_DEC      = 3,
    SGKS_VO_SRC_MPEG2_DEC     = 5,
    SGKS_VO_SRC_MPEG4_DEC     = 6,
    SGKS_VO_SRC_MIXER_A       = 7,
    SGKS_VO_SRC_VCAP          = 8,
    SGKS_VO_SRC_YUV422        = 10,
} sgks_vo_src_e;

typedef enum __sgks_vo_src_yuv_e
{
    SGKS_VO_SRC_TYPE_YUV420   = 0,
    SGKS_VO_SRC_TYPE_YUV422   = 1,
} sgks_vo_src_yuv_e;



typedef enum
{
    SGKS_VO_PREVIEW_A = 0,
    SGKS_VO_PREVIEW_B,
    SGKS_VO_PREVIEW_C
} sgks_vo_preview_type_e;

typedef enum
{
    SGKS_PREVIEW_SRC_TYPE_SENSOR  = 0,
    SGKS_PREVIEW_SRC_TYPE_CVBS    = 1,
    SGKS_PREVIEW_SRC_TYPE_MIPI    = 2,
    SGKS_PREVIEW_SRC_TYPE_LVDS    = 3,
} sgks_preview_src_type_e;

typedef enum
{
    SGKS_PREVIEW_TYPE_OFF     = 0, /* Preview unit is not used */
    SGKS_PREVIEW_TYPE_SD      = 1, /* Preview unit used to generate SD Preview for VO */
    SGKS_PREVIEW_TYPE_HD      = 2, /* Preview unit used to generate HD Preview for VO */
    SGKS_PREVIEW_TYPE_CAPTURE = 3, /* Preview unit used to generate capture buffers for encoding */
	SGKS_PREVIEW_TYPE_ARM	  = 4, /* Preview unit used to ARM (now don't support)*/
    SGKS_PREVIEW_TYPE_MAX     = 5
} sgks_preview_type_e;


typedef struct __sgks_preview_info
{
    u32						 enable;
    sgks_preview_type_e      pre_type;
    sgks_preview_src_type_e  pre_src;
    u32 					 format;
    u32						 fps;
    u32                      max_output_width;
    u32                      max_output_height;
    u32                      output_width;
    u32                      output_height;
} sgks_preview_info_s;

typedef struct __sgks_preview_config
{
    sgks_preview_info_s  preview_info[MAX_PREVIEW_NUM];
} sgks_preview_config_s;

typedef struct __sgks_preview_change
{
    u8  preview_change_enable[MAX_PREVIEW_NUM];
    sgks_preview_type_e	preview_change_type[MAX_PREVIEW_NUM];//now only support SGKS_PREVIEW_TYPE_OFF,SD,HD,the others need change dsp buffer
} sgks_preview_change_s;


typedef struct __sgks_isp_initParam_s
{
    u32	 enable;
    u32* isp_aaa_fifo1_base_phy;
	u32* isp_aaa_fifo1_base_vir;
    u32  isp_aaa_fifo1_base_size;

    u32* isp_aaa_fifo2_base_phy;
	u32* isp_aaa_fifo2_base_vir;
    u32  isp_aaa_fifo2_base_size;

    u32* isp_input_lookup_table_phy;
    u32* isp_input_lookup_table_vir;
    u32  isp_input_lookup_table_size;

    u32* isp_matrix_dram_address_phy;
    u32* isp_matrix_dram_address_vir;
    u32  isp_matrix_dram_address_size;

    u32* isp_output_lookup_table_phy;
    u32* isp_output_lookup_table_vir;
    u32  isp_output_lookup_table_size;

    u32* isp_chroma_gain_curve_phy;
    u32* isp_chroma_gain_curve_vir;
    u32  isp_chroma_gain_curve_size;

    u32* isp_hot_pixel_thd_table_phy;
    u32* isp_hot_pixel_thd_table_vir;
    u32  isp_hot_pixel_thd_table_size;

    u32* isp_dark_pixel_thd_table_phy;
    u32* isp_dark_pixel_thd_table_vir;
    u32  isp_dark_pixel_thd_table_size;

    u32* isp_mctf_cfg_addr_phy;
    u32* isp_mctf_cfg_addr_vir;
    u32  isp_mctf_cfg_addr_size;

    u32* isp_k0123_table_phy;
    u32* isp_k0123_table_vir;
    u32  isp_k0123_table_size;

    u32* isp_exposure_gain_curve_phy;
    u32* isp_exposure_gain_curve_vir;
    u32  isp_exposure_gain_curve_size;

    u32* isp_luma_sharpening_alpha_table_phy;
    u32* isp_luma_sharpening_alpha_table_vir;
    u32  isp_luma_sharpening_alpha_table_size;

    u32* isp_coeff_fir1_addr_phy;
    u32* isp_coeff_fir1_addr_vir;
    u32  isp_coeff_fir1_addr_size;

    u32* isp_coeff_fir2_addr_phy;
    u32* isp_coeff_fir2_addr_vir;
    u32  isp_coeff_fir2_addr_size;

    u32* isp_coring_table_phy;
    u32* isp_coring_table_vir;
    u32  isp_coring_table_size;

    u32* isp_vignette_r_gain_phy;
    u32* isp_vignette_r_gain_vir;
    u32  isp_vignette_r_gain_size;

    u32* isp_vignette_go_gain_phy;
    u32* isp_vignette_go_gain_vir;
    u32  isp_vignette_go_gain_size;

    u32* isp_vignette_ge_gain_phy;
    u32* isp_vignette_ge_gain_vir;
    u32  isp_vignette_ge_gain_size;

    u32* isp_vignette_b_gain_phy;
    u32* isp_vignette_b_gain_vir;
    u32  isp_vignette_b_gain_size;

    u32* isp_pixel_map_addr_phy;
    u32* isp_pixel_map_addr_vir;
    u32  isp_pixel_map_addr_size;

    u32* isp_fpn_reg_addr_phy;
    u32* isp_fpn_reg_addr_vir;
    u32  isp_fpn_reg_addr_size;

    u32* isp_hor_still_phy;
    u32* isp_hor_still_vir;
    u32  isp_hor_still_size;

    u32* isp_ver_still_phy;
    u32* isp_ver_still_vir;
    u32  isp_ver_still_size;

    u32* isp_eis_enhance_turbo_buf_phy;
    u32* isp_eis_enhance_turbo_buf_vir;
    u32  isp_eis_enhance_turbo_buf_size;

} sgks_isp_initParam_s;


typedef struct __sgks_vio_src_video_info_s

{
    u32 width;
    u32 height;
    u32 fps;
    u8  format;
    u8  type;
    u8  bits;
    u8  ratio;
    u8  system;
    u8  flip;
    u8  rotate;
    u16 rev;
} sgks_vio_src_video_info_s;


typedef enum
{
    SGKS_MDI_IMG_CONFIG_STATISTICS = 0,
    SGKS_MDI_IMG_GET_STATISTICS,
    SGKS_MDI_IMG_NOISE_FILTER_SETUP,
    SGKS_MDI_IMG_BLACK_LEVEL_GLOBAL_OFFSET,
    SGKS_MDI_IMG_BAD_PIXEL_CORRECTION,
    SGKS_MDI_IMG_CFA_LEAKAGE_FILTER_SETUP,
    SGKS_MDI_IMG_CFA_NOISE_FILTER_SETUP,
    SGKS_MDI_IMG_RGB_GAIN_ADJUST,
    SGKS_MDI_IMG_VIGNETTE_COMPENSATION,
    SGKS_MDI_IMG_LOCAL_EXPOSURE,
    SGKS_MDI_IMG_COLOR_CORRECTION,
    SGKS_MDI_IMG_RGB_TO_YUV_SETUP,
    SGKS_MDI_IMG_CHROMA_SCALE,
    SGKS_MDI_IMG_CHROMA_MEDIAN_FILTER_SETUP,
    SGKS_MDI_IMG_LUMA_SHARPENING,
    SGKS_MDI_IMG_LUMA_SHARPENING_FIR_CONFIG,
    SGKS_MDI_IMG_LUMA_SHARPENING_EDGE_CONTROL,
    SGKS_MDI_IMG_LUMA_SHARPENING_BLEND_CONFIG,
    SGKS_MDI_IMG_LUMA_SHARPENING_LEVEL_CONTROL,
    SGKS_MDI_IMG_LUMA_SHARPENING_MISC,
    SGKS_MDI_IMG_MCTF_GMV_SETUP,
    SGKS_MDI_IMG_MCTF_MV_STABILIZER_SETUP,
    SGKS_MDI_IMG_AAA_SETUP_EX,
    SGKS_MDI_IMG_ANTI_ALIASING_CONFIG,
    SGKS_MDI_IMG_DIGITAL_SATURATION_LEVEL,
    SGKS_MDI_IMG_SET_ZOOM_FACTOR,
    SGKS_MDI_IMG_DEMOSAIC_CONIFG,
    SGKS_MDI_IMG_SENSOR_CONFIG,
    SGKS_MDI_IMG_STATIC_BAD_PIXEL_CORRECTION,
    SGKS_MDI_IMG_SET_WARP_CTRL,
    SGKS_MDI_IMG_CANCEL_GETTING_STATISTICS,
    SGKS_MDI_IMG_START_STILL_CAPTURE,
    SGKS_MDI_IMG_STILL_CAPTURE_ADV,
    SGKS_MDI_IMG_STILL_PROC_FROM_MEMORY,
    SGKS_MDI_IMG_READ_RAW_INFO,
    SGKS_MDI_IMG_JPEG_ENC_SETUP,
    SGKS_MDI_IMG_INTERVAL_CAP,
    SGKS_MDI_IMG_STLL_LOW_ISO_ADDR,
    SGKS_MDI_IMG_STLL_HIGH_ISO_ADDR,
    SGKS_MDI_IMG_DUMP_LOW_ISO_BINS,
    SGKS_MDI_IMG_DUMP_HI_ISO_BINS,
    SGKS_MDI_IMG_SET_AAA_STATIC_FREQ,

} SGKS_MDI_IMAGE_E;


#define SGKS_DSP_VIDEO_FORMAT(format)         (format)
#define SGKS_DSP_VIDEO_FORMAT_PROGRESSIVE     SGKS_DSP_VIDEO_FORMAT(0)
#define SGKS_DSP_VIDEO_FORMAT_INTERLACE       SGKS_DSP_VIDEO_FORMAT(1)
#define SGKS_DSP_VIDEO_FORMAT_DEF_PROGRESSIVE SGKS_DSP_VIDEO_FORMAT(2)
#define SGKS_DSP_VIDEO_FORMAT_DEF_INTERLACE   SGKS_DSP_VIDEO_FORMAT(3)
#define SGKS_DSP_VIDEO_FORMAT_TOP_PROGRESSIVE SGKS_DSP_VIDEO_FORMAT(4)
#define SGKS_DSP_VIDEO_FORMAT_BOT_PROGRESSIVE SGKS_DSP_VIDEO_FORMAT(5)
#define SGKS_DSP_VIDEO_FORMAT_NO_VIDEO        SGKS_DSP_VIDEO_FORMAT(6)

#define SGKS_DSP_VIDEO_FPS(format)            (format)
#define SGKS_DSP_VIDEO_FPS_29_97              SGKS_DSP_VIDEO_FPS(0)
#define SGKS_DSP_VIDEO_FPS_59_94              SGKS_DSP_VIDEO_FPS(1)
#define SGKS_DSP_VIDEO_FPS_23_976             SGKS_DSP_VIDEO_FPS(2)
#define SGKS_DSP_VIDEO_FPS_12_5               SGKS_DSP_VIDEO_FPS(3)
#define SGKS_DSP_VIDEO_FPS_6_25               SGKS_DSP_VIDEO_FPS(4)
#define SGKS_DSP_VIDEO_FPS_3_125              SGKS_DSP_VIDEO_FPS(5)
#define SGKS_DSP_VIDEO_FPS_7_5                SGKS_DSP_VIDEO_FPS(6)
#define SGKS_DSP_VIDEO_FPS_3_75               SGKS_DSP_VIDEO_FPS(7)
#define SGKS_DSP_VIDEO_FPS_15                 SGKS_DSP_VIDEO_FPS(15)
#define SGKS_DSP_VIDEO_FPS_24                 SGKS_DSP_VIDEO_FPS(24)
#define SGKS_DSP_VIDEO_FPS_25                 SGKS_DSP_VIDEO_FPS(25)
#define SGKS_DSP_VIDEO_FPS_30                 SGKS_DSP_VIDEO_FPS(30)
#define SGKS_DSP_VIDEO_FPS_50                 SGKS_DSP_VIDEO_FPS(50)
#define SGKS_DSP_VIDEO_FPS_60                 SGKS_DSP_VIDEO_FPS(60)
#define SGKS_DSP_VIDEO_FPS_120                SGKS_DSP_VIDEO_FPS(120)


typedef enum
{
    /*use default sensor frame rate.*/
    SGKS_VI_FPS_AUTO   = 0,
    /*fps:1.*/
    SGKS_VI_FPS_1      = 1,
    SGKS_VI_FPS_2      = 2,
    SGKS_VI_FPS_3      = 3,
    SGKS_VI_FPS_4      = 4,
    SGKS_VI_FPS_5      = 5,
    SGKS_VI_FPS_6      = 6,
    SGKS_VI_FPS_10     = 10,
    SGKS_VI_FPS_12     = 12,
    SGKS_VI_FPS_13     = 13,
    SGKS_VI_FPS_14     = 14,
    SGKS_VI_FPS_15     = 15,
    SGKS_VI_FPS_20     = 20,
    SGKS_VI_FPS_24     = 24,
    SGKS_VI_FPS_25     = 25,
    SGKS_VI_FPS_30     = 30,
    SGKS_VI_FPS_50     = 50,
    SGKS_VI_FPS_60     = 60,
    SGKS_VI_FPS_120    = 120,
    /*fps:3.125.*/
    SGKS_VI_FPS_3_125  = 3125000,
    /*fps:3.75.*/
    SGKS_VI_FPS_3_75   = 37500,
    /*fps:6.25.*/
    SGKS_VI_FPS_6_25   = 62500,
    /*fps:7.5.*/
    SGKS_VI_FPS_7_5    = 750,
    /*fps:12.5.*/
    SGKS_VI_FPS_12_5   = 1250,
    /*fps:23.976.*/
    SGKS_VI_FPS_23_976 = 23976000,
    /*fps:29.97.*/
    SGKS_VI_FPS_29_97  = 299700,
    /*fps:59.94.*/
    SGKS_VI_FPS_59_94  = 599400,
} SGKS_VI_FrameRateEnumT;


typedef struct __sgks_vo_deviceInfo_s
{
    int                            id;             // Sink ID   0  lcd  1 cvbs
    u32                            mode;           // SGKS_VIDEO_MODE_E
    u32                            ratio;          // SGKS_VIDEO_RATIO
    u32                            bits;           // SGKS_VIDEO_BITS
    u32                            type;           // SGKS_VIDEO_TYPE
    u32                            format;         // SGKS_VIDEO_FORMAT
    u32                            frame_rate;     // SGKS_VIDEO_FPS
    int                            csc_en;         // enable csc or not
    sgks_vo_bg_color_info_s  		   bg_color;
    sgks_vo_display_input_e    display_input;    // input from SMEM or Mixer
    sgks_vo_sink_type_e            sink_type;


    /* Video */
    sgks_vio_src_video_info_s      video_info;
    int                            video_en;       // enable video or not
    sgks_vo_video_size_s     video_size;     // video size   mpi
    sgks_vo_video_offset_s   video_offset;   // video offset   mpi
    sgks_vo_flip_rotate_ctrl_e video_rotate_flip_ctrl;   // rotate  flip

    /* OSD */
    int                            fb_id;          // frame buffer id
    sgks_vo_osd_size_s       osd_size;       // OSD size
    sgks_vo_osd_rescale_s    osd_rescale;    // OSD rescale
    sgks_vo_osd_offset_s     osd_offset;     // OSD offset
    sgks_vo_flip_info_e        osd_flip;       // flip
    sgks_vo_rotate_info_e      osd_rotate;     // rotate
    sgks_vo_tailored_info_e    osd_tailor;     // no csc, auto copy

    /* Misc */
    u32                             direct_to_dsp;      // bypass media
    sgks_vo_lcd_info_s        lcd_cfg;            // LCD only
    sgks_vo_hdmi_color_space_e   hdmi_color_space;   // HDMI only
    sgks_vo_ddd_structure_e                 hdmi_3d_structure;  // HDMI only
    sgks_vo_hdmi_overscan_e         hdmi_overscan;      // HDMI only

    u32 						   reserved[8];

    /*vo source config*/
    sgks_vo_src_e                  vo_src;
    sgks_vo_src_yuv_e              yuv_type;       //only external data
    int                            yuv_width;      //only external data
    int                            yuv_height;     //only external data
    int                            yuv_offset_x;      //only external data
    int                            yuv_offset_y;     //only external data

    ///////////////////////////////////

    u8				            device_node[32];

    u8							*disp_config_dram_addr_phy;
    u8							*disp_config_dram_addr_vir;
    u32							disp_config_dram_addr_size;


    u8							*tv_config_dram_addr_phy;
    u8							*tv_config_dram_addr_vir;
    u32                         tv_config_dram_addr_size;

    u8							*dram_clut_phy;
    u8							*dram_clut_vir;
    u32                         dram_clut_size;

    u8							*dram_osd_phy;
    u8							*dram_osd_vir;
    u32                         dram_osd_size;

    u8							*disp_bank_phy;
    u8							*disp_bank_vir;
    u32                         disp_bank_size;



} sgks_vo_deviceInfo_s;

typedef struct __sgks_mpi_init_s
{
    sgks_fw_init_param_s   dsp_init_param;
    sgks_fw_info_s 	       fw_info;
    sgks_fw_init_dram_config_s  dsp_dram_config[BUF_MAX];	
    u32 reserved[8];
} sgks_mpi_init_s;

typedef struct __sgks_mpi_vo_device_s
{
    u32 vo_device_num;
    sgks_vo_deviceInfo_s vo_deviceinfo[MAX_VO_NUM];
} sgks_mpi_vo_device_s;

typedef enum __sgks_mpi_osd_voDirectMode_e
{
    SGKS_MPI_OSD_VO_DIRECT_MODE_565_UYV      = 0,
    SGKS_MPI_OSD_VO_DIRECT_MODE_4444_AYUV    = 1,
    SGKS_MPI_OSD_VO_DIRECT_MODE_1555_AYUV    = 2,
    SGKS_MPI_OSD_VO_DIRECT_MODE_1555_YUV     = 3,
    SGKS_MPI_OSD_VO_DIRECT_MODE_565_YUV      = 4,
    SGKS_MPI_OSD_VO_RESERVED,
} sgks_mpi_osd_voDirectMode_e;

typedef enum sgks_mpi_osd_voSrc_e
{
    SGKS_MPI_OSD_VO_SRC_MAPPED_IN    = 0,
    SGKS_MPI_OSD_VO_DIRECT_IN    = 1,
} sgks_mpi_osd_voSrc_e;

typedef enum
{
    SGKS_MPI_VO_TYPE_LCD,
    SGKS_MPI_VO_TYPE_CVBS,
} sgks_mpi_vo_dev_type_e;

typedef struct __sgks_mpi_vo_ImgParams
{
    u16 yuv_type;
    u16 yuv_width;
    u16 yuv_height;
    u16 video_offset_x;
    u16 video_offset_y;
} sgks_mpi_vo_ImgParams_s;

typedef struct __sgks_mpi_vo_DisBankParams
{
    u16 banknum;
    u32 *bankaddr;
    u32 banksize;
} sgks_mpi_vo_DisBankParams_s;

typedef struct __sgks_mpi_vo_VdecImgInfo
{
    int width;
    int height;
} sgks_mpi_vo_VdecImgInfo_s;

typedef struct __sgks_mpi_osd_voParamSetup_s
{
	u8 vo_id;
    u8  en;
	u8  flip; 
	u16 winWidth;
	u16 winHeight;
	u16 winOffsetX;
	u16 winOffsetY;
	u8  rescalerEn;
	u16 rescalerInputWidth;
	u16 rescalerInputHeight;
    sgks_mpi_osd_voSrc_e src;
	sgks_mpi_osd_voDirectMode_e  directMode;
}sgks_mpi_osd_voParamSetup_s;

/*********************VENC******************/
typedef enum __sgks_mpi_cap_yuv_source_type_e
{
	SGKS_CAP_YUV_SOURCE_TYPE_CVBS_DMA = 1,
	SGKS_CAP_YUV_SOURCE_TYPE_VIN,
    SGKS_CAP_YUV_SOURCE_TYPE_VIN_RESCALE,
    SGKS_CAP_YUV_SOURCE_TYPE_CVBS_RESCALE,
	SGKS_CAP_YUV_SOURCE_TYPE_VDEC_RAW,
	SGKS_CAP_YUV_SOURCE_TYPE_VDEC_SCREEN,
	SGKS_CAP_YUV_SOURCE_TYPE_VDEC_RESCALE,
	SGKS_CAP_YUV_SOURCE_TYPE_NUM,
} sgks_mpi_cap_yuv_source_type_e;

typedef enum __sgks_cap_yuv_semiplanar_e
{
	SGKS_CAP_YUV_SEMIPLANAR_420 = 0, 
	SGKS_CAP_YUV_SEMIPLANAR_422, 
}sgks_cap_yuv_semiplanar_e;

typedef struct __sgks_mpi_cap_yuv_info_s
{
	sgks_cap_yuv_semiplanar_e 		frameFormat;           /*format of caputre frame*/
	u32    							width;                 /*width of caputre frame*/
	u32    							height;                /*height of caputre frame*/
	u8     							*yAddr;                /*Y address of caputre frame*/
	u8     							*uvAddr;               /*UV address of caputre frame, the UV data is interleaved*/
	u32    							stride;                /*stride of caputre frame*/
	sgks_mpi_cap_yuv_source_type_e 	source_type;		   /*yuv data source,1:dvp(cvbs_dma),2:mipi sensor(isp dma) 3 vdec main 4 vdec SCREEN 5 vdec scale*/
	u8     						    addr_type;             /*yuv data addr,0:virt addr (cvbs_dma),1: phy addr*/
}sgks_mpi_cap_yuv_info_s;

typedef struct __sgks_mpi_cap_yuv_rescale_s
{
	//only SGKS_CAP_YUV_SOURCE_TYPE_MIPI_RESCALE, SGKS_CAP_YUV_SOURCE_TYPE_VDEC_RESCALE
    sgks_cap_yuv_semiplanar_e  yuv_type;  
	int width;
    int height;
}sgks_mpi_cap_yuv_rescale_s;

typedef struct __sgks_mpi_cap_init_param_s
{
	u32 cap_type;
	sgks_mpi_mem_s yuv_mem_info[SGKS_CAP_YUV_SOURCE_TYPE_NUM];
	sgks_mpi_cap_yuv_rescale_s yuv_rescale;
}sgks_mpi_cap_init_param_s;


typedef enum
{
    SGKS_ENCODE_NONE,
    SGKS_ENCODE_H264,
    SGKS_ENCODE_MJPEG,
} SGKS_ENCODE_TYPE;

typedef enum __sgks_mpi_venc_channel_type_e
{
    /*channel disabled*/
    VENC_CHANNEL_TYPE_OFF = 0,
    /*channel for encoding*/
    VENC_CHANNEL_TYPE_ENCODE = 1,
    /*channel for preview(CVBS output)*/
    VENC_CHANNEL_TYPE_PREVIEW = 2,
} sgks_mpi_venc_channel_type_e;

    typedef enum __sgks_mpi_venc_stream_num_e
    {
        VENC_STREAM_FIRST = 0,
        VENC_STREAM_SECOND,
        VENC_STREAM_THIRD,
        VENC_STREAM_FORTH,
        VENC_STREAM_NUM,
    }sgks_mpi_venc_stream_num_e;

    typedef enum __sgks_mpi_venc_param_type_e
    {
        VENC_PARAM_TYPE_BITRATE = 0,
        VENC_PARAM_TYPE_FRAMERATE,
        VENC_PARAM_TYPE_CHANNEL,
        VENC_PARAM_TYPE_STREAM,
        VENC_PARAM_TYPE_NUM
    } sgks_mpi_venc_param_type_e;

    typedef enum __sgks_mpi_venc_config_type_e
    {
        VENC_CONFIG_TYPE_H264 = 0,
        VENC_CONFIG_TYPE_MJPEG,
    } sgks_mpi_venc_config_type_e;


    typedef enum __sgks_mpi_venc_state_control_type_e
    {
        VENC_STATE_CONTROL_START = 0,
        VENC_STATE_CONTROL_STOP,
    } sgks_mpi_venc_state_control_type_e;

    typedef enum __sgks_mpi_venc_framerate_e
    {
        VENC_FPS_AUTO   = 0,     /*use default sensor frame rate.*/
        VENC_FPS_1      = 1,         /*fps:1.*/
        VENC_FPS_2      = 2,
        VENC_FPS_3      = 3,
        VENC_FPS_4      = 4,
        VENC_FPS_5      = 5,
        VENC_FPS_6      = 6,
        VENC_FPS_7      = 7,
        VENC_FPS_8      = 8,
        VENC_FPS_9      = 9,
        VENC_FPS_10     = 10,
        VENC_FPS_11     = 11,
        VENC_FPS_12     = 12,
        VENC_FPS_13     = 13,
        VENC_FPS_14     = 14,
        VENC_FPS_15     = 15,
        VENC_FPS_16     = 16,
        VENC_FPS_17     = 17,
        VENC_FPS_18     = 18,
        VENC_FPS_19     = 19,
        VENC_FPS_20     = 20,
        VENC_FPS_21     = 21,
        VENC_FPS_22     = 22,
        VENC_FPS_23     = 23,
        VENC_FPS_24     = 24,
        VENC_FPS_25     = 25,
        VENC_FPS_26     = 26,
        VENC_FPS_27     = 27,
        VENC_FPS_28     = 28,
        VENC_FPS_29     = 29,
        VENC_FPS_30     = 30,
        VENC_FPS_50     = 50,
        VENC_FPS_60     = 60,
        VENC_FPS_120    = 120,
        VENC_FPS_3_125  = 3125000,     /*fps:3.125.*/
        VENC_FPS_3_75   = 37500,       /*fps:3.75.*/
        VENC_FPS_6_25   = 62500,       /*fps:6.25.*/
        VENC_FPS_7_5    = 750,         /*fps:7.5.*/
        VENC_FPS_12_5   = 1250,        /*fps:12.5.*/
        VENC_FPS_23_976 = 23976000,    /*fps:23.976.*/
        VENC_FPS_29_97  = 299700,      /*fps:29.97.*/
        VENC_FPS_59_94  = 599400,      /*fps:59.94.*/
    } sgks_mpi_venc_framerate_e;

    typedef struct __sgks_mpi_venc_framerate_param_s
    {
        sgks_mpi_venc_stream_num_e  streamId; /*stream index for setting the bitrate.*/
        sgks_mpi_venc_framerate_e    fps;/*encoding stream target frame rate value.*/
    } sgks_mpi_venc_framerate_param_s;

    typedef struct __sgks_mpi_venc_channel_param_s
    {
        /*first channel(capture buffer) type 0:disable, 1:for encoding, 2:for preview.*/
        u32            chan1Type;
        /*first channel buffer width.*/
        u16            chan1Width;
        /*first channel buffer height.*/
        u16            chan1Height;
        /*deintlc mode of first channel buffer.*/
        u16            chan1DeintlcForIntlcVin;
        /*second channel type.*/
        u32            chan2Type;
        /*second channel buffer width.*/
        u16            chan2Width;
        /*second channel buffer height.*/
        u16            chan2Height;
        /*deintlc mode of second channel buffer.*/
        u16            chan2DeintlcForIntlcVin;
        /*third channel type.*/
        u32            chan3Type;
        /*third channel buffer width.*/
        u16            chan3Width;
        /*third channel buffer height.*/
        u16            chan3Height;
        /*deintlc mode of third channel buffer.*/
        u32            chan3DeintlcForIntlcVin;
        /*fourth channel type.*/
        u32            chan4Type;
        /*fourth channel buffer width.*/
        u16            chan4Width;
        /*third channel buffer height.*/
        u16            chan4Height;
        /*deintlc mode of fourth channel buffer.*/
        u16            chan4DeintlcForIntlcVin;
        /*0: OFF  1: use progressive VIN to encode interlaced video*/
        u8             intlcScan;
    } sgks_mpi_venc_channel_param_s;

    typedef struct __sgks_mpi_venc_stream_format_param_s
    {
        /*stream index.*/
        sgks_mpi_venc_stream_num_e       streamId;
        /*0: none, 1: H.264, 2: MJPEG*/
        SGKS_ENCODE_TYPE                     encodeType;
        /*channel index:GADI_VENC_ChannelEnumT.*/
        u8                     channelId;
        /*rotate: 0x01:means flip horizontal,0x02:means flip vertical, 0x04:means rotate 90.*/
        u8                     flipRotate;
        /*encode width.*/
        u16                    width;
        /*encode height.*/
        u16                    height;
        /*encode x offset.*/
        u16                    xOffset;
        /*encode y offset.*/
        u16                    yOffset;
        /*encode frame rate.*/
        sgks_mpi_venc_framerate_e    fps;
        /*encode video keep aspect ratio.*/
        /*vi width scale to video width ratio not equal video height ratio.*/
        /*keep aspect ration means use the small aspect ratio.*/
        /* 1: means keep aspect ration, 0: means do not keep.*/
        u8                     keepAspRat;
    } sgks_mpi_venc_stream_format_param_s;

    typedef struct __sgsk_mpi_venc_h264_config_s
    {
        /*stream index.*/
        sgks_mpi_venc_stream_num_e       streamId;
        /*gop M value.*/
        u8             gopM;
        /*gop N value.*/
        u8             gopN;
        /*IDR interval .*/
        u8             idrInterval;
        /*gop model.*/
        u8             gopModel;
        /*encode profile.*/
        u8             profile;
        /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: GADI_VENC_BrcModeEnumT*/
        u8             brcMode;
        /*cbr mode, bit rate.*/
        u32            cbrAvgBps;
        /*vbr mode, min bit rate.*/
        u32            vbrMinbps;
        /*vbr mode, max bit rate.*/
        u32            vbrMaxbps;
        /*picure quality 0,3,0: poor, 3: best*/
        u8             adaptQp;
        /*picure quality consistency ,0: poor ,3 best*/
        u8             qcon;
        /*rate control factor. value:0~51, qpMinI <= qpMinP.*/
        u8             qpMinI;
        /*rate control factor. value:0~51, qpMinI <= qpMinP.*/
        u8             qpMinP;
        /*rate control factor: I frame qp weight, range: 1~10,*/
        u8             qpIWeight;
        /*rate control factor. P frame qp weight, range: 1~5.*/
        u8             qpPWeight;
    } sgks_mpi_venc_h264_config_s;

    typedef struct __sgks_mpi_venc_mjpeg_config_s
    {
        /*stream index.*/
        sgks_mpi_venc_stream_num_e       streamId;
        /*0: YUV 422, 1: YUV 420.*/
        u8             chromaFormat;
        /*1 ~ 100, 100 is best quality.*/
        u8             quality;
    } sgks_mpi_venc_mjpeg_config_s;

    typedef struct __sgks_mpi_venc_stream_s
    {
        sgks_mpi_venc_stream_num_e    stream_id;
        u32    size;
        u32    frame_num;
        s8     pic_type;
        u32    PTS;
        u8     *addr;
        u32    stream_end : 1;
        u32    stream_buffer_start_addr;
        u32    stream_buffer_size;
    } sgks_mpi_venc_stream_s;


    typedef struct __sgks_mpi_venc_enc_buf_s
    {
        u8   *addr;  /*video encode bit stream buffer start address.*/
        u32  length; /*video encode bit stream buffer size.*/
    } sgks_mpi_venc_enc_buf_s;

    typedef struct __sgks_venc_yuv_enc_s
    {
        u32 enc_enable;
        u32 y_addr;
        u32 uv_addr;
        u32 repeat_nums;
        u16 pitch;
        u16 height;
		u32 yuv_pts;
        u8  area_id;
        u8  zoom_enable;
    } sgks_mpi_venc_yuv_enc_s;

    typedef struct __sgks_venc_yuv_enc_ex_s
    {
        u32 enc_enable;
        u32 y_addr;
        u32 uv_addr;
        u32 repeat_nums;
        u16 pitch;
        u16 height;
		u32 yuv_pts;
        u8  area_id;
        u8  zoom_enable;
        u32 outputAddr;
        u32 outputSize;
    } sgks_mpi_venc_yuv_enc_ex_s;
    /********************* VENC END******************/

/*********************OSD******************/
typedef enum __sgks_osd_enc_font_type_e
{
    OSD_FONT_FREETYPE_TYPE = 0,
    OSD_FONT_LATTICE_TYPE,
} sgks_osd_enc_font_type_e;

typedef struct __sgks_osd_enc_area_mem_param_s
{
    u8     planeNum;
    u8     areaNum;      /*the index of area(0~2), each plane has 3 areas.*/
    u32    areaSize[3];
} sgks_osd_enc_area_mem_param_s;


typedef struct __sgks_mpi_osd_enc_area_params_s
{
    u8     planeId;    /*the index of plane(0~3).*/
    u8     areaId;     /*the index of area(0~2).*/
    u8     enable;     /*the enable/disable flag of area(0:disable, 1:enable).*/
    u16    width;       /*area width.*/
    u16    height;     /*area height.*/
    u16    offsetX;    /*area x offset.*/
    u16    offsetY;    /*area y offset.*/
} sgks_mpi_osd_enc_area_params_s;

typedef struct __sgks_mpi_osd_enc_area_index_s
{
    u8      planeId;  /*the index of plane(0~3), each stream has only one plane, so plane index means stream index.*/
    u8      areaId;   /*the index of area(0~2), each plane has 3 areas.*/
} sgks_mpi_osd_enc_area_index_s;

typedef struct __sgks_osd_enc_area_mapping_s
{
    u8      areaId;                   /*the index of area.*/
    u8      *clutStartAddr;       /*yuv colour look-up table start address.*/
    u32    clutSize;                  /*number of bytes of colour look-up table.*/
    u8    *areaStartAddr;     /*plane area start address.*/
    u32    areaSize;                /*number of bytes of area.*/
} sgks_mpi_osd_enc_area_mapping_s;

/*********************OSD END******************/
/*
 ******************************************************************************************
 * ISP START
 ******************************************************************************************
 */

typedef struct __sgks_mpi_isp_ContrastAttr
{

    /*!
        0: disable auto contrast
        1: enable  auto contrast
    */
    int    enableAuto;

    /*!
        manual contrast strength.
        possible value: [0, 0xFF]
        default value: 0x80
    */
    int    manualStrength;
    /*!
        auto contrast strength
        possible value: [0, 0xFF]
        default value: 0x80
    */
    int    autoStrength;

} sgks_mpi_isp_ContrastAttr_s;

typedef enum __sgks_mpi_isp_ExposureType_e
{
    SGKS_ISP_EXPOSURE_TYPE_AUTO = 0,
    SGKS_ISP_EXPOSURE_TYPE_MANUAL,
} sgks_mpi_isp_ExposureType_e;

typedef enum __sgks_mpi_isp_ae_metering_mode_e
{
    SGKS_AE_SPOT_METERING = 0,
    SGKS_AE_CENTER_METERING,
    SGKS_AE_AVERAGE_METERING,
    SGKS_AE_CUSTOM_METERING,
    SGKS_AE_METERING_TYPE_NUMBER,
}sgks_mpi_isp_ae_metering_mode_e;


typedef struct __sgks_mpi_isp_AeAttr_s
{
    unsigned int        speed;
    /*! values from 1 to 8000.
        means the counts of shutter works each second.
        The value of shutterTimeMin should be larger than shutterTimeMax.
        For the larger the value is, the smaller shutter time will be.
    */
    unsigned int        shutterTimeMin;
    /*! values from 1 to 8000.
        means the counts of shutter works each second.
    */
    unsigned int        shutterTimeMax;
    unsigned int        gainMax;
    unsigned int        gainMin;
    /*!
        ae target ratio level
        possible value: [0, 0xFF]
        default value: 0x80,
        [0,   0x7f]  decrease ae target
        [0x80,0xff]  increase ae target
    */
    unsigned int        tagetRatio;
}  sgks_mpi_isp_AeAttr_s;

typedef struct __sgks_mpi_isp_MeAttr_s
{
    unsigned int        shutterTime;
    unsigned int        gain;
} sgks_mpi_isp_MeAttr_s;


typedef enum __sgks_mpi_isp_WhiteBalanceType_e
{
    SGKS_ISP_WB_TYPE_AUTO = 0,
    SGKS_ISP_WB_TYPE_MANUAL,
} sgks_mpi_isp_WhiteBalanceType_e;

typedef struct __sgks_mpi_isp_MwbAttr_s
{

    /* 0x0000 - 0x4000 */
    unsigned int                gainRed;

    /* 0x0000 - 0x4000 */
    unsigned int                gainGreen;

    /* 0x0000 - 0x4000 */
    unsigned int                gainBlue;

} sgks_mpi_isp_MwbAttr_s;







/********************errcode************/
#define  SGKS_SUCCESS                     			(0)
#define  SGKS_FAIL                        			(-1)

#define  SGKS_ERR_SYS_INITFAIL			    		    (-110001)
#define  SGKS_ERR_SYS_OPENFWFILEFAIL         			(-110002)
#define  SGKS_ERR_SYS_SEEKFWFILEFAIL		    		(-110003)
#define  SGKS_ERR_SYS_READFWFILEFAIL					(-110004)
#define  SGKS_ERR_SYS_FWCODEMEMCONFIGERR	    		(-110005)
#define  SGKS_ERR_SYS_OPENDRIVERDEVERR       		    (-110006)
#define  SGKS_ERR_SYS_MMAPADDRERR						(-110007)
#define  SGKS_ERR_SYS_DRIVERIOCTLERR					(-110008)
#define  SGKS_ERR_SYS_FWCMDSENDERR					    (-110009)
#define  SGKS_ERR_SYS_INITMEMPARAMFAIL			        (-110010)
#define  SGKS_ERR_SYS_PARAMERR                          	(-110011) 
#define  SGKS_ERR_SYS_MALLOCERR                       	(-110012) 
#define  SGKS_ERR_SYS_FREEERR                           	(-110013) 
#define  SGKS_ERR_SYS_SHMCREATEERR                    	(-110014) 
#define  SGKS_ERR_SYS_SHMGETERR     		               	(-110015) 
#define  SGKS_ERR_SYS_VENC_TO_VDEC_ERR     		         	(-110016) 
#define  SGKS_ERR_SYS_SET_DSP_TIMER_MODE_ERR     		         	(-110017) 
#define  SGKS_ERR_SYS_WAIT_DSP_STATUS_ERR     		         	(-110018) 



#define  SGKS_ERR_VI_xxxxx			                	(-120001)
#define  SGKS_ERR_VI_PARAM_ERR                          (-120002)

#define  SGKS_ERR_VO_xxxxx			                	(-130001)


#define  SGKS_ERR_VENC_MALLOCERR                            (-140001)
#define  SGKS_ERR_VENC_PARAMERR                             (-140002)
#define  SGKS_ERR_VENC_DEVICEERR                            (-140003)
#define  SGKS_ERR_VENC_SENDCMDFAIL                          (-140004)
#define  SGKS_ERR_VENC_STATEERROR                           (-140005)



#define  SGKS_ERR_VDEC_INITPARAMERR			            	(-150001)
#define  SGKS_ERR_VDEC_DSPBUFFROOM_NOT_ENOUGH				(-150002)
#define  SGKS_ERR_VDEC_DSPBUFFROOM_ERR						(-150003)
#define  SGKS_ERR_VDEC_INFO_ERR								(-150004)
#define  SGKS_ERR_VDEC_STOPDECERR       	                    (-150005)
#define  SGKS_ERR_VDEC_STOPDECSENDFLAGERR  	                (-150006)
#define  SGKS_ERR_VDEC_REPEAT_ENABLE		  	                (-150007)

#define  SGKS_ERR_VCAP_xxxxx			            	(-160001)

#define  SGKS_ERR_OSD_PARAMERR		              (-170001)
#define  SGKS_ERR_OSD_MALLOCERR                   (-170002)
#define  SGKS_ERR_OSD_DEVICEERR                   (-170003)

#define  SGKS_ERR_ISP_STATISTICS_FAIL               (-180001)
#define  SGKS_ERR_ISP_DEINIT_FAIL                   (-180002)
#define  SGKS_ERR_ISP_INIT_FAIL                     (-180003)
#define  SGKS_ERR_ISP_PARAM_ERR                     (-180004)
#define  SGKS_ERR_ISP_FUNCTION_ERR                  (-180005)
#define  SGKS_ERR_ISP_RESET_3A_ERR                  (-180006)

#define  SGKS_ERR_SENSOR_DRIVERIOCTLERR				(-190001)




/*************************************************************************
*                          function                                      *
**************************************************************************/


/************* sys ********/
int sgks_mpi_sys_Init(sgks_mpi_init_s *mpi_init_param, mpi_mem_map_table_s *mpi_mem_param);
int sgks_mpi_sys_DeInit();
int sgks_mpi_syc_closefd(void);
int sgks_mpi_sys_Start();
int sgks_mpi_vi_SetDispalySrc(sgks_mpi_vi_type_e vi_src);
int sgks_mpi_sys_MallcKMem(sgks_mpi_mem_s *meminfo, u32 size);
int sgks_mpi_sys_FreeKMem(sgks_mpi_mem_s *meminfo);
int sgks_mpi_sys_PreViewInit(sgks_preview_config_s *preview_config);
int sgks_mpi_sys_update_change_preview(sgks_preview_change_s *pre_change_info);
int sgks_mpi_sys_DecToEnc();
int sgks_mpi_sys_GetDspStats(sgks_mpi_dsp_status_s *dsp_status);
int sgks_mpi_sys_SetupDspMem(sgks_fw_init_dram_config_s *mem_config);
void* sgks_mpi_Malloc(int size);
void sgks_mpi_Free(void* ptr);



/************* vi ********/
int sgks_mpi_vi_init(sgks_mpi_vi_device_s *vi_device);
int sgks_mpi_vi_Deinit(void);

int sgks_mpi_vi_SetMirrorMode(sgks_mpi_vi_MirrorMode_e mirror_mode);
int sgks_mpi_vi_SetBayerPattern(sgks_mpi_vi_BayerPattern_e bayer_pattern);

int sgks_mpi_vi_GetSupportResolution(sgks_mpi_vi_resolution_s *resolution);

u32 sgks_mpi_vi_GetSensorID(void);
u32 sgks_mpi_vi_GetInputFormat(void);


/************* vo ********/
int sgks_mpi_vo_GetScreenParam(sgks_mpi_vo_device_s *vo_dev);
int sgks_mpi_vo_init(sgks_mpi_vo_device_s *vo_device);
int sgks_mpi_vout_Deinit();
int sgks_mpi_vo_GetFreeBank(sgks_mpi_vo_DisBankParams_s *vobankparams);
int sgks_mpi_vo_UpdateBank(sgks_mpi_vo_DisBankParams_s *pBankInfo);

/************* isp ********/
int sgks_mpi_isp_Init(sgks_isp_initParam_s *isp_config);
int sgks_mpi_isp_DeInit();
// nBrightness [0-255]
int sgks_mpi_isp_SetBrightness(int nBrightness);
int sgks_mpi_isp_GetBrightness(int *pBrightness);

// nSaturation [0-255]
int sgks_mpi_isp_SetSaturation(int nSaturation);
int sgks_mpi_isp_GetSaturation(int *pSaturation);
int sgks_mpi_isp_SetContrast(int enableAuto, int nContrast);

int sgks_mpi_isp_GetExposureCurrentShutter(double *p_shutter);
int sgks_mpi_isp_GetExposureCurrentGain(u32 *p_gain);
int sgks_mpi_isp_GetExposureCurrentTarget(int *p_target);
int sgks_mpi_isp_GetExposureCurrentLuma(int *p_luma);
int sgks_mpi_isp_SetExposureRoi(int roi[96]);
int sgks_mpi_isp_SetExposureMeterMode(sgks_mpi_isp_ae_metering_mode_e mode);
int sgks_mpi_isp_SetExposureType(sgks_mpi_isp_ExposureType_e type);
int sgks_mpi_isp_SetAeSpeed(u8 ae_speed);
int sgks_mpi_isp_SetAeEvBias(int ev_bias);
int sgks_mpi_isp_SetAeAttr(sgks_mpi_isp_AeAttr_s AeAttr);
int sgks_mpi_isp_GetAeAttr(sgks_mpi_isp_AeAttr_s *pAeAttr);
int sgks_mpi_isp_SetMeAttr(sgks_mpi_isp_MeAttr_s MeAttr);
int sgks_mpi_isp_GetMeAttr(sgks_mpi_isp_MeAttr_s *pMeAttr);
int sgks_mpi_isp_SetWhiteBalanceType(sgks_mpi_isp_WhiteBalanceType_e type);
int sgks_mpi_isp_SetAwbSpeed(unsigned char awb_speed);
int sgks_mpi_isp_SetMwbAttr(sgks_mpi_isp_MwbAttr_s MwbAttr);
int sgks_mpi_isp_GetMwbAttr(sgks_mpi_isp_MwbAttr_s *pMwbAttr);
int sgks_mpi_isp_SetLeaMode(u32 mode);

/************* cap ********/
int sgks_mpi_cap_Init(sgks_mpi_cap_init_param_s *cap_init_param);
int sgks_mpi_cap_GetData(sgks_mpi_cap_yuv_info_s *yuv_info);


/************* venc ********/
int sgks_mpi_venc_init(void);
int sgks_mpi_venc_deinit(void);
int sgks_mpi_venc_get_params(sgks_mpi_venc_param_type_e paramType, void *pParam);
int sgks_mpi_venc_set_params(sgks_mpi_venc_param_type_e paramType, void *pParam);
int sgks_mpi_venc_set_config(sgks_mpi_venc_config_type_e configType, void *pConfig);
int sgks_mpi_venc_get_config(sgks_mpi_venc_config_type_e configType, void *pConfig);
int sgks_mpi_venc_state_control(sgks_mpi_venc_state_control_type_e stateControlType, u32 streamId);
int sgks_mpi_venc_get_stream(u32 streamId, sgks_mpi_venc_stream_s *pStream);
int sgks_mpi_venc_map_encode_buf(sgks_mpi_venc_enc_buf_s *info);
int sgks_mpi_venc_map_encode_buf_ex(sgks_mpi_venc_enc_buf_s *info, int nWidth, int nHeight, int nFrames);
int sgks_mpi_venc_feed_yuv_to_encode(sgks_mpi_venc_yuv_enc_s *info);
int sgks_mpi_venc_yuv_to_jpeg( u32 srcYuv420Addr, u16 srcWidth, u16 srcHeight, u32 *dstJpegAddr, u32 *dstSize);
int sgks_mpi_venc_unmap_encode_buf(void);






/************* vdec ********/
int sgks_mpi_vdec_Enable(sgks_mpi_vdec_init_Param_s *vdec_param);
int sgks_mpi_vdec_Disable(sgks_mpi_vdec_stop_Param_s *vdec_stop_Param);
inline int sgks_mpi_vdec_SendData(u8 *data, int size);
int sgks_mpi_vdec_GetDecInfo(sgks_mpi_vdec_info_s *dev_info);
int sgks_mpi_vdec_h264_fw_buffer_malloc(sgks_mpi_vdec_init_Param_s *vdec_param);
int sgks_mpi_vdec_mjpeg_fw_buffer_malloc(sgks_mpi_vdec_init_Param_s *vdec_param);
int sgks_mpi_vdec_fw_buffer_free(void);
/************* osd ********/
int sgks_mpi_Osd_Open(int fbEn);
int sgks_mpi_Osd_Mallc(sgks_mpi_mem_s *meminfo, u32 size);
int sgks_mpi_Osd_Free(sgks_mpi_mem_s *meminfo);
int sgks_mpi_Osd_VoParamSet(sgks_mpi_osd_voParamSetup_s *param);
int sgks_mpi_Osd_VoUpdateDisplay(sgks_mpi_mem_s *meminfo);
int sgks_mpi_osd_enc_init(void);
int sgks_mpi_osd_enc_deinit(void);
int sgks_mpi_osd_enc_config_area_mem(sgks_osd_enc_area_mem_param_s *areamem);
int sgks_mpi_osd_enc_get_area_mapping(sgks_mpi_osd_enc_area_index_s areaIndex, sgks_mpi_osd_enc_area_mapping_s *areaMapping);
int sgks_mpi_osd_enc_set_area_params(sgks_mpi_osd_enc_area_params_s *areaParams);
int sgks_mpi_osd_enc_get_area_params(sgks_mpi_osd_enc_area_params_s  *areaParams);



#ifdef  __cplusplus
}
#endif

#endif







