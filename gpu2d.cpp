#include "gpu2d.h"
#include "HAL/gc_hal_base.h"
#include "HAL/gc_hal_types.h"
#include "HAL/gc_hal.h"
#include "HAL/gc_hal_raster.h"
#include <pthread.h>
#include <sys/prctl.h>

GPU2D::GPU2D()
{
	gceSTATUS status;
	created = gcvSTATUS_FALSE;
	os = gcvNULL;
	hal= gcvNULL;
	egn2D=gcvNULL;
	format = gcvSURF_UNKNOWN;
	pUVPlane = gcvNULL;
	memset(&srcSurf, 0x00, sizeof(SGKS_SURFACE));
	memset(&frgSurf, 0x00, sizeof(SGKS_SURFACE));
	memset(&dstSurf, 0x00, sizeof(SGKS_SURFACE));
	/* Construct the gcoOS object. */
	status = gcoOS_Construct(gcvNULL, &os);
	if (status >= 0)
	{
		/* Construct the gcoHAL object. */
		status = gcoHAL_Construct(gcvNULL, os, &hal);
		if (status >= 0)
		{
			status = gcoHAL_Get2DEngine(hal, &egn2D);
			created= status;
			pUVPlane = new gctUINT8[1920*1080/2];
		}
	}
}

