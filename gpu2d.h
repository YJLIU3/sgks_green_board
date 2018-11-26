/*
 * gpu2d.hpp
 *
 *  Created on: 2018年1月26日
 *      Author: sgks
 */

#ifndef _GPU2D_HPP_
#define _GPU2D_HPP_
#include <HAL/gc_hal_types.h>
#include <HAL/gc_hal.h>
#include <HAL/gc_hal_raster.h>
#include "mpi/mpi_lib.h"
typedef struct _IMAGE_INFO_
{
	gctUINT32		VirtAddr[3];
	gctINT          AddressNum;
	gctINT          StrideNum;
	gceSURF_FORMAT  Format;
	gctUINT32       Width;
	gctUINT32       Height;
}IMAGE_INFO;
#define MAX_MEMINFO	32
typedef struct
{
	gctUINT32		PhysAddr;
	gctPOINTER		VirtAddr;
	gctSIZE_T		Size;
	gctUINT32		Info;
}MEM_INFO;
typedef struct
{
	gcoSURF         Surf;
	gctBOOL			bInited;
	gctINT          AddressNum;
	gctINT          StrideNum;
	gctUINT32       PhysAddr[3];
	gctPOINTER      VirtAddr[3];
	gctUINT32       Stride[3];
	gceSURF_FORMAT  Format;
	gctUINT32       Width;
	gctUINT32       Height;
	gctUINT32       AlignedWidth;
	gctUINT32       AlignedHeight;
	gctUINT32		MaxMemInfo;
	MEM_INFO	*	MemInfo;
}SGKS_SURFACE,*LPSGKS_SURFACE;

class GPU2D
{
private:
	gcoOS			os;
	gcoHAL			hal;
	gco2D 			egn2D;
	gceSURF_FORMAT	format;
	gceSTATUS		created;
	gctUINT8	*	pUVPlane;
	SGKS_SURFACE	srcSurf;
	SGKS_SURFACE	frgSurf;
	SGKS_SURFACE	dstSurf;
	SGKS_SURFACE	tmpSurf;
	SGKS_SURFACE	bmpSurf;

public:
	GPU2D();
	~GPU2D();
	gceSTATUS IsReady(){return created;};
	gceSTATUS GetPhyAddrFromVirt(SGKS_SURFACE * pSGKSurf,gctPOINTER * pVirtAddr,gctUINT32 nSize[],gctUINT32 nVirtNum,gctUINT32 * pPhysAddr);
	gceSTATUS NV2Grey(sgks_mpi_cap_yuv_info_s SrcImage,gctPOINTER pDstGrey,int nDstW,int nDstH);
	gceSTATUS RenderSurface(gcsRECT srcRC,gcsRECT frgRC,gceSURF_ROTATION rotate);
	gceSTATUS FormatConvert(void);
	gceSTATUS ConfigSurface(unsigned int nSurfIndex, gctPOINTER * pVirtAddr, gctUINT32 nSize[],gctUINT32 nVirtNum,int Width,int Height,int nMaxVirts,gceSURF_FORMAT surFormat);
private:
	gceSTATUS ValidateSurface(SGKS_SURFACE * pSGKSurf,gctUINT32	Width,gctUINT32	Height,gctUINT32 nMaxMeminfo,gceSURF_FORMAT surFormat);
};
#endif
/* _GPU2D_HPP_ */
