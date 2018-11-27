#include "panorama.h"
#include <opencv2/opencv.hpp>
#include "parameter.h"
#include <time.h> 
#include "mpi/mpi_lib.h"
#include "gpu2d.h"
#include "omp.h"

static Mat Map_Fx = Mat(image_size, CV_32FC1);
static Mat Map_Fy = Mat(image_size, CV_32FC1);
static Mat Map_Rx = Mat(image_size, CV_32FC1);
static Mat Map_Ry = Mat(image_size, CV_32FC1);
static Mat front_image(720, 1280, CV_8UC4);
static Mat rear_image(720, 1280, CV_8UC4);

#define Size_Out_AGRB 640*320*4


static char *ptr = NULL;
static char *pData=NULL;
static char * out_buf = (char *)malloc(sizeof(char) * Size_Out_AGRB);


Mat frontimage,  rearimage;
Mat front_mask, rear_mask;
Mat front_mask1, rear_mask1;
Mat frontMat,rearMat;
using namespace std;
using namespace cv;
bool init_ = true;
static Panorama pa;

static Mat front_trs(image_size, CV_8UC4, Scalar::all(0));
static Mat rear_trs(image_size, CV_8UC4, Scalar::all(0));
static Mat out= Mat(Size(space_x, space_y), CV_8UC4, Scalar::all(0));


gctUINT			nLCDWidth = 480;
gctUINT			nLCDHeight = 854;

void GetMapForRemap(Mat matrix[(grid_rows - 1)*(grid_cols - 1)],Mat Map_Fx, Mat Map_Fy)
{

	Mat output;

	for (size_t i = 0; i < (grid_rows - 1)*(grid_cols - 1); i++)
	{
		matrix[i] = matrix[i].inv();
	}

	Point2f po;

	for (size_t p = 0; p < grid_cols - 1; p++)
	{
		for (size_t q = 0; q < grid_rows - 1; q++)
		{
			vector<Point2f> Map_FP;
			vector<Point2f> SRC;
			for (float i = (grid_size) * p; i < (grid_size)*(p + 1); i++)
			{
				for (float j = (grid_size) * q; j < (grid_size)*(q + 1); j++)
				{
					po.x = j;
					po.y = i;
					SRC.push_back(po);
				}
			}
			perspectiveTransform(SRC, Map_FP, matrix[p*(grid_rows - 1) + q]);
			int idpix = 0;
			for (float i = grid_size * p; i < grid_size*(p + 1); i++)
			{
				for (float j = grid_size * q; j < grid_size*(q + 1); j++)
				{
					idpix = (i - grid_size * p) * grid_size + (j - grid_size * q);
					Map_Fx.at<float>(i, j) = Map_FP[idpix].x;
					Map_Fy.at<float>(i, j) = Map_FP[idpix].y;
				}
			}
			Map_FP.clear();
		}
	}	
}



void ShowImageToLCD(Mat img)
{
	static GPU2D	gpu;
	static bool bInited = false;
	sgks_mpi_vo_DisBankParams_s disbank = { 0 };
	gctPOINTER 		pVirt[3];
	gctUINT32		nSize[3];
	gcsRECT			srcRC = { 0 };
	gcsRECT			frgRC = { 0 };

	static unsigned char *ptr = NULL;
	static unsigned char *pData=NULL;

	if(ptr == NULL)
	{
		ptr = new unsigned char[img.cols * img.rows *4+1024];
		pData = (unsigned char *)(((unsigned int)ptr + 0x3F)&0xFFFFFFC0);
	}
	memcpy(pData, img.datastart,img.cols * img.rows *4);
	if (bInited == false)
	{
		//GetLCDSize(&nLCDWidth, &nLCDHeight);
		//gpu = new GPU2D();
		bInited = true;
	}
	srcRC.left = 0;
	srcRC.top = 0;
	srcRC.right = img.cols;
	srcRC.bottom = img.rows;
	pVirt[0] = pData;//(gctPOINTER)img.datastart;
	nSize[0] = img.cols * img.rows * 4;
	nSize[1] = 0;

	gpu.ConfigSurface(0, pVirt, nSize, 1, srcRC.right, srcRC.bottom, 8, gcvSURF_A8R8G8B8);

	sgks_mpi_vo_GetFreeBank(&disbank);
	
	pVirt[0] = (gctPOINTER *)disbank.bankaddr;
	pVirt[1] = (gctPOINTER *)((u8*)disbank.bankaddr + (disbank.banksize >> 1));
	nSize[0] = disbank.banksize >> 1;
	nSize[1] = nSize[0];

	gpu.ConfigSurface(2, pVirt, nSize, 2, nLCDWidth, nLCDHeight, 16, gcvSURF_NV16);
	gpu.RenderSurface(srcRC, frgRC, gcvSURF_90_DEGREE);
	int pitch = 896;//(srcRC.right +0x3F)&0xFFFFFFC0;

	sgks_mpi_vo_UpdateBank(&disbank);
}


Mat Mask[grid_rows * grid_cols];
Mat matrix_affine[grid_rows*grid_cols];
Mat matrix_affine_r[grid_rows*grid_cols];
Mat result[grid_rows*grid_cols];
Mat result_r[grid_rows*grid_cols];