GPU2D::~GPU2D()
{
	/*
	for(unsigned int i = 0;i<nMaxMemInfos;i++)
	{
		if(meminfo[i].VirtAddr!=gcvNULL)
			gcoHAL_UnmapUserMemory(meminfo[i].VirtAddr,meminfo[i].Size,&meminfo[i].Info,(gctUINT32)meminfo[i].PhysAddr);
	}
	delete []meminfo;
	meminfo = NULL;
	*/
	if(pUVPlane)
	{
		delete []pUVPlane;
		pUVPlane = gcvNULL;
	}
	if(created)
	{
		gcoHAL_DestroyEx(hal);
		gcoOS_Destroy(os);
		created = gcvSTATUS_FALSE;
	}
}
gceSTATUS GPU2D::GetPhyAddrFromVirt(
		SGKS_SURFACE * pSGKSurf,
		gctPOINTER * pVirtAddr,
		gctUINT32 nSize[],
		gctUINT32 nVirtNum,
		gctUINT32 * pPhysAddr)
{
	gceSTATUS	status = gcvSTATUS_FALSE;
	gctBOOL		bRemap = gcvTRUE;
	unsigned int 		i,j;

	if(nVirtNum < 3) //YUV format is less than 3 Planes.
	{
		for(i = 0;i<nVirtNum;i++)
		{
			bRemap = gcvTRUE;
			for(j = 0;j<pSGKSurf->MaxMemInfo;j++)
			{
				if((pSGKSurf->MemInfo[j].VirtAddr == pVirtAddr[i])&&(pSGKSurf->MemInfo[j].VirtAddr!=gcvNULL))
				{
					if(nSize[i] != pSGKSurf->MemInfo[j].Size)
					{
						gcmONERROR(gcoHAL_UnmapUserMemory(pSGKSurf->MemInfo[j].VirtAddr,pSGKSurf->MemInfo[j].Size,&pSGKSurf->MemInfo[j].Info,pSGKSurf->MemInfo[j].PhysAddr));
						memset(&pSGKSurf->MemInfo[j],0x00,sizeof(MEM_INFO));
					}
					else
						bRemap = gcvFALSE;
					break;
				}
			}
			if(bRemap == gcvTRUE)
			{
				for(j = 0;j<pSGKSurf->MaxMemInfo;j++)
				{
					if(pSGKSurf->MemInfo[j].VirtAddr == gcvNULL)
					{
						pSGKSurf->MemInfo[j].Size = nSize[i];
						gcmONERROR(gcoHAL_MapUserMemory(pVirtAddr[i], gcvINVALID_ADDRESS,nSize[i],(void **)&pSGKSurf->MemInfo[j].Info,(unsigned int *)&pSGKSurf->MemInfo[j].PhysAddr));
						pSGKSurf->MemInfo[j].VirtAddr = pVirtAddr[i];
						bRemap = gcvFALSE;
						break;
					}
				}
			}
			status = (bRemap ==gcvFALSE)?gcvSTATUS_TRUE:gcvSTATUS_FALSE;
			if(pPhysAddr!=NULL)
				pPhysAddr[i] = (gctUINT32)pSGKSurf->MemInfo[j].PhysAddr;
			else
				pSGKSurf->PhysAddr[i] = (gctUINT32)pSGKSurf->MemInfo[j].PhysAddr;
		}
	}
	return status;
OnError:
	printf("%s(%d) failed %d %d\n",__FUNCTION__, __LINE__, status, nSize[0]);
	return status;
}
gceSTATUS GPU2D::NV2Grey(sgks_mpi_cap_yuv_info_s SrcImage,gctPOINTER pDstGrey, int nDstW,int nDstH)
{
	gceSTATUS 	status = gcvSTATUS_FALSE;
	gctUINT32	srcVirtAddr[3] = {(gctUINT32)SrcImage.yAddr, (gctUINT32)SrcImage.uvAddr, gcvNULL};
	gctUINT32	srcPhysAddr[3] = {gcvNULL}, srcStride[3] = {0};
	gctUINT32	dstPhysAddr[2] = {gcvNULL}, dstStride[3] = {0};
	gctPOINTER	dstVirtAddr[2] = {pDstGrey,pUVPlane};
	gctUINT32	srcStridNum;
	gcsRECT		srcRC,dstRC;
	gctUINT32	nSize[3];
	gctUINT32	dw;
	gceSURF_FORMAT	nFormat;


	switch(SrcImage.frameFormat)
	{
		case SGKS_CAP_YUV_SEMIPLANAR_420:
			srcStridNum = 2;
			srcStride[0] = (SrcImage.width+0x3f)&0xFFFFFFC0;
			srcStride[1] = ((SrcImage.width>>1)+0x3f)&0xFFFFFFC0;
			nSize[0] = srcStride[0] * SrcImage.height;
			nSize[1] = srcStride[1] * SrcImage.height;
			nFormat = gcvSURF_NV12;
			break;
		case SGKS_CAP_YUV_SEMIPLANAR_422:
			srcStridNum = 1;
			srcStride[0] = (SrcImage.width+0x3f)&0xFFFFFFC0;
			nSize[0] = srcStride[0] * SrcImage.height;
			nSize[1] = nSize[0];
			nFormat = gcvSURF_NV16;
			break;
		default:
			return gcvSTATUS_NOT_SUPPORTED;
			break;
	}
	//Default settings: the image is horizontal.
	dstRC.left	= 0;
	dstRC.right	= nDstW;
	dstRC.top	= 0;
	dstRC.bottom= nDstH;

	dw = nDstW* SrcImage.height / nDstH;
	srcRC.left  = (SrcImage.width - dw)/2;
	srcRC.right = dw;
	srcRC.top	= 0;
	srcRC.bottom= SrcImage.height;

	GetPhyAddrFromVirt(&srcSurf,(void **)&srcVirtAddr,nSize,2,(gctUINT32 *) &srcPhysAddr[0]);

	nSize[0] = ((nDstW+0x3F)&0xFFFFFFC0) * nDstH;
	nSize[1] = nSize[0]>>1;
	GetPhyAddrFromVirt(&dstSurf,(void**)&dstVirtAddr[0],nSize,2, (gctUINT32*)&dstPhysAddr[0]);

	dstStride[0] = (nDstW+0x3F)&0xFFFFFFC0;
	dstStride[1] = ((nDstW>>1)+0x3F)&0xFFFFFFC0;
	gcmONERROR(gco2D_FilterBlitEx2(egn2D,
				(gctUINT32_PTR)&srcPhysAddr,2,
				(gctUINT32_PTR)&srcStride, srcStridNum,
				gcvLINEAR,nFormat, gcvSURF_0_DEGREE,
				SrcImage.width, SrcImage.height, &srcRC,
				(gctUINT32_PTR)&dstPhysAddr, 2,
				(gctUINT32_PTR)&dstStride, 2,
				gcvLINEAR, gcvSURF_NV12,  gcvSURF_0_DEGREE,
				nDstW,nDstH,
				&dstRC, gcvNULL));
	gcmONERROR(gco2D_Flush(egn2D));
	gcmONERROR(gcoHAL_Commit(hal, gcvTRUE));
	return status;
OnError:
	printf("%s(%d) failed\n",__FUNCTION__, __LINE__);
	return status;
}
gceSTATUS GPU2D::ConfigSurface(
		unsigned int	nSurfIndex,
		gctPOINTER*		pVirtAddr,
		gctUINT32		nSize[],
		gctUINT32		nVirtNum,
		int				Width,
		int				Height,
		int				nMaxVirts,
		gceSURF_FORMAT	surFormat)
{
	SGKS_SURFACE * 	pSurf[]={&srcSurf,&frgSurf,&dstSurf};
	if(nSurfIndex>=(sizeof(pSurf)/sizeof(SGKS_SURFACE *)))
		return gcvSTATUS_FALSE;
	ValidateSurface(pSurf[nSurfIndex],Width,Height,16,surFormat);
	GetPhyAddrFromVirt(pSurf[nSurfIndex],pVirtAddr, nSize, nVirtNum, NULL);
	return gcvSTATUS_TRUE;
}
gceSTATUS GPU2D::ValidateSurface(
		SGKS_SURFACE * pSGKSurf,
		gctUINT32	Width,
		gctUINT32	Height,
		gctUINT32	nMaxMeminfo,
		gceSURF_FORMAT surFormat)
{
	gceSTATUS	status = gcvSTATUS_FALSE;
	gctUINT32	alignedWidth = (Width + 0x3F)&0xFFFFFFC0;
	gctUINT32	alignedHeight= (Height + 0x3)&0xFFFFFFFC;
	if(pSGKSurf->bInited == gcvTRUE)
	{
		//Check the properties is not changed.
		if((pSGKSurf->AlignedWidth != alignedWidth)
				|| (pSGKSurf->AlignedHeight != alignedHeight)
				||(pSGKSurf->Format != surFormat))
		{
			if(pSGKSurf->Surf!=gcvNULL)
			{
				gcoSURF_Destroy(pSGKSurf->Surf);
				pSGKSurf->Surf = gcvNULL;
			}
			pSGKSurf->bInited = gcvFALSE;
		}
		else
			return gcvSTATUS_OK;
	}
	if(pSGKSurf->MemInfo)
	{
		for(unsigned int i = 0;i<pSGKSurf->MaxMemInfo;i++)
		{
			if(pSGKSurf->MemInfo[i].VirtAddr!=gcvNULL)
				gcoHAL_UnmapUserMemory(pSGKSurf->MemInfo[i].VirtAddr,pSGKSurf->MemInfo[i].Size,&pSGKSurf->MemInfo[i].Info,(gctUINT32)pSGKSurf->MemInfo[i].PhysAddr);
		}
		free(pSGKSurf->MemInfo);
	}
	pSGKSurf->MaxMemInfo = nMaxMeminfo;
	pSGKSurf->MemInfo	 = (MEM_INFO*)malloc(sizeof(MEM_INFO)*nMaxMeminfo);
	if(pSGKSurf->MemInfo)
	{
		memset(pSGKSurf->MemInfo,0x00, sizeof(MEM_INFO)*nMaxMeminfo);
		gcmONERROR(gcoSURF_Construct(hal,alignedWidth , alignedHeight, 1, gcvSURF_BITMAP,surFormat, gcvPOOL_DEFAULT, &pSGKSurf->Surf));
		gcmONERROR(gcoSURF_GetAlignedSize(pSGKSurf->Surf,
			&pSGKSurf->AlignedWidth,
			&pSGKSurf->AlignedHeight,
			(gctINT*)pSGKSurf->Stride));
		pSGKSurf->Width = Width;
		pSGKSurf->Height= Height;
		gcmONERROR(gcoSURF_Lock(pSGKSurf->Surf,
				&pSGKSurf->PhysAddr[0],
				&pSGKSurf->VirtAddr[0]));
		gcmONERROR(gcoSURF_GetFormat(pSGKSurf->Surf, gcvNULL, &pSGKSurf->Format));
		switch(surFormat)
		{
		case gcvSURF_NV12:
			pSGKSurf->AddressNum= 2;
			pSGKSurf->StrideNum = 2;
			pSGKSurf->Stride[1] = pSGKSurf->Stride[0]>>1;
			break;
		case gcvSURF_NV16:
			pSGKSurf->StrideNum = 2;
			pSGKSurf->AddressNum= 2;
			pSGKSurf->Stride[1] = pSGKSurf->Stride[0];
			break;
		case gcvSURF_A8R8G8B8:
		case gcvSURF_R5G6B5:
		case gcvSURF_A1R5G5B5:
		case gcvSURF_YUY2:
			pSGKSurf->StrideNum = 1;
			pSGKSurf->AddressNum= 1;
			break;
		default:
			break;
		}
		pSGKSurf->bInited = gcvTRUE;
	}
OnError:
	if((pSGKSurf == &srcSurf)
		||(pSGKSurf == &frgSurf)
		||(pSGKSurf == &dstSurf))
	{
		gcoSURF_Destroy(pSGKSurf->Surf);
	}
	return status;
}
gceSTATUS GPU2D::RenderSurface(gcsRECT srcRC,gcsRECT frgRC,gceSURF_ROTATION rotate)
{
	gctUINT8    srcAlpha=0xff,dstAlpha=0xE0;
	gceSTATUS	status = gcvSTATUS_OK;
	gcsRECT 	dstRC={0};
	gctUINT		w,h;

	gceSURF_ROTATION	srcRot;
	LPSGKS_SURFACE		pSrcSurf;

	if(dstSurf.Width>dstSurf.Height)
	{
		w = dstSurf.Width;
		h = dstSurf.Height;
	}
	else
	{
		h = dstSurf.Width;
		w = dstSurf.Height;
	}
	ValidateSurface(&bmpSurf, w, w, 1,gcvSURF_R5G6B5);
	ValidateSurface(&tmpSurf, w, h, 1,gcvSURF_R5G6B5);
	dstRC.left	= 0;
	dstRC.top	= 0;
	dstRC.right	= tmpSurf.Width;
	dstRC.bottom= tmpSurf.Height;

	//Scaling & format converting to prepare the background.
	gcmONERROR(gco2D_SetClipping(egn2D, &dstRC));
	gcmONERROR(gco2D_SetSource(egn2D, &srcRC));
	gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 5));
	gcmONERROR(gco2D_EnableDither(egn2D, gcvTRUE));
	gcmONERROR(gco2D_FilterBlitEx2(egn2D,
			srcSurf.PhysAddr,srcSurf.AddressNum,
			srcSurf.Stride,srcSurf.StrideNum,
			gcvLINEAR, srcSurf.Format,gcvSURF_0_DEGREE,
			srcSurf.AlignedWidth, srcSurf.AlignedHeight, &srcRC,
			tmpSurf.PhysAddr, tmpSurf.AddressNum,
			tmpSurf.Stride, tmpSurf.StrideNum,
			gcvLINEAR, tmpSurf.Format,  gcvSURF_0_DEGREE,
			tmpSurf.AlignedWidth,  tmpSurf.AlignedHeight,
			&dstRC, gcvNULL));
	if(frgSurf.bInited == gcvTRUE)
	{
		srcRC.right = frgSurf.Width;
		srcRC.bottom= frgSurf.Height;
		gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(egn2D, srcAlpha << 24));
		gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(egn2D, dstAlpha << 24));
		gcmONERROR(gco2D_EnableAlphaBlendAdvanced(egn2D,
					gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,
					gcvSURF_GLOBAL_ALPHA_ON, gcvSURF_GLOBAL_ALPHA_ON,
					gcvSURF_BLEND_STRAIGHT, gcvSURF_BLEND_STRAIGHT));

		gcmONERROR(gco2D_FilterBlitEx2(egn2D,
				frgSurf.PhysAddr,frgSurf.AddressNum,
				frgSurf.Stride,frgSurf.StrideNum,
				gcvLINEAR, frgSurf.Format,gcvSURF_0_DEGREE,
				frgSurf.AlignedWidth, frgSurf.AlignedHeight, &srcRC,
				tmpSurf.PhysAddr, tmpSurf.AddressNum,
				tmpSurf.Stride, tmpSurf.StrideNum,
				gcvLINEAR, tmpSurf.Format,  gcvSURF_0_DEGREE,
				tmpSurf.AlignedWidth,  tmpSurf.AlignedHeight,
				&dstRC, gcvNULL));

		// disalbe alphablend
		gcmONERROR(gco2D_DisableAlphaBlend(egn2D));
		gcmONERROR(gco2D_Flush(egn2D));
		gcmONERROR(gcoHAL_Commit(hal, gcvTRUE));
	}
