#ifndef _SGKS_MDI_COMM_H_
#define _SGKS_MDI_COMM_H_

#include "mpi_common.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define SGKS_MDI_IOC_MAGIC                        'm'
#define _MDI_IOR(IOCTL, param)                    _IOR(SGKS_MDI_IOC_MAGIC, IOCTL, param)
#define SGKS_MDI_IOCTL_OPERATION_VALUE         (0x1001001)
#define SGKS_MDI_IOCTL_OPERATION                _MDI_IOR(SGKS_MDI_IOCTL_OPERATION_VALUE, struct __msg_t*)
#define CMD_BUFF_TMP_SIZE						    (4096)

#define SGKS_MDI_DRV_BUFF(_ptr)                              \
	msg_t ioctl_msg;				                            \
	memset(ioctl_msg.msg_data, 0, sizeof(ioctl_msg.msg_data));  \
	_ptr = ioctl_msg.msg_data;

#define SGKS_MDI_DRV_CLEAN_DATABUFF()                              \
	memset(ioctl_msg.msg_data, 0, sizeof(ioctl_msg.msg_data));  \
 
#define SGKS_MDI_DRV_IOCTL(_Msgtype, _MsgSubType, _MsgDataLen, _err_ret)      															\
	memset(&ioctl_msg.msg_head, 0, sizeof(ioctl_msg.msg_head));     						  													\
	ioctl_msg.msg_head.msg_type     = _Msgtype;							                      												\
    ioctl_msg.msg_head.msg_subtype  = _MsgSubType;																								\
	ioctl_msg.msg_head.msg_size     = _MsgDataLen;																								\
	if (_MsgDataLen > sizeof(ioctl_msg.msg_data))																								\
	{																																			\
		Printf("ioctl msg len err\n");																											\
		return _err_ret;										    																			\
	}																																			\
	if (ioctl(mpi_get_managerHanle()->driver_handle[mpi_get_managerHanle()->curr_module_id], SGKS_MDI_IOCTL_OPERATION, &ioctl_msg) < 0)	\
    {																																			\
      	Printf("ioctl msg send err.\n");   																										\
        return _err_ret;																														\
    }

#define SGKS_MDI_DRV_IOCTL_EX(_Fd, _Msgtype, _MsgSubType, _MsgDataLen, _err_ret)      	\
	memset(&ioctl_msg.msg_head, 0, sizeof(ioctl_msg.msg_head));     						  	\
	ioctl_msg.msg_head.msg_type     = _Msgtype;							                      	\
    ioctl_msg.msg_head.msg_subtype  = _MsgSubType;												\
	ioctl_msg.msg_head.msg_size     = _MsgDataLen;														\
	if (_MsgDataLen > sizeof(ioctl_msg.msg_data))														\
	{																							\
		Printf("ioctl msg len err\n");															\
		return _err_ret;										    								\
	}																							\
	if (ioctl(_Fd, SGKS_MDI_IOCTL_OPERATION, &ioctl_msg) < 0)	\
    {																							\
      	Printf("ioctl msg send err.\n");   														\
        return _err_ret;																			\
    }