void get_Univariate_matrix(void)
{
#if 1
		Mat img = imread("F.bmp",0);
		Mat img_r = imread("B.bmp",0);
		vector<Point2f> corners;
		vector<Point2f> corners_r;
        vector<Point2f> corner_tmp;
		cout << findChessboardCorners(img, CALIBRATOR_BOARD_SIZE, corners)<< endl;

		cout << findChessboardCorners(img_r, CALIBRATOR_BOARD_SIZE, corners_r) << endl;
		TermCriteria criteria = TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 40, 0.1);
		cornerSubPix(img, corners, Size(5,5), Size(-1,-1), criteria);
		cornerSubPix(img_r, corners_r, Size(5,5), Size(-1, -1), criteria);
        for(int i = corners_r.size() - 1; i > -1; i--)
        {
            corner_tmp.push_back(corners_r[i]);
        }
        corners_r = corner_tmp;
#else
	
		ifstream myfile("F1F.txt");
		ifstream myfile_R("B1B.txt");
		string temp;
		string temp_R;
		if (!myfile.is_open())
		{
			cout << "未成功打开文件" << endl;
		}
		if (!myfile_R.is_open())
		{
			cout << "未成功打开文件" << endl;
		}
		vector<Point2f> corners;
		vector<Point2f> corners_r;
		int index = 0;
		while (getline(myfile, temp))
		{
				Point2f p1;
				int a[2] = { 0 };
				istringstream iss;//istringstream提供读 string 的功能
				iss.str(temp);//将 string 类型的 test 复制给 iss，返回 void
				string s;
				int i = 0;
				while (iss >> s) 
				{
					int x = stoi(s);
					a[i] = x;
					i++;
				}
				p1.x = a[0];
				p1.y = a[1];
				corners.push_back(p1);
			index++;
		}
		
		int index_r = 0;
		while (getline(myfile_R, temp_R))
		{
				Point2f p1;
				int a[2] = { 0 };
				istringstream iss;//istringstream提供读 string 的功能
				iss.str(temp_R);//将 string 类型的 test 复制给 iss，返回 void
				string s;
				int i = 0;
				while (iss >> s) 
				{
					int x = stoi(s);
					a[i] = x;
					i++;
				}
				p1.x = a[0];
				p1.y = a[1];
				corners_r.push_back(p1);
			index_r++;
		}
	cout<< corners_r << endl;
#endif		
		for (int i = 0; i < 9; i++)
		{
			Mat mode = Mat::zeros(dstImg_cols, dstImg_rows, CV_8UC1);
			Mat mode1 = Mat::zeros(dstImg_cols, dstImg_rows, CV_8UC1);
			Mat mode2 = Mat::zeros(dstImg_cols, dstImg_rows, CV_8UC1);
			Mat mask_mini = Mat::ones(grid_size, grid_size, CV_8UC1);
			Mat ROI;
			ROI = mode(Rect(i*grid_size, 0, grid_size, grid_size));
			mask_mini.copyTo(ROI);
			Mask[i] = mode;
			Mat ROI1;
			ROI1 = mode1(Rect(i * grid_size, grid_size, grid_size, grid_size));
			mask_mini.copyTo(ROI1);
			Mask[i + grid_rows] = mode1;
			Mat ROI2;
			ROI2 = mode2(Rect(i * grid_size, grid_size * 2, grid_size, grid_size));
			mask_mini.copyTo(ROI2);
			Mask[i + grid_rows * 2] = mode2;
		}
	
		Point2f Src[grid_cols*grid_rows], Dst[grid_cols*grid_rows], Src_r[grid_cols*grid_rows];
		for (int i = 0; i < grid_cols*grid_rows; i++)
		{
			Src[i] = corners[i];
			Src_r[i] = corners_r[i];
		}
		for (int i = 0; i < grid_rows; i++)
		{
			if (i == 0)
			{
				for(int j = 0; j < grid_cols; j++)
				{
					Dst[i + grid_rows * j].x = 0;
				}
			}
			else
			{
				for(int j = 0; j < grid_cols; j++)
				{
					Dst[i + grid_rows * j].x = i * grid_size - 1;
				}
			}
			Dst[i].y = 0;
			for (int j = 1; j < grid_cols; j++)
			{
				Dst[i + grid_rows * j].y = grid_size * j - 1;;
			}
		}
		vector<Point2f> Dsst, Test;
		for (int i = 0; i < grid_cols*grid_rows; i++)
		{
			Dsst.push_back(Dst[i]);
		}
		for (int i = 0; i < grid_rows-1; i++)
		{
			Point2f m[4], n[4], m_r[4];
			for(int j = 0; j < grid_cols - 1; j++)
			{
				m[0] = corners[grid_rows * j + i];
				m[1] = corners[grid_rows * j + i + 1];
				m[2] = corners[grid_rows * (j + 1) + i];
				m[3] = corners[grid_rows * (j + 1) + 1 + i];
				m_r[0] = corners_r[grid_rows * j + i];
				m_r[1] = corners_r[grid_rows * j + i + 1];
				m_r[2] = corners_r[grid_rows * (j + 1) + i];
				m_r[3] = corners_r[grid_rows * (j + 1) + 1 + i];
				n[0] = Dst[grid_rows * j + i];
				n[1] = Dst[grid_rows * j + i + 1];
				n[2] = Dst[grid_rows * (j + 1) + i];
				n[3] = Dst[grid_rows * (j + 1) + 1 + i];
				matrix_affine_r[i + (grid_rows - 1) * j] = getPerspectiveTransform(m_r, n);
				matrix_affine[i + (grid_rows - 1) * j] = getPerspectiveTransform(m, n);	
			}

		}

}