//-----------------------------------------
	pSrcSurf = &tmpSurf;
	srcRC.left	= 0;
	srcRC.top	= 0;
	srcRC.right	= tmpSurf.Width;
	srcRC.bottom= tmpSurf.Height;
	srcRot = rotate;
	switch(rotate)
	{
	case gcvSURF_270_DEGREE:
		/*
		 * 270度旋转，先把图像X/Y均做一次镜像，实现180度旋转，然后再旋转90度。
		 * 1080P图像的处理性能与旋转90度几乎一样，在毫秒级别的差距。
		 */
		srcRot = gcvSURF_90_DEGREE;
		gcmONERROR(gco2D_SetBitBlitMirror(egn2D,gcvTRUE,gcvTRUE));
	case gcvSURF_90_DEGREE:
		/*
		 * 由于gco2D_FilterBlitEx2实现90/270度旋转有bug，不能得到正确的结果，
		 * 因此,用Blit实现90/270度旋转，然后再进行RGB转NV16的操作。
		 */
		//Rotate the image to big bitmap surface:bmpSurface;
		srcRC.right	= pSrcSurf->Width;
		srcRC.bottom= pSrcSurf->Height;

		dstRC.right	= pSrcSurf->Height;
		dstRC.bottom= pSrcSurf->Width;

		gcmONERROR(gco2D_SetColorSource(egn2D,
				pSrcSurf->PhysAddr[0],
				pSrcSurf->Stride[0],
				pSrcSurf->Format,
				srcRot,
				pSrcSurf->AlignedWidth,
				gcvFALSE,
				gcvSURF_OPAQUE,
				0));
		gcmONERROR(gco2D_SetSource(egn2D, &srcRC));
		gcmONERROR(gco2D_SetTarget(egn2D,
									bmpSurf.PhysAddr[0],
									bmpSurf.Stride[0],
									gcvSURF_0_DEGREE,
									bmpSurf.AlignedWidth));
		gcmONERROR(gco2D_SetClipping(egn2D, &dstRC));
		gcmONERROR(gco2D_Blit(egn2D,
								1,
								&dstRC,
								0xCC,
								0xCC,
								bmpSurf.Format));
		pSrcSurf= &bmpSurf;
		srcRC.top 	= pSrcSurf->AlignedWidth - pSrcSurf->Width;
		srcRC.right	= dstRC.right;//pSrcSurf->Height;
		srcRC.bottom= dstRC.bottom;//pSrcSurf->Width;
		break;
	case gcvSURF_180_DEGREE:
		//180旋转，在操作上，只是X/Y均做镜像即可。
		gcmONERROR(gco2D_SetBitBlitMirror(egn2D,gcvTRUE,gcvTRUE));
		break;
	case gcvSURF_0_DEGREE:
	default:
		break;
	}