/////////////////////////////////////////////////////////////

    typedef struct sgks_dsp_cmd_header
    {
        u32 cmd_seq_num;
        u32 num_cmd;
        //u32 num_enc_cmds;
    } sgks_dsp_cmd_header_s;

    typedef struct sgks_cmd_info
    {
        sgks_dsp_cmd_header_s cmd_head;
        u32* buffer_addr;
        u32  buffer_size;
        u8   *buffer_temp_area;
    } sgks_cmd_info_s;



    typedef enum
    {
        SGKS_MDI_CMD_MODE_NONE 							= 0x0,       // not booted
        SGKS_MDI_CMD_MODE_BUSY 							= 0x1,       // changing mode
        SGKS_MDI_CMD_MODE_DEFAULT_LOADING 				= 0x2,    // default command mode
        SGKS_MDI_CMD_MODE_DEFAULT_READY 					= 0x3,    // default command mode
        SGKS_MDI_CMD_MODE_BLOCK_LOADING 					= 0x4,      // block mode
        SGKS_MDI_CMD_MODE_BLOCK_READY 					= 0x5,      // block mode
        SGKS_MDI_CMD_MODE_NORMAL_LOADING 				= 0x6,     // normal command mode
        SGKS_MDI_CMD_MODE_NORMAL_READY 					= 0x7,     // normal command mode
        SGKS_MDI_CMD_MODE_VDEC_NORMAL_LOADING 			= 0x8,     // normal command mode
        SGKS_MDI_CMD_MODE_VDEC_NORMAL_READY 			= 0x9,     // normal command mode
    } sgks_mdi_cmd_mode_e;

    typedef enum __sgks_kernel_mem_type
    {
        KER_MEM_PPM = 0,
	    KER_MEM_DSP,
	    KER_MEM_BSB,
	    KER_MEM_AFT, 
	    KER_MEM_NUM
    }sgks_kernel_mem_type_e;
	
    typedef struct sgks_cmd_buffer_config
    {
        sgks_mdi_cmd_mode_e   	cmd_mode;
        sgks_cmd_info_s 		default_cmd_buffer;
        sgks_cmd_info_s			normal_cmd_buffer;
        sgks_cmd_info_s			block_cmd_buffer;
		sgks_cmd_info_s			block_occupy_buffer;/*mdi only*/
    } sgks_cmd_buffer_config_s;

    typedef struct sgks_result_queue_buffer
    {
        u8  *dsp_result_queue_start;
        u8 	*dsp_result_queue_end;
        u8	*dsp_result_queue_ptr;
        u32 dsp_result_queue_size;
    } sgks_result_queue_buffer_s;

    typedef struct sgks_dsp2_info_buffer
    {
        u8  *dsp2_info_ptr;
        u32 dsp2_info_ptr_size;
    } sgks_dsp2_info_buffer_s;

    typedef struct sgks_cap_info_buffer
    {
        u8  *cap_info_ptr;
        u32 cap_info_ptr_size;
    } sgks_cap_info_buffer_s;

typedef struct sgks_mdi_yuv_info
{
	u32 encode_pts;
	u32 encode_seqnum;    
	u32 encode_y_pic_addr;
	u32 encode_uv_pic_addr;
	u32 encode_yuv_pitch;	
    u32 encode_yuv_width;
    u32 encode_yuv_height;
    u32 rescale_y_pic_addr;
    u32 rescale_uv_pic_addr;
    u32 rescale_yuv_pitch;
    u32 rescale_yuv_height;
    u32 rescale_yuv_width;
    u8  rescale_yuv_type;
    u32 cvbs_rescale_y_pic_addr;      
    u32 cvbs_rescale_uv_pic_addr;      
    u32 cvbs_rescale_y_pitch;      

	/* VDEC YUV */
	u32 yuv422_y_addr;	//SCREEN
	u32 yuv422_uv_addr;
	u32 second_rescale_buf_address_y; //SCALE
	u32 second_rescale_buf_address_uv;
	u32 second_rescale_buf_pitch; /* pitch of scaled JPEG after decoding */
	u16 second_rescale_buf_width;
	u16 second_rescale_buf_height;
	u32 jpeg_y_addr; //MAIN
	u32 jpeg_uv_addr;
	u32 jpeg_pitch; /* DRAM pitch of decoded JPEG */
	u32 jpeg_width; /* width of decoded JPEG */
	u32 jpeg_height;
	u16 yuv422_width;
	u16 yuv422_height;
	u16 yuv422_pitch;
	u16 yuv422_type;
	u16 jpeg_type;
	u16 second_rescale_buf_type;
} sgks_mdi_yuv_info_s;

    typedef struct __sgks_bitsteam_buffer_info
    {
        u8 *bsb_phy_start;
        u8 *bsb_kernel_start;
        u8 *bsb_user_start;
        u32 bsb_size;

        u8 *bs_desc_phy_start;
        u8 *bs_desc_kernel_start;
        u8 *bs_desc_user_start;
        u32 bs_desc_size;
    } sgks_bitsteam_buffer_info_s;

    typedef struct sgks_dsp_buffer_config
    {
        sgks_cmd_buffer_config_s 	 cmd_buff;
        sgks_result_queue_buffer_s   result_queue;
        sgks_dsp2_info_buffer_s		 dsp2_info;
        sgks_bitsteam_buffer_info_s   bsb_info;
    } sgks_dsp_buffer_config_s;

    typedef struct sgks_mdi_dsp_status
    {
        u32  op_mode;       /*sgks_dsp_op_mode_e*/
        u32  op_sub_mode;  /*sgks_encode_mode_e*/
        u32  state;         /*sgks_encode_state_e, sgks_decode_state_e */
        u16  stream_encode_state[4];
    } sgks_mdi_dsp_status_s;