static mpi_mem_map_table_s g_mem_map_table[] =
{
	/*************** fw base ***************/
{ dsp_init_param,         ADDR_AUTO,  0x1000 },
{ dsp_log,                ADDR_AUTO,  0x1E000 },
{ fw_orce,                ADDR_AUTO,  0x200000 },
{ fw_me,                  ADDR_AUTO,  0x40000 },
{ fw_binary,              ADDR_AUTO,  0x40000 },

/*************** fw config base**************/
{ dsp2_info,              ADDR_AUTO,  0x1000 },
{ dsp_chip,               ADDR_AUTO,  0x1000 },
{ dsp_default_cmd,        ADDR_AUTO,  0x1000 },
{ dsp_normal_cmd,         ADDR_AUTO,  0x1000 },
{ dsp_result_queue,       ADDR_AUTO,  0x1000 },
{ bsb_desc_addr,          ADDR_AUTO,  0x4000 },
{ msg_addr_arm_osd,       ADDR_AUTO,  0x1000 },
{ display_vouta_addr,     ADDR_AUTO,  0x1000 },
{ display_voutb_addr,     ADDR_AUTO,  0x1000 },
{ osd_dram_addr,          ADDR_AUTO,  0x1000 },
{ osd_clut_dram_addr,     ADDR_AUTO,  0x1000 },
{ tv_dram_addr,           ADDR_AUTO,  0x1000 },
{ disp_bank_addr,         ADDR_AUTO,  0x2F1000 },
/*************** bsb base***************/
{ bsb_buff,               ADDR_AUTO,  0x400000 },

/*************** dsp base ***************/
{ dsp_buff,               ADDR_AUTO,  0x02000000 },

/***************isp base***************/
{ isp_aaa_fifo1_base,                    ADDR_AUTO,  0x2000 },/*2048*4*/
{ isp_aaa_fifo2_base,                    ADDR_AUTO,  0xd000 },/*12416*4*/
{ isp_input_lookup_table,                ADDR_AUTO,  0x1000 },/*192*3*4*/
{ isp_matrix_dram_address,               ADDR_AUTO,  0x4000 },/*16*16*16*4*/
{ isp_output_lookup_table,               ADDR_AUTO,  0x1000 },/*256*4*/
{ isp_chroma_gain_curve,                 ADDR_AUTO,  0x1000 },/*128*2*/
{ isp_hot_pixel_thd_table,               ADDR_AUTO,  0x1000 },/*3*128*2*/
{ isp_dark_pixel_thd_table,              ADDR_AUTO,  0x1000 },/*3*128*2*/
{ isp_mctf_cfg_addr,                     ADDR_AUTO,  0x1000 },/*528*/
{ isp_k0123_table,                       ADDR_AUTO,  0x1000 },/*24*2*/
{ isp_exposure_gain_curve,               ADDR_AUTO,  0x1000 },/*256*2*/
{ isp_luma_sharpening_alpha_table,       ADDR_AUTO,  0x1000 },/*512*/
{ isp_coeff_fir1_addr,                   ADDR_AUTO,  0x1000 },/*256*/
{ isp_coeff_fir2_addr,                   ADDR_AUTO,  0x1000 },/*256*/
{ isp_coring_table,                      ADDR_AUTO,  0x1000 },/*256*/
{ isp_vignette_r_gain,                   ADDR_AUTO,  0x1000 },/*33*33*2*/
{ isp_vignette_go_gain,                  ADDR_AUTO,  0x1000 },/*33*33*2*/
{ isp_vignette_ge_gain,                  ADDR_AUTO,  0x1000 },/*33*33*2*/
{ isp_vignette_b_gain,                   ADDR_AUTO,  0x1000 },/*33*33*2*/
{ isp_pixel_map_addr,                    ADDR_AUTO,  0xC0000 },/*384*1944*/
{ isp_fpn_reg_addr,                      ADDR_AUTO,  0x1000 },/*1024*/
{ isp_hor_still,                         ADDR_AUTO,  0x3000 },/*64*94*2*/
{ isp_ver_still,                         ADDR_AUTO,  0x3000 },/*64*94*2*/
{ isp_eis_enhance_turbo_buf,             ADDR_AUTO,  0x1000 },/*192*/
};
int bsv_VinInit(int nImgWidth, int nImgHeight, int nCapWidth, int nCapHeight, int nCapX, int nCapY, sgks_mpi_venc_framerate_e nFPS)
{
	sgks_mpi_vi_device_s vi_dev;

	memset(&vi_dev, 0, sizeof(sgks_mpi_vi_device_s));
	vi_dev.vi_device_num = 1;

	vi_dev.vi_deviceinfo[0].enable = 1;
	vi_dev.vi_deviceinfo[0].device_id = 0;
	vi_dev.vi_deviceinfo[0].vi_type = SGKS_VI_TYPE_VIN;
	vi_dev.vi_deviceinfo[0].interface_type = SGKS_VI_INTERFACE_MIPI;

	// ���ʹ��Ĭ�Ϸֱ��ʣ���������cap_width = 0, cap_height = 0, cap_fps = 0,
	vi_dev.vi_deviceinfo[0].vi_width = nImgWidth;	//1920;
	vi_dev.vi_deviceinfo[0].vi_height = nImgHeight;	//2160;
	vi_dev.vi_deviceinfo[0].vi_fps = nFPS;			//30;

	vi_dev.vi_deviceinfo[0].cap_start_x = nCapX;		//0;
	vi_dev.vi_deviceinfo[0].cap_start_y = nCapY;		//540;
	vi_dev.vi_deviceinfo[0].cap_width = nCapWidth;	//1920;
	vi_dev.vi_deviceinfo[0].cap_height = nCapHeight;	//1080;

	vi_dev.vi_deviceinfo[0].mirror_mode = SGKS_MPI_VI_MIRROR_AUTO;
	vi_dev.vi_deviceinfo[0].bayer_pattern = SGKS_MPI_VI_BAYER_PATTERN_AUTO;

	sprintf((char*)vi_dev.vi_deviceinfo[0].device_node, "/dev/sgks_vin");


	//N3 DMA
	vi_dev.vi_deviceinfo[1].enable = 0;
	vi_dev.vi_deviceinfo[1].device_id = 1;
	vi_dev.vi_deviceinfo[1].vi_type = SGKS_VI_TYPE_AUTO;
	vi_dev.vi_deviceinfo[1].interface_type = SGKS_VI_INTERFACE_AUTO;

	// ���ʹ��Ĭ�Ϸֱ��ʣ���������cap_width = 0, cap_height = 0, cap_fps = 0,
	vi_dev.vi_deviceinfo[1].vi_width = 0;
	vi_dev.vi_deviceinfo[1].vi_height = 0;
	vi_dev.vi_deviceinfo[1].vi_fps = 0;

	vi_dev.vi_deviceinfo[1].cap_start_x = 0;
	vi_dev.vi_deviceinfo[1].cap_start_y = 0;
	vi_dev.vi_deviceinfo[1].cap_width = 0;
	vi_dev.vi_deviceinfo[1].cap_height = 0;

	vi_dev.vi_deviceinfo[0].mirror_mode = SGKS_MPI_VI_MIRROR_AUTO;
	vi_dev.vi_deviceinfo[0].bayer_pattern = SGKS_MPI_VI_BAYER_PATTERN_AUTO;

	sprintf((char*)vi_dev.vi_deviceinfo[0].device_node, "/dev/sgks_vin");


	return sgks_mpi_vi_init(&vi_dev);
}
int bsv_PrevInit()
{
	sgks_mpi_vo_device_s vo_device;

	sgks_mpi_vo_GetScreenParam(&vo_device);

	///////////////preview config///////////////////
	sgks_preview_config_s pre_config;
	memset(&pre_config, 0, sizeof(sgks_preview_config_s));
	pre_config.preview_info[SGKS_VO_PREVIEW_A].enable = 1;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].pre_src = SGKS_PREVIEW_SRC_TYPE_SENSOR;