/*
	gcmONERROR(gco2D_Flush(egn2D));
	gcmONERROR(gcoHAL_Commit(hal, gcvTRUE));
	//sprintf(filename,"%dx%d.bmp",rt.lcdSurf.AlignedWidth,rt.lcdSurf.AlignedHeight);
	GalSaveDIB(bmpSurf.VirtAddr[0],bmpSurf.Format,bmpSurf.Stride[0],bmpSurf.AlignedWidth,bmpSurf.AlignedHeight,"abc.bmp");
*/
	dstRC.right	= dstSurf.Width;
	dstRC.bottom= dstSurf.Height;
	gcmONERROR(gco2D_SetClipping(egn2D, &srcRC));
	gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 5));
	gcmONERROR(gco2D_FilterBlitEx2(egn2D,
			pSrcSurf->PhysAddr, pSrcSurf->AddressNum,
			pSrcSurf->Stride,pSrcSurf->StrideNum,
			gcvLINEAR, pSrcSurf->Format,gcvSURF_0_DEGREE,
			pSrcSurf->AlignedWidth, pSrcSurf->AlignedHeight, &srcRC,
			dstSurf.PhysAddr, dstSurf.AddressNum,
			dstSurf.Stride, dstSurf.StrideNum,
			gcvLINEAR, dstSurf.Format,  gcvSURF_0_DEGREE,//t2d.dstTemp->rotation,
			dstSurf.AlignedWidth,  dstSurf.AlignedHeight,
			&dstRC, &dstRC));
	gcmONERROR(gco2D_Flush(egn2D));
	gcmONERROR(gcoHAL_Commit(hal, gcvTRUE));
	return status;