typedef struct sgks_isp_buffer_config
{
    u8*  isp_aaa_fifo1_base;
    u32  isp_aaa_fifo1_base_size;
    u8*  isp_aaa_fifo2_base;
    u32  isp_aaa_fifo2_base_size;
} sgks_isp_buffer_config_s;

typedef enum
{
    SGKS_OPT_D_CACHE_CLAEN = 0,        
    SGKS_OPT_D_CACHE_INV,       
} sgks_mdi_opt_d_cache_e;

    typedef struct sgks_mdi_opt_d_cache
    {
        sgks_mdi_opt_d_cache_e 	cache_type;
        u8 						*start;
        unsigned long 			size;
    } sgks_mdi_opt_d_cache_s;
    typedef struct sgks_crypto_des_key
    {
        u32     key_hi;
        u32     key_lo;
    } sgks_crypto_des_key_s;

    typedef struct sgks_crypto_aes128_key
    {
        u32     key_127_96;
        u32     key_95_64;
        u32     key_63_32;
        u32     key_31_0;
    } sgks_crypto_aes128_key_s;

    typedef struct sgks_crypto_aes192_key
    {
        u32     key_191_160;
        u32     key_159_128;
        u32     key_127_96;
        u32     key_95_64;
        u32     key_63_32;
        u32     key_31_0;
    } sgks_crypto_aes192_key_s;

    typedef struct sgks_crypto_aes256_key
    {
        u32     key_255_224;
        u32     key_223_192;
        u32     key_191_160;
        u32     key_159_128;
        u32     key_127_96;
        u32     key_95_64;
        u32     key_63_32;
        u32     key_31_0;
    } sgks_crypto_aes256_key_s;


    typedef enum
    {
        SGKS_CRYPTO_DES        = 0,    // used DES
        SGKS_CRYPTO_AES_128,           // used AES_128
        SGKS_CRYPTO_AES_192,           // used AES_192
        SGKS_CRYPTO_AES_256,           // used AES_256
        SGKS_CRYPTO_MODE_MAX,          //
    } sgks_crypto_mode_e;

    typedef union
    {
        u32                       key[8];
        sgks_crypto_des_key_s     des_key;
        sgks_crypto_aes128_key_s  aes128_key;
        sgks_crypto_aes192_key_s  aes192_key;
        sgks_crypto_aes256_key_s  aes256_key;
    } sgks_crypto_key_s;


    typedef struct sgks_mdi_decrpty_mem_data
    {
        sgks_crypto_mode_e    mode;
        sgks_crypto_key_s     pkey;
		u8*					  phy_mem_addr;
		u32					  size;
    } sgks_mdi_decrpty_mem_data_s;

typedef int  (*CB_ISP_CMD_FUN)(u32 cmd, u8 *arg);
typedef int  (*CB_ISP_MDI_FUN)(u32 cmd, u8 *arg);
typedef int  (*CB_ISP_GETFRAMETIME_FUN)(u32 fps);
typedef int  (*CB_ISP_GET_MPI_STATIS_FUN)(u32 *status);
typedef int  (*CB_ISP_CONFIG_SENSOR_FUN)(u32 id, u8 *arg, int size);

typedef struct sgks_mdi_main_mpi
{
	CB_ISP_CMD_FUN	 			isp_CmdCallBack;
	CB_ISP_MDI_FUN	 			isp_MdiCallBack;
	CB_ISP_GETFRAMETIME_FUN		isp_GetFrameTime;
	CB_ISP_GET_MPI_STATIS_FUN   isp_GetMpiState;	
	CB_ISP_CONFIG_SENSOR_FUN	isp_ConfigSensor;	
} sgks_mdi_main_mpi_s;

typedef struct sgks_cmd_dump_mem_file
{
    u64   addr;
    u8    file_path[32];
    u64   len;
} sgks_cmd_dump_mem_file_s;


typedef void (*LOG_CB_FUN)(void *data, int size);
typedef struct sgks_cap_dsp_log_file
{
    u8      	type;
    u8      	*log_mem_addr;
    u8      	log_file_path[32];
    u32     	log_mem_size;
    LOG_CB_FUN  log_callback;
} sgks_cap_dsp_log_file_s;


typedef struct sgks_icore_printf
{
    u32 seq_num;        /**< Sequence number */
    u32 dsp;            /**< 0 - DSP1, 1 - DSP2 */
    u32 format_addr;    /**< Address (offset) to find '%s' arg */
    u32 arg1;           /**< 1st var. arg */
    u32 arg2;           /**< 2nd var. arg */
    u32 arg3;           /**< 3rd var. arg */
    u32 arg4;           /**< 4th var. arg */
    u32 arg5;           /**< 5th var. arg */
} sgks_icore_printf_s;