#if 1
	pre_config.preview_info[SGKS_VO_PREVIEW_A].pre_type = SGKS_PREVIEW_TYPE_SD;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].max_output_width = vo_device.vo_deviceinfo[0].video_size.vo_width;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].max_output_height = vo_device.vo_deviceinfo[0].video_size.vo_height;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].output_width = vo_device.vo_deviceinfo[0].video_size.vo_width;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].output_height = vo_device.vo_deviceinfo[0].video_size.vo_height;
#else
	pre_config.preview_info[SGKS_VO_PREVIEW_A].pre_type = SGKS_PREVIEW_TYPE_CAPTURE;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].max_output_width = 3840;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].max_output_height = 540;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].output_width = 3840;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].output_height = 540;
#endif
	pre_config.preview_info[SGKS_VO_PREVIEW_A].format = SGKS_DSP_VIDEO_FORMAT_NO_VIDEO;
	pre_config.preview_info[SGKS_VO_PREVIEW_A].fps = SGKS_DSP_VIDEO_FPS_25;

	pre_config.preview_info[SGKS_VO_PREVIEW_B].enable = 0;
	pre_config.preview_info[SGKS_VO_PREVIEW_B].pre_type = SGKS_PREVIEW_TYPE_OFF;
	pre_config.preview_info[SGKS_VO_PREVIEW_B].format = SGKS_DSP_VIDEO_FPS_29_97;

	pre_config.preview_info[SGKS_VO_PREVIEW_C].enable = 0;
	pre_config.preview_info[SGKS_VO_PREVIEW_C].pre_type = SGKS_PREVIEW_TYPE_OFF;

	return sgks_mpi_sys_PreViewInit(&pre_config);
}