OnError:
	printf("%s(%d) failed\n",__FUNCTION__, __LINE__);
	return status;
}
gceSTATUS GPU2D::FormatConvert(void)
{
	gceSTATUS	status = gcvSTATUS_OK;
	gcsRECT		srcRC={0};
	srcRC.right = srcSurf.AlignedWidth;
	srcRC.bottom= srcSurf.AlignedHeight;
	gcmONERROR(gco2D_SetClipping(egn2D, &srcRC));
	gcmONERROR(gco2D_SetKernelSize(egn2D, 5, 5));
	gcmONERROR(gco2D_FilterBlitEx2(egn2D,
			srcSurf.PhysAddr,srcSurf.AddressNum,
			srcSurf.Stride,srcSurf.StrideNum,
			gcvLINEAR, srcSurf.Format,gcvSURF_0_DEGREE,
			srcSurf.AlignedWidth, srcSurf.AlignedHeight, &srcRC,
			dstSurf.PhysAddr, dstSurf.AddressNum,
			dstSurf.Stride, dstSurf.StrideNum,
			gcvLINEAR, dstSurf.Format,  gcvSURF_0_DEGREE,//t2d.dstTemp->rotation,
			dstSurf.AlignedWidth,  dstSurf.AlignedHeight,
			&srcRC, &srcRC));
	gcmONERROR(gco2D_Flush(egn2D));
	gcmONERROR(gcoHAL_Commit(hal, gcvTRUE));
	return status;
OnError:
	printf("%s(%d) failed %d\n",__FUNCTION__, __LINE__,status);
	return status;
}