typedef enum
{
    SGKS_MDI_REG_ADDR_TYPE_PHY   = 0,
    SGKS_MDI_REG_ADDR_TYPE_VIR,
} sgks_mdi_reg_addrType_e;

typedef struct sgks_mdi_reg_info
{
    unsigned long *addr;
    sgks_mdi_reg_addrType_e addr_type;
    unsigned long val;
    unsigned long delay_ms;
} sgks_mdi_reg_info_s;

typedef struct sgks_mdi_reg_rw
{
    u32					reg_num;
    sgks_mdi_reg_info_s  reg_info[50];
} sgks_mdi_reg_rw_s;

typedef struct __sgks_mdi_memaddr_conv_info
{
    u32  type;/*0: phy to kvir 1:kvit to phy*/
    u32  src_addr;
    u32	 desc_addr;
} sgks_mdi_memaddr_conv_info_s;


typedef struct __sgks_mdi_memaddr_conv
{
    u32							  addr_num;
    sgks_mdi_memaddr_conv_info_s  addr_info[50];
} sgks_mdi_memaddr_conv_s;

typedef struct __sgks_mdi_mmap
{
    char *mmap_phy_start_addr;
    int  mmap_phy_size;
    char *mmap_user_start_addr;
    int  mmap_user_size;
} sgks_mdi_mmap_s;

    typedef enum
    {
        SGKS_COMPLETION_ISP_COMPL               = 0x01,
        SGKS_COMPLETION_CMD_AVAIL_COMPL ,
        SGKS_COMPLETION_DECODE_COMPL ,
        SGKS_COMPLETION_STATE_CHANGE_COMPL ,
    } sgks_completion_type_e;

    typedef struct __sgks_mdi_wait_completion
    {
        sgks_completion_type_e completion;
    } sgks_mdi_wait_completion_s;

    typedef struct __sgks_mdi_isp_statistics
    {
        u8*     isp_rgb_aaa_ptr;
        int     isp_rgb_aaa_ptr_size;
        u8*     isp_rgb_aaa_ptr_next;  /*only read*/
        u8* 	isp_cfa_aaa_ptr;
        int 	isp_cfa_aaa_ptr_size;
        u8* 	isp_cfa_aaa_ptr_next;  /*only read*/
        u8* 	isp_hist_aaa_ptr;
        int 	isp_hist_aaa_ptr_size;
        u32 	isp_hist_pitch;
    } sgks_mdi_isp_statistics_s;

typedef struct __sgks_mdi_isp_statistics_pos
{
    u32     isp_rgb_aaa_ptr_pos;
    u32 	isp_cfa_aaa_ptr_pos;
    u32 	isp_hist_aaa_ptr_pos;	
} sgks_mdi_isp_statistics_pos_s;

    typedef struct __sgks_mdi_vdec_buff_pos
    {
        u32		curr_pos;
    } sgks_mdi_vdec_buff_pos_s;


	typedef struct __sgks_mdi_vdec_decode_info
	{
		u32 curr_pts;
		u32 curr_pts_high;
		u32 decoded_frames;
	} sgks_mdi_vdec_decode_info_s;

    typedef struct __sgks_mdi_vdec_buff
    {
        u32     buff_phy;
        int     buff_size;
    } sgks_mdi_vdec_buff_s;

typedef struct __sgks_get_pagesize
{
    u32 page_size;
} sgks_get_pagesize_s;

typedef struct __sgks_mdi_session
{
    int streamId;
    u32 *session;
} sgks_mdi_session_s;