int bsv_VoutInit(sgks_vo_src_e vo_0_src, sgks_vo_src_e vo_1_src)
{
	int fd = -1;
	sgks_mpi_vo_device_s 		vo_dev;
	msg_t 						ioctl_msg;
 #if 0

#else

	memset(&vo_dev, 0, sizeof(sgks_mpi_vo_device_s));
	vo_dev.vo_device_num = 1;
	sprintf((char*)vo_dev.vo_deviceinfo[0].device_node, "/dev/sgks_vout");
	vo_dev.vo_deviceinfo[0].id = SGKS_VOUT_A;
	vo_dev.vo_deviceinfo[0].type = SGKS_VO_SINK_TYPE_AUTO;
	vo_dev.vo_deviceinfo[0].sink_type = SGKS_VO_SINK_TYPE_AUTO;
	//vo_dev.vo_deviceinfo[0].mode = mipicfg->MODE;
	vo_dev.vo_deviceinfo[0].ratio = SGKS_VIDEO_RATIO_AUTO;
	vo_dev.vo_deviceinfo[0].bits = SGKS_VIDEO_BITS_AUTO;
	vo_dev.vo_deviceinfo[0].format = SGKS_VIDEO_FORMAT_AUTO;
	vo_dev.vo_deviceinfo[0].frame_rate = SGKS_DSP_VIDEO_FPS_50;//SGKS_DSP_VIDEO_FPS_29_97;
	vo_dev.vo_deviceinfo[0].csc_en = 1;
	vo_dev.vo_deviceinfo[0].hdmi_3d_structure = DDD_RESERVED;
	vo_dev.vo_deviceinfo[0].video_en = 1;

	/* �ֱ������� */
	vo_dev.vo_deviceinfo[0].video_info.width = 480;
	vo_dev.vo_deviceinfo[0].video_info.height = 854;

	vo_dev.vo_deviceinfo[0].video_info.format = SGKS_DSP_VIDEO_FORMAT_NO_VIDEO;
	vo_dev.vo_deviceinfo[0].video_info.fps = SGKS_DSP_VIDEO_FPS_50;//SGKS_DSP_VIDEO_FPS_29_97;

																   /* �ֱ������� */
	vo_dev.vo_deviceinfo[0].video_size.video_width =480;
	vo_dev.vo_deviceinfo[0].video_size.video_height = 854;
	vo_dev.vo_deviceinfo[0].video_size.vo_width = 480;
	vo_dev.vo_deviceinfo[0].video_size.vo_height = 854;

	vo_dev.vo_deviceinfo[0].video_offset.offset_x = 0;
	vo_dev.vo_deviceinfo[0].video_offset.offset_y = 0;

	/* ��Ƶ�����ת��ת���ã�������Ҫ�޸Ķ�Ӧ�ķֱ��� */
	vo_dev.vo_deviceinfo[0].video_rotate_flip_ctrl = SGKS_VO_NORMAL_DISPLAY;

	vo_dev.vo_deviceinfo[0].bg_color.y = 0x10;
	vo_dev.vo_deviceinfo[0].bg_color.cb = 0x80;
	vo_dev.vo_deviceinfo[0].bg_color.cr = 0x80;

	vo_dev.vo_deviceinfo[0].lcd_cfg.mode = SGKS_VO_LCD_MODE_DISABLE;
	vo_dev.vo_deviceinfo[0].fb_id = 0;

	vo_dev.vo_deviceinfo[0].vo_src = vo_0_src;
	vo_dev.vo_deviceinfo[0].yuv_width = (vo_dev.vo_deviceinfo[0].video_size.video_width + 63) & 0xFFFFFFC0; //64对齐
	vo_dev.vo_deviceinfo[0].yuv_height = (vo_dev.vo_deviceinfo[0].video_size.video_height + 3) & 0xFFFFFFFE; //4行对齐
	vo_dev.vo_deviceinfo[0].yuv_offset_x = 0;
	vo_dev.vo_deviceinfo[0].yuv_offset_y = 0;


#endif
	vo_dev.vo_deviceinfo[0].disp_config_dram_addr_phy = (u8 *)g_mem_map_table[display_vouta_addr].phy_addr;
	vo_dev.vo_deviceinfo[0].disp_config_dram_addr_size = g_mem_map_table[display_vouta_addr].size;

	vo_dev.vo_deviceinfo[0].tv_config_dram_addr_phy = (u8 *)g_mem_map_table[tv_dram_addr].phy_addr;
	vo_dev.vo_deviceinfo[0].tv_config_dram_addr_size = g_mem_map_table[tv_dram_addr].size;

	vo_dev.vo_deviceinfo[0].dram_clut_phy = (u8 *)g_mem_map_table[osd_clut_dram_addr].phy_addr;
	vo_dev.vo_deviceinfo[0].dram_clut_size = g_mem_map_table[osd_clut_dram_addr].size;

	vo_dev.vo_deviceinfo[0].dram_osd_phy = (u8 *)g_mem_map_table[osd_dram_addr].phy_addr;
	vo_dev.vo_deviceinfo[0].dram_osd_size = g_mem_map_table[osd_dram_addr].size;

	vo_dev.vo_deviceinfo[0].disp_bank_phy = (u8 *)g_mem_map_table[disp_bank_addr].phy_addr;
	vo_dev.vo_deviceinfo[0].disp_bank_size = g_mem_map_table[disp_bank_addr].size;
	return sgks_mpi_vo_init(&vo_dev);
}
int bsv_SysInit(sgks_dsp_op_mode_e dsp_op_mode)
{
	sgks_mpi_init_s mpi_init;

	memset(&mpi_init, 0x00, sizeof(sgks_mpi_init_s));
#if 0
	mpi_init.fw_info.fw_type = SGKS_MPI_FW_ENCRYPTION;
	mpi_init.fw_info.fw_file_num = 1;
	sprintf(mpi_init.fw_info.fw_path[0], "/home/lib/firmware/sgks_fw.bin");
#else
	mpi_init.fw_info.fw_type = SGKS_MPI_FW_UNENCRYPTION;
	mpi_init.fw_info.fw_file_num = 3;
	sprintf(mpi_init.fw_info.fw_path[0], "/home/lib/firmware/orccode.bin");
	sprintf(mpi_init.fw_info.fw_path[1], "/home/lib/firmware/orcme.bin");
	sprintf(mpi_init.fw_info.fw_path[2], "/home/lib/firmware/default_binary.bin");
#endif
	// fw file load mem addr
	mpi_init.fw_info.code_addr[0].dsp_code_start = (u32 *)g_mem_map_table[fw_orce].phy_addr;
	mpi_init.fw_info.code_addr[0].dsp_code_size = g_mem_map_table[fw_orce].size;

	mpi_init.fw_info.code_addr[1].dsp_code_start = (u32 *)g_mem_map_table[fw_me].phy_addr;
	mpi_init.fw_info.code_addr[1].dsp_code_size = g_mem_map_table[fw_me].size;

	mpi_init.fw_info.code_addr[2].dsp_code_start = (u32 *)g_mem_map_table[fw_binary].phy_addr;
	mpi_init.fw_info.code_addr[2].dsp_code_size = g_mem_map_table[fw_binary].size;


	//fw init param

	memset(&mpi_init.dsp_init_param, 0, sizeof(sgks_fw_init_param_s));

	mpi_init.dsp_init_param.operation_mode = dsp_op_mode;

	mpi_init.dsp_init_param.dsp_init_data_phy = (u32 *)g_mem_map_table[dsp_init_param].phy_addr;
	mpi_init.dsp_init_param.dsp_init_data_size = g_mem_map_table[dsp_init_param].size;

	mpi_init.dsp_init_param.default_binary_data_ptr_phy = (u32 *)g_mem_map_table[fw_binary].phy_addr;

	mpi_init.dsp_init_param.default_config_ptr_phy = (u32 *)g_mem_map_table[dsp_default_cmd].phy_addr;
	mpi_init.dsp_init_param.default_config_size = g_mem_map_table[dsp_default_cmd].size;

	mpi_init.dsp_init_param.cmd_data_ptr_phy = (u32 *)g_mem_map_table[dsp_normal_cmd].phy_addr;
	mpi_init.dsp_init_param.cmd_data_size = g_mem_map_table[dsp_normal_cmd].size;

	mpi_init.dsp_init_param.result_queue_ptr_phy = (u32 *)g_mem_map_table[dsp_result_queue].phy_addr;
	mpi_init.dsp_init_param.result_queue_size = g_mem_map_table[dsp_result_queue].size;

	mpi_init.dsp_init_param.dram_print_buf_addr_phy = g_mem_map_table[dsp_log].phy_addr;
	mpi_init.dsp_init_param.dram_printf_buf_size = g_mem_map_table[dsp_log].size;
	mpi_init.dsp_init_param.dsp2_info_ptr_phy = (u32 *)g_mem_map_table[dsp2_info].phy_addr;
	mpi_init.dsp_init_param.chip_id_ptr_phy = (u32 *)g_mem_map_table[dsp_chip].phy_addr;
	mpi_init.dsp_init_param.DSP_buf_ptr_phy = (u32 *)g_mem_map_table[dsp_buff].phy_addr;
	mpi_init.dsp_init_param.DSP_buf_size = g_mem_map_table[dsp_buff].size;
	mpi_init.dsp_init_param.msg_addr_arm_osd_phy = g_mem_map_table[msg_addr_arm_osd].phy_addr;
	mpi_init.dsp_init_param.msg_addr_arm_osd_size = g_mem_map_table[msg_addr_arm_osd].size;

	mpi_init.dsp_init_param.bsb_phy = (u32 *)g_mem_map_table[bsb_buff].phy_addr;
	mpi_init.dsp_init_param.bsb_size = g_mem_map_table[bsb_buff].size;
	mpi_init.dsp_init_param.bsb_desc_phy = g_mem_map_table[bsb_desc_addr].phy_addr;
	mpi_init.dsp_init_param.bsb_desc_size = g_mem_map_table[bsb_desc_addr].size;
	mpi_init.dsp_init_param.sec2_out_sec5_off = 1;
	return sgks_mpi_sys_Init(&mpi_init, g_mem_map_table);
}
void TestProc()
{
	Mat img;
	unsigned int i, x, d;
	unsigned int * p;
	img.create(720, 1280, CV_8UC4);

	bsv_SysInit(SGKS_DSP_DECODE_MODE);
	bsv_VinInit(1920, 1080, 1920, 1080, 0, 0, VENC_FPS_25);
	//bsv_VinInit(1920,2160,1920,1080,0,540,VENC_FPS_30);
	bsv_PrevInit();
	sgks_mpi_sys_Start(); //在该函数调用之前，必须初始化好Vin和Preview，否则FW无法启动；
	bsv_VoutInit(SGKS_VO_SRC_YUV422, SGKS_VO_SRC_ENC); //初始化Vout输出，输出内容由Encoder
													   //memset(img.datastart, i++, img.rows * img.cols * 4);
	p = (unsigned int *)img.datastart;

	cout<<(unsigned int)p<<endl;
	
	d = 0x000000FF;
	for (i = 0;i<img.cols;i++)
	{
		if ((i & 0x7F) == 0x7F)
			d = d << 8;
		if (d == 0)
			d = 0x000000FF;
		for (x = 0;x<img.rows;x++)
			*p++ = d;
	}

	ShowImageToLCD(img);
//	while (1)
//		usleep(10000);
}


