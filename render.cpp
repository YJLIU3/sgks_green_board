#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "ringbuffer.hpp"
#include "mpi/mpi_lib.h"
#include "gpu2d.h"
#include <HAL/gc_hal_types.h>
#include <HAL/gc_hal.h>
#include <HAL/gc_hal_raster.h>
#include <pthread.h>
#include <sys/prctl.h>
using namespace std;
RingBuffer<sgks_mpi_cap_yuv_info_s>gVOutQ(4);
int SetVOutQ(sgks_mpi_cap_yuv_info_s yuv_info)
{
	return gVOutQ.PushItem(yuv_info);
}
void SaveNV12File(sgks_mpi_cap_yuv_info_s yuv_info)
{
	static int idx = 0;
	int		fd;
	char	filename[256];

	sprintf(filename,"/mnt/%dx%d-%d.bin",yuv_info.width, yuv_info.height,idx++);
	fd = open(filename, O_CREAT|O_WRONLY, 0666);
	if(fd>=0)
	{
		write(fd, yuv_info.yAddr, yuv_info.stride * yuv_info.height);
		if(yuv_info.frameFormat == SGKS_CAP_YUV_SEMIPLANAR_420)
			write(fd, yuv_info.uvAddr,yuv_info.stride * yuv_info.height/2);
		else
			write(fd, yuv_info.uvAddr,yuv_info.stride * yuv_info.height);
		fsync(fd);
		close(fd);
	}
}
extern void GetLCDSize(unsigned int *nLCDWidth, unsigned int * nLCDHeight);
void * RenderProc(void * pParams)
{
	sgks_mpi_cap_yuv_info_s 	yuv_info;
	sgks_mpi_vo_DisBankParams_s disbank={0};
	struct timeval 	tv0={0},tv1={0},tv2={0};
	struct timezone tz;
	GPU2D 			gpu;
	gctPOINTER 		pVirt[3];
	gctUINT32		nSize[3];
	gcsRECT			srcRC={0};
	gcsRECT			frgRC={0};
	gctUINT			nLCDWidth = 0;
	gctUINT			nLCDHeight= 0;
	gctUINT			nTimeInterval = 0,y0,y1;
	int				xFlag = 0;
	prctl(PR_SET_NAME, "RenderProc");
	gettimeofday(&tv0, &tz);
	while(1)
	{
		if(!gVOutQ.PopItem(yuv_info))
		//if(!gVOutQ.PopLastedItem(yuv_info))
		{
			usleep(10000);
			continue;
		}
/*
		xFlag++;
		if(xFlag>200 && xFlag <212 && ((xFlag&0x03) == 0))
		{
			SaveNV12File(yuv_info);
		}

		//gVOutQ.printQInfo();
		//if(((xFlag+1)&0xFF) != yuv_info.addr_type)
		//	printf("%d\n", (unsigned int)yuv_info.addr_type);
		//xFlag = yuv_info.addr_type;
*/
		if(nLCDWidth == 0)
		{
			//GetLCDSize(&nLCDWidth, &nLCDHeight);
		}
		srcRC.left	= 0;
		srcRC.top	= 0;
		srcRC.right	= yuv_info.width/2;
		srcRC.bottom= yuv_info.height;
		pVirt[0] = (gctPOINTER )yuv_info.yAddr;//(((unsigned int)pBuffer[0]+0x63)&0xFFFFFFC0);//
		pVirt[1] = (gctPOINTER )yuv_info.uvAddr;//(((unsigned int)pBuffer[1]+0x63)&0xFFFFFFC0);//
		nSize[0] = yuv_info.stride * yuv_info.height;
		nSize[1] = (yuv_info.stride * yuv_info.height);

		gpu.ConfigSurface(0,pVirt,nSize,2,yuv_info.width,yuv_info.height,16,gcvSURF_NV12);
/*
		pVirt[0] = (gctPOINTER *)yuv_info.yAddr;
		pVirt[1] = (gctPOINTER *)yuv_info.uvAddr;
		nSize[0] = yuv_info.stride * yuv_info.height;
		nSize[1] = (yuv_info.stride * yuv_info.height)/2;
		gpu.ConfigSurface(1,pVirt,nSize,2,yuv_info.width,yuv_info.height,16,gcvSURF_R8G8B8A8);
*/
		gettimeofday(&tv1, &tz);
		sgks_mpi_vo_GetFreeBank(&disbank);
		pVirt[0] = (gctPOINTER *)disbank.bankaddr;
		pVirt[1] = (gctPOINTER *)((u8*)disbank.bankaddr+(disbank.banksize>>1));
		nSize[0] = disbank.banksize>>1;
		nSize[1] = nSize[0];

		gpu.ConfigSurface(2,pVirt,nSize,2,nLCDWidth,nLCDHeight,16,gcvSURF_NV16);
		gpu.RenderSurface(srcRC,frgRC,gcvSURF_0_DEGREE);

		gettimeofday(&tv2, &tz);
		nTimeInterval = (tv2.tv_sec - tv0.tv_sec) * 1000 + (tv2.tv_usec - tv0.tv_usec)/1000;
		//Frame rate controlling
		if(nTimeInterval<20)
			;//usleep(1000);
		y0 = (tv1.tv_sec - tv0.tv_sec) * 1000 + (tv1.tv_usec - tv0.tv_usec)/1000;
		y1 = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec)/1000;
		//printf("%03d\t%03d\t%03d\t%08X\n", nTimeInterval,y0,y1,(unsigned long )yuv_info.yAddr);
	//	sgks_mpi_vo_UpdateBank(&disbank);
		gettimeofday(&tv0, &tz);
		memset(&yuv_info, 0x00, sizeof(sgks_mpi_cap_yuv_info_s));
	}
	return NULL;
}
