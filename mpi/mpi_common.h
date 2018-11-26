
#ifndef _SGKS_MPI_COMM_H_
#define _SGKS_MPI_COMM_H_


#ifdef  __cplusplus
extern "C"
{
#endif


#define MSG_DATA_LEN  (1024)

typedef struct __msg_head_t
{
	unsigned int   msg_type;  
	unsigned int   msg_subtype; 
	unsigned int   msg_size;  
    int            ack_flag;  
	unsigned char  reserved[4];
} msg_head_t;

typedef struct __msg_t
{
    msg_head_t msg_head; 
    char msg_data[MSG_DATA_LEN];  
}msg_t;
															

typedef unsigned char       u8;     /**< UNSIGNED 8-bit data type */
typedef unsigned short     u16;     /**< UNSIGNED 16-bit data type */
typedef unsigned int       u32;     /**< UNSIGNED 32-bit data type */
typedef unsigned long long u64;     /**< UNSIGNED 64-bit data type */
typedef signed char         s8;     /**< SIGNED 8-bit data type */
typedef signed short       s16;     /**< SIGNED 16-bit data type */
typedef signed int         s32;     /**< SIGNED 32-bit data type */


#ifdef  __cplusplus
}
#endif

#endif