int main(int argc, char **argv)
{
    TestProc();
            Mat dst = Mat::zeros(480, 960, CV_8UC4);
            Mat output( 480, 905, CV_8UC4);
            Mat show_img;
            show_img.create(480, 960, CV_8UC4);
            unsigned int *pbuff = NULL;
            unsigned int *ptr = NULL;
            ptr = new unsigned int[nLCDWidth*nLCDHeight+1024];
            pbuff = (unsigned int *)(((unsigned int)ptr+0x3F)&0xFFFFFFC0);
            Mat img;
            unsigned int i,x,d;
            unsigned int * p;
            img.create(720,1280,CV_8UC4);
    
            
            p = (unsigned int *)img.datastart;
            d = 0x000000FF;
            for(i = 0;i<img.rows;i++)
            {
                if((i&0x7F) == 0x7F)
                    d = d<<8;
                if(d == 0)
                    d = 0x000000FF;
                for(x = 0;x<img.cols;x++)
                    *p++ = d;
        }
            
            memset(pbuff, 0, sizeof(pbuff));
    
#if 1
            int ret = -1;
            int count = 0,offset = 0,read_i =0;
            int fd_sav;
            char openfile[40], filedir[30];
            char *filename = argv[4];
            int uv_size=0;
            int yuv_type = atoi(argv[2]);
        
            int fd = -1;
            struct stat fstatbuf;
            int rlen    = 0;
            int size    = 0;
            sgks_mpi_vdec_info_s vdec_info;
            //frame_head_t frame_head;
            int num = 0;
            int pos = 0;
            char *data_buf = (char *)sgks_mpi_Malloc(sizeof(char) * 1024 * 1024);
    
            ShowImageToLCD(img);
#endif	



	
	if (init_ = true)
	{
		cout << "*********start**********"<<endl;
		get_Univariate_matrix();
		cout << "*********start**********"<<endl;
		GetMapForRemap(matrix_affine, Map_Fx, Map_Fy);
		GetMapForRemap(matrix_affine_r, Map_Rx, Map_Ry);

		pa.compute_merge_matrix(frontMat, rearMat, CALIBRATOR_BOARD_SIZE, offsize_xx, offsize_yy);

		front_mask1 = Mat::ones(image_size, CV_8UC1);
		imwrite("result/融合参数/front_mask1.bmp", front_mask1);

		rear_mask1 = Mat::ones(image_size, CV_8UC1);
		imwrite("result/融合参数/rear_mask1.bmp", rear_mask1);

		pa.preProcess(front_mask1, rear_mask1);

		init_ = false;
	}


	cout << "步骤4, 开始处理图像序列..." << endl;
	int idx0 = 0; //FR

	VideoCapture front_cap("3_F.mkv");   //1_F.mkv    5F.avi
	VideoCapture rear_cap("3_B.mkv");   //1_B.mkv     5B.avi

	/*-----------------------------------------------------------------------------------------------------------------------------------------------------*/
	/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
	clock_t total_start = clock();
	if (front_cap.isOpened() && rear_cap.isOpened())
	{
		
		front_cap >> frontimage;
		rear_cap >> rearimage;

		Mat front_trs(front_image.size(), CV_8UC4, Scalar::all(0));
		Mat rear_trs(rear_image.size(), CV_8UC4, Scalar::all(0));

		while (frontimage.data && rearimage.data)
		{
			clock_t tc3;	
			cout << "image " << idx0++ << endl;
	

			if (idx0 >300 && idx0 < 20 * 500) 
			{

				{	
					cvtColor(frontimage, front_image, COLOR_BGR2BGRA);				
					cvtColor(rearimage, rear_image, COLOR_BGR2BGRA);

                    clock_t FT_st = clock();
                    
					output = av_merge_image(front_image, rear_image, 1);

                    if(DEBUG_MSG)
                        imwrite("debug/output.png", output);
                    clock_t FT_en = clock();
                    if(DEBUG_MSG)
                    cout<< "process Running time  is: " << static_cast<double>(FT_en - FT_st) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

					/*--------------------------------------------------------------------------------------------------------------------------------------*/
					if (idx0 > 300 + 1) {
#if 1 	
						Mat show(Size(900, 480), CV_8UC4, Scalar(0));
						resize(output, output, Size(300, 480));
						resize(front_image, front_image,Size(300,100));
						resize(rear_image, rear_image,Size(300,100));
						//cvtColor(output, output, COLOR_BGRA2BGR);
						front_image.copyTo(show(Rect(0, 200, 300, 100)));
						output.copyTo(show(Rect(300, 0, 300, 480)));
						rear_image.copyTo(show(Rect(600, 200, 300, 100)));
						putText(show, "Front_Image", Point(50,50), FONT_HERSHEY_COMPLEX,1,Scalar(0, 255, 255), 2, 8, 0);
						putText(show, "Back_Image", Point(650,50), FONT_HERSHEY_COMPLEX,1,Scalar(0, 255, 255), 2, 8, 0);
						resize(show, dst, dst.size());
                        if(DEBUG_MSG)
                            imwrite("debug/show.png", dst);
						// cvtColor(dst, dst, COLOR_BGRA2BGR);
						// abMatBGR2ARGB(dst, show_img);
						cout <<"****-----show-------****" << endl;
						ShowImageToLCD(dst);
                        cout <<"****-----show-------****" << endl;
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------*/

					}
					
				}
			}
			front_cap >> frontimage;
			rear_cap >> rearimage;
			
		}
		clock_t total_end = clock();
		front_cap.release();
		rear_cap.release();

	}
	return 0;
}

