#include "cv_vx.h"

using namespace cv;
using namespace std;
Mat err_Mat;

#define OBJCHECK(objVX) if(!(objVX)) { printf("[%s : %d] %s\n",__FILE__, __LINE__, "obj create error.");return err_Mat; }
#define FUNCHECK(funRet) if(VX_SUCCESS!=(funRet)) { printf("[%s : %d] %s\n",__FILE__, __LINE__, "function error.");vxReleaseContext(&context);return err_Mat;}
#define ERROR_CHECK_OBJECT(obj)                                                                 \
    {                                                                                           \
        vx_status status_ = vxGetStatus((vx_reference)(obj));                                   \
        if (status_ != VX_SUCCESS)                                                              \
        {                                                                                       \
            printf("ERROR: failed with status = (%d) at " __FILE__ "#%d\n", status_, __LINE__); \
            exit(0);                                                                            \
        }                                                                                       \
    }
#define ERROR_CHECK_STATUS(status)                                                              \
    {                                                                                           \
        vx_status status_ = (status);                                                           \
        if (status_ != VX_SUCCESS)                                                              \
        {                                                                                       \
            printf("ERROR: failed with status = (%d) at " __FILE__ "#%d\n", status_, __LINE__); \
            exit(0);                                                                            \
        }                                                                                       \
    }


vx_image CV2VX(Mat input_image, vx_context context)
{
	//    Mat input_image;

	vx_df_image vx_format;

	int cv_format = input_image.type();

	vx_uint32 width, height;
	width = input_image.size().width;
	height = input_image.size().height;

	vx_rectangle_t image_region;
	image_region.start_x = 0;
	image_region.start_y = 0;
	image_region.end_x = width;
	image_region.end_y = height;

	vx_imagepatch_addressing_t image_layout;
	image_layout.stride_y = input_image.step;

	if (cv_format == CV_16S)
	{
		image_layout.stride_x = 2;
		vx_format = VX_DF_IMAGE_S16;
	}
	else if (cv_format == CV_8U)
	{
		image_layout.stride_x = 1;
		vx_format = VX_DF_IMAGE_U8;
	}
	else if (cv_format == CV_8UC3)
	{
		image_layout.stride_x = 3;
		vx_format = VX_DF_IMAGE_RGB;
	}
	else if (cv_format == CV_8UC4)
	{
		image_layout.stride_x = 4;
		vx_format = VX_DF_IMAGE_RGBX;
	}

	vx_image dst_image;
	dst_image = vxCreateImage(context, width, height, vx_format);
	vxCopyImagePatch(dst_image, &image_region, 0, &image_layout, input_image.data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

	return dst_image;
}

int VX2CV(Mat &mat, vx_image image)
{

	vx_uint32 width = 0;
	vx_uint32 height = 0;
	vx_df_image format = VX_DF_IMAGE_VIRT;
	int CV_format = 0;
	vx_size planes = 0;
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_WIDTH, &width, sizeof(width)));
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(height)));
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format)));
	ERROR_CHECK_STATUS(vxQueryImage(image, VX_IMAGE_PLANES, &planes, sizeof(planes)));

	if (format == VX_DF_IMAGE_U8)
		CV_format = CV_8U;
	if (format == VX_DF_IMAGE_S16)
		CV_format = CV_16S;
	if (format == VX_DF_IMAGE_S32)
		CV_format = CV_32S;
	if (format == VX_DF_IMAGE_RGB)
		CV_format = CV_8UC3;
	if (format == VX_DF_IMAGE_U32)
		CV_format = CV_8UC4;

	if (format != VX_DF_IMAGE_U8 && format != VX_DF_IMAGE_S16 && format != VX_DF_IMAGE_RGB && format != VX_DF_IMAGE_S32 && format != VX_DF_IMAGE_U32)
	{
		vxAddLogEntry((vx_reference)image, VX_ERROR_INVALID_FORMAT, "VX2CV_Image ERROR: Image type not Supported in this RELEASE\n");
		return VX_ERROR_INVALID_FORMAT;
	}
	vx_rectangle_t rect = {0, 0, width, height};

	mat = Mat(height, width, CV_format);
	Mat *pMat = (Mat *)&mat;

	vx_uint8 *src[4] = {NULL, NULL, NULL, NULL};
	vx_uint32 p;
	void *ptr = NULL;
	vx_imagepatch_addressing_t addr[4] = {0, 0, 0, 0};
	vx_uint32 y = 0u;

	vx_map_id map_id;

	for (p = 0u; (p < (int)planes); p++)
	{
		ERROR_CHECK_STATUS(vxMapImagePatch(image, &rect, p, &map_id, &addr[p], (void **)&src[p], VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));

		size_t len = addr[p].stride_x * (addr[p].dim_x * addr[p].scale_x) / VX_SCALE_UNITY;
		for (y = 0; y < height; y += addr[p].step_y)
		{
			ptr = vxFormatImagePatchAddress2d(src[p], 0, y - rect.start_y, &addr[p]);
			memcpy(pMat->data + y * pMat->step, ptr, len);
		}
	}

	ERROR_CHECK_STATUS(vxUnmapImagePatch(image, map_id));
	pMat->convertTo(mat, CV_8U);

	return 0;
}