typedef struct __sgks_mdi_enc_bits_info
{
    u32 frame_num;
    u32 PTS;
    u32 start_addr;
    u32 pic_type                :  3;
    u32 level_idc               :  3;
    u32 ref_idc                 :  1;
    u32 pic_struct              :  1;
    u32 pic_size                : 24;

    u32 channel_id              :  8;
    u32 stream_id               :  8;

    u32 cavlc_pjpeg             :  1;   // no used.
    u32 stream_end              :  1;   // 0: normal stream frames,  1: stream end null frame
    u32 top_field_first         :  1;   // 0: non-CAVLC   1: CAVLC pjpeg
    u32 repeat_first_field      :  1;   // 0: normal stream frames,  1: stream end null frame
    u32 progressive_sequence    :  1;
    u32 pts_minus_dts_in_pic    :  5;   // (pts-dts) in frame or field unit. (if progressive_sequence=0, in field unit. If progressive_sequence=1, in frame unit.)
    u32 res_1        : 6;
    u32 session_id;                     // use 32-bit session id
    u32 bsb_start_addr;               //bsb buffer user start addr
    u32 pjpeg_start_addr;               // not used
    u32 pjpeg_size;                     // not used

    u16 addr_offset_sliceheader;        // slice header address offset in
    u16 res_2;                          // slice header address offset in
    s32 cpb_fullness;                   // cpb fullness with considering real cabac/cavlc bits. use sign int, such that negative value indicate cpb underflow.

    u64 stream_pts;                     // 64-bit pts for that stream
}sgks_mdi_enc_bits_info;

    typedef enum
    {
        SGKS_MDI_OPERATION_GETVER   = 0x10001,
        SGKS_MDI_OPERATION_MMAP,
        SGKS_MDI_OPERATION_UNMMAP,
        SGKS_MDI_OPERATION_SETDSPBUFFER,
        SGKS_MDI_OPERATION_SETISPBUFFER,
        SGKS_MDI_OPERATION_SETCMDMODE,
        SGKS_MDI_OPERATION_SENDCMD,
        SGKS_MDI_OPERATION_OPT_D_CACHE,
        SGKS_MDI_OPERATION_REG_READ,
        SGKS_MDI_OPERATION_REG_WRITE,
        SGKS_MDI_OPERATION_SETUPIRQ,
        SGKS_MDI_OPERATION_SETMAINMPI,
        SGKS_MDI_OPERATION_GETMAINMPI,
        SGKS_MDI_OPERATION_GETPAGESIZE,
        SGKS_MDI_OPERATION_ADDR_CONV,
        SGKS_MDI_OPERATION_GET_ISP_STATISTICS,
        SGKS_MDI_OPERATION_GET_VDEC_BUFF_POS,
        SGKS_MDI_OPERATION_GET_VDEC_INFO,
        SGKS_MDI_OPERATION_SET_VDEC_BUFF_INFO,
        SGKS_MDI_OPERATION_DEBUG_DUMPMEMTOFILE,
        SGKS_MDI_OPERATION_DEBUG_REGWRITE,
        SGKS_MDI_OPERATION_DEBUG_REGREAD,
        SGKS_MDI_OPERATION_DEBUG_CAPDSPLOG,
        SGKS_MDI_OPERATION_MALLOC,
        SGKS_MDI_OPERATION_FREE,
        SGKS_MDI_OPERATION_GETDSPSTATUS,
        SGKS_MDI_OPERATION_WAITDSPSTATUS,
        SGKS_MDI_OPERATION_SETENCODE,
        SGKS_MDI_OPERATION_READ_BITSTREAM,
        SGKS_MDI_OPERATION_GET_SESSION_ID,
        SGKS_MDI_OPERATION_CREATE_SESSION_ID,
        SGKS_MDI_OPERATION_GET_YUV_INFO,
        SGKS_MDI_OPERATION_WAIT_COMPLETION,
        SGKS_MDI_OPERATION_OSD_BIND_FB,
        SGKS_MDI_OPERATION_DECRYPT_MEM_DATA,
        SGKS_MDI_OPERATION_WAIT_YUV_ENC_COMPLETE,
        SGKS_MDI_OPERATION_GET_YUV_ENC_JPEG_INFO,
        SGKS_MDI_OPERATION_CLEARUPIRQ,
        SGKS_MDI_OPERATION_GET_KERNEL_MEM_INFO,
    } mpi_driver_ioctl_e;

typedef struct __sgks_mdi_mem 
{
    u8 *user_addr;
    u8 *phy_addr;
    u32 size;
} sgks_mdi_mem_s;

typedef struct __sgks_mdi_osdVoInfoToFb
{
    u16 width;
    u16 height;
    u32 directMode;
    u32 osd_buf_info_phy_addr;
} sgks_mdi_osdVoInfoToFb_s;

typedef struct __sgks_mdi_osdVoInfo
{
    u32 osd_clut_addr;
    u32 osd_data_buf_addr;
    u32 osd_info_buf_addr;
} sgks_mdi_osdVoInfo_s;

typedef struct __sgks_mdi_jpeg_info
{
    u32 data_addr;
    u32 data_size;
} sgks_mdi_jpeg_info_s;

typedef struct __sgks_kernel_mem_info
{
    sgks_mdi_mem_s  mem_info[KER_MEM_NUM];
}sgks_kernel_mem_info;

#ifdef  __cplusplus
}
#endif

#endif