Mat av_merge(Mat front_image, Mat rear_image, bool Reversing)
{
	Mat out;
    if(!Reversing)
    {
        clock_t end_remap = clock();
        remap(front_image, front_trs, Map_Fx, Map_Fy, INTER_NEAREST, BORDER_CONSTANT);
        if(front_trs.size() != image_size)
    	{
            if(DEBUG_MSG)
    		    cout << "#################resize####################"<< endl;
    		resize(front_trs, front_trs, image_size);
    		resize(rear_trs, rear_trs, image_size);
    	}
        
        clock_t end_process = clock();
        out = pa.front_process(front_trs, rear_trs);
        
        if(!DEBUG_MSG)
            cout<< "###############################front process Running time  is: " << static_cast<double>(end_process - end_remap) / CLOCKS_PER_SEC * 1000 << "ms#####################" << endl;
    }
    else
    {
        clock_t end_remap = clock();
        remap(rear_image, rear_trs, Map_Rx, Map_Ry, INTER_NEAREST, BORDER_CONSTANT);
        if(front_trs.size() != image_size)
    	{
            if(DEBUG_MSG)
    		    cout << "#################resize####################"<< endl;
    		resize(front_trs, front_trs, image_size);
    		resize(rear_trs, rear_trs, image_size);
    	}
        
        out = pa.rear_process(front_trs, rear_trs);
        clock_t end_process = clock();
        if(!DEBUG_MSG)
            cout<< "###############################rear process Running time  is: " << static_cast<double>(end_process - end_remap) / CLOCKS_PER_SEC * 1000 << "ms#####################" << endl;
    }	
	return out;
}