static bool _init_vx = true;

static vx_uint32 width = 320;
static vx_uint32 height = 640;

static vx_context context = NULL;
static vx_graph graph = NULL;
static vx_image image_input = NULL;
static vx_image image_RGB[3];
static vx_image image_WRGB[3];
static vx_image OUT = NULL;
static vx_matrix wrap_affine_matrix = NULL;
static vx_interpolation_type_e interpolation[2];
static Mat vx_output;
static vx_node node[7];

static vx_imagepatch_addressing_t imgInfo = VX_IMAGEPATCH_ADDR_INIT;
static vx_uint8* imgData = (vx_uint8*)malloc(width*height*3*sizeof(vx_uint8));
static vx_rectangle_t rect = {0,0,width,height};
static vx_map_id map_id = 0;
static uchar * out_buff = (uchar *)malloc(sizeof(uchar) * width*height*3);


void init_vx(vx_context& context, vx_graph& graph)
{
    _init_vx = false;

    vx_output = Mat::zeros(Size(width, height),CV_8UC3);
    context = vxCreateContext();
    graph = vxCreateGraph(context);
    image_input = vxCreateImage(context,width,height,VX_DF_IMAGE_RGB);
    OUT = vxCreateImage(context,width,height,VX_DF_IMAGE_RGB);

    for(int i = 0; i < 3; i++)
    {
        image_RGB[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
        image_WRGB[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    }


    interpolation[0] = VX_INTERPOLATION_NEAREST_NEIGHBOR;
    interpolation[1] = VX_INTERPOLATION_BILINEAR;

    wrap_affine_matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);


    node[0] = vxChannelExtractNode(graph, image_input, VX_CHANNEL_R, image_RGB[0]);
    node[1] = vxChannelExtractNode(graph, image_input, VX_CHANNEL_G, image_RGB[1]);
    node[2] = vxChannelExtractNode(graph, image_input, VX_CHANNEL_B, image_RGB[2]);
    node[3] = vxWarpAffineNode(graph, image_RGB[0], wrap_affine_matrix, interpolation[0], image_WRGB[0]);
    node[4] = vxWarpAffineNode(graph, image_RGB[1], wrap_affine_matrix, interpolation[0], image_WRGB[1]);
    node[5] = vxWarpAffineNode(graph, image_RGB[2], wrap_affine_matrix, interpolation[0], image_WRGB[2]);
    node[6] = vxChannelCombineNode(graph, image_WRGB[0], image_WRGB[1], image_WRGB[2], NULL, OUT);
    
    vxMapImagePatch(image_input,&rect,0,&map_id,&imgInfo,(void**)&imgData,VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST,0);
    vxMapImagePatch(OUT,&rect,0,&map_id,&imgInfo,(void**)&out_buff,VX_READ_ONLY,VX_MEMORY_TYPE_HOST,0);

}


Mat vx_Affine_RGB(Mat input, Mat matrix){

    // create graph
    if(_init_vx)
    init_vx(context, graph);
    
    vx_uint32 SIZE = width*height*3;

    memcpy(imgData,(uchar *)input.data,sizeof(uchar)*SIZE);

    double _rotate[2][2] = {
        {matrix.at<double>(0, 0), matrix.at<double>(0, 1)},
        {matrix.at<double>(1, 0), matrix.at<double>(1, 1)}
    };
    Mat rotate_mat = Mat(2, 2, CV_64FC1, _rotate); 
    Mat affine_parameter = rotate_mat.inv();
    vx_float32 mat[3][2] = {
        {(float)affine_parameter.at<double>(0, 0), (float)affine_parameter.at<double>(0, 1)},
        {(float)affine_parameter.at<double>(1, 0), (float)affine_parameter.at<double>(1, 1)},
        {(float)(-matrix.at<double>(0, 2)), (float)(-matrix.at<double>(1, 2))},
    };

     
    FUNCHECK(vxCopyMatrix(wrap_affine_matrix , mat, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    clock_t a = clock(); 

    FUNCHECK(vxProcessGraph(graph));
    clock_t b = clock(); 
    
    cout<< "Process_Graph Running time  is: " << static_cast<double>(b - a) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

    vx_output.data = (uchar *)out_buff;

    return vx_output;


}

Mat vx_Remap_RGB(Mat input, Mat map_x, Mat map_y, int OUTW, int OUTH){


    Mat output;
    vx_context context = vxCreateContext();

    // create graph
    vx_graph graph = vxCreateGraph(context);

    vx_uint32 width, height;
    width = input.size().width;
    height = input.size().height;
    vx_uint32 SIZE = width*height*3;
    ///////////////////////////////////////////////////////////////
    vx_image image_input = vxCreateImage(context,width,height,VX_DF_IMAGE_RGB);
    OBJCHECK(image_input);
    output = Mat::zeros(input.size(),CV_8UC3);
	vx_interpolation_type_e interpolation[] = {
    VX_INTERPOLATION_NEAREST_NEIGHBOR,
    VX_INTERPOLATION_BILINEAR};

    vx_imagepatch_addressing_t imgInfo = VX_IMAGEPATCH_ADDR_INIT;
	vx_uint8* imgData = (vx_uint8*)malloc(SIZE*sizeof(vx_uint8));
    vx_rectangle_t rect = {0,0,width,height};
    vx_map_id map_id = 0;

    vxMapImagePatch(image_input,&rect,0,&map_id,&imgInfo,(void**)&imgData,VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST,0);
    memcpy(imgData,(uchar *)input.data,sizeof(uchar)*SIZE);

    vx_remap remap = vxCreateRemap(context, width,height, width,height);


    vx_uint32 x,y  = 0;

    for( x = 0; x<width; x ++)
    {
        for(y = 0; y<height; y ++)
        {
            vxSetRemapPoint(remap, x, y, map_x.at<float>(x, y), map_x.at<float>(x, y));//dstx, dsty, srcx, srcy.srcx=mapx,srcy=mapy
        }
    }


	vx_image OUT = vxCreateImage(context,height,width,VX_DF_IMAGE_RGB);
	OBJCHECK(OUT);

    vx_image image_RGB[3] =
    {
        vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
        vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
        vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
    };

    vx_image image_WRGB[3] =
    {
        vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
        vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
        vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
    };

    vx_node node[] = {
        vxChannelExtractNode(graph, image_input, VX_CHANNEL_R, image_RGB[0]),
        vxChannelExtractNode(graph, image_input, VX_CHANNEL_G, image_RGB[1]),
        vxChannelExtractNode(graph, image_input, VX_CHANNEL_B, image_RGB[2]),
        vxRemapNode(graph, image_RGB[0], remap, interpolation[0], image_WRGB[0]),
        vxRemapNode(graph, image_RGB[1], remap, interpolation[0], image_WRGB[1]),
        vxRemapNode(graph, image_RGB[2], remap, interpolation[0], image_WRGB[2]),
        vxChannelCombineNode(graph, image_WRGB[0], image_WRGB[1], image_WRGB[2], NULL, OUT),
    };
    FUNCHECK(vxVerifyGraph(graph));
    FUNCHECK(vxProcessGraph(graph));

    VX2CV(output, OUT);

    
    return output;

}




#if 0
int main()
{
    Mat in_image = imread("src.bmp");
    
    Mat affine_m = in_image;
    Mat map_x = imread("mapx.jpg", 0);
    Mat map_y = imread("mapy.jpg", 0);
    map_x.convertTo(map_x, CV_32FC1);
    map_y.convertTo(map_y, CV_32FC1);
    map_x = map_x+255;
    map_y = map_y+255;

    cout << map_x << endl;
  
    Mat output = vx_Remap_RGB(in_image, in_image, in_image, 0, 0);
    
    imwrite("vx_Affine_RGB_1.jpg", output);
}
#endif