Mat av_merge_image(Mat front_buf, Mat rear_buf, bool Reversing)
{
	if (init_ == true) 
	{
		cout << "*********start**********"<<endl;
		get_Univariate_matrix();
		cout << "*********start**********"<<endl;
		GetMapForRemap(matrix_affine, Map_Fx, Map_Fy);
		GetMapForRemap(matrix_affine_r, Map_Rx, Map_Ry);

		pa.compute_merge_matrix(frontMat, rearMat, CALIBRATOR_BOARD_SIZE, offsize_xx, offsize_yy);

		Mat front_chess = imread("F.bmp");
		Mat rear_chess = imread("B.bmp");

		remap(front_chess, front_chess, Map_Fx, Map_Fy, INTER_LINEAR, BORDER_CONSTANT);
		remap(rear_chess, rear_chess, Map_Rx, Map_Ry, INTER_LINEAR, BORDER_CONSTANT);
        if(DEBUG_MSG)
        {
            imwrite("debug/F_chess.jpg", front_chess);
		    imwrite("debug/B_chess.jpg", rear_chess);
        }
		front_mask1 = Mat::ones(image_size, CV_8UC1);

		rear_mask1 = Mat::ones(image_size, CV_8UC1);

		pa.preProcess(front_mask1, rear_mask1);
        if(DEBUG_MSG)
            cout << "##################end Init_parameter###########" <<endl;
		init_ = false;
	}

    clock_t st_b = clock();
    if(!Reversing)
    {
//        front_image.data = (unsigned char *)front_buf;
        if(DEBUG_MSG)
            imwrite("front_input.png", front_image);
        clock_t en_b = clock();

        cout<< "###############################bef Running time  is: " << static_cast<double>(en_b - st_b) / CLOCKS_PER_SEC * 1000 << "ms#####################" << endl;
    
        out =  av_merge(front_image, rear_image, Reversing);
    }
    else
    {
//        rear_image.data = (unsigned char *)rear_buf;

        if(DEBUG_MSG)
            imwrite("rear_input.png", rear_image);
        
        clock_t en_c = clock();
        
        cout<< "###############################bef Running time  is: " << static_cast<double>(en_c - st_b) / CLOCKS_PER_SEC * 1000 << "ms#####################" << endl;
        out =  av_merge(front_image, rear_image, Reversing);  
    }

	return out;
}



