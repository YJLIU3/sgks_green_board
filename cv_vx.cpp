#include "cv_vx.h"

using namespace cv;
using namespace std;
Mat err_Mat;

#define OBJCHECK(objVX) if(!(objVX)) { printf("[%s : %d] %s\n",__FILE__, __LINE__, "obj create error.");return err_Mat; }
#define FUNCHECK(funRet) if(VX_SUCCESS!=(funRet)) { printf("[%s : %d] %s\n",__FILE__, __LINE__, "function error.");vxReleaseContext(&context);return err_Mat;}


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
    clock_t a = clock(); 

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



    FUNCHECK(vxProcessGraph(graph));
    clock_t b = clock(); 
    
    cout<< "Process_Graph Running time  is: " << static_cast<double>(b - a) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

    vx_output.data = (uchar *)out_buff;

    return vx_output;


}
static bool _init_vx_remap = true;

static vx_uint32 width_remap = 1280;
static vx_uint32 height_remap = 720;

static vx_context context_remap = NULL;
static vx_graph graph_remap = NULL;
static vx_image image_input_remap = NULL;
static vx_image image_RGB_remap[3];
static vx_image image_WRGB_remap[3];
static vx_image OUT_remap = NULL;
static vx_interpolation_type_e interpolation_remap[2];
static vx_remap remap_vx;
static Mat vx_output_remap;
static vx_node node_remap[7];

static vx_imagepatch_addressing_t imgInfo_remap = VX_IMAGEPATCH_ADDR_INIT;
static vx_uint8* imgData_remap = (vx_uint8*)malloc(width_remap*height_remap*3*sizeof(vx_uint8));
static vx_rectangle_t rect_remap = {0,0,width_remap,height_remap};
static vx_map_id map_id_remap = 0;
static uchar * out_buff_remap = (uchar *)malloc(sizeof(uchar) * width_remap*height_remap*3);

void init_vx_remap( Mat map_x, Mat map_y)
{
    _init_vx_remap = false;

    {
        
    vxReleaseImage(&image_input_remap);

    vxReleaseImage(&image_RGB_remap[0]);
    vxReleaseImage(&image_RGB_remap[1]);
    vxReleaseImage(&image_RGB_remap[2]);
    vxReleaseImage(&image_WRGB_remap[0]);
    vxReleaseImage(&image_WRGB_remap[1]);
    vxReleaseImage(&image_WRGB_remap[2]);

    vxReleaseGraph(&graph_remap);
    vxReleaseContext(&context_remap);
    
    }

    vx_output_remap = Mat::zeros(Size(width_remap, height_remap),CV_8UC3);
    context_remap = vxCreateContext();
    graph_remap = vxCreateGraph(context_remap);
    image_input_remap = vxCreateImage(context_remap,width_remap,height_remap,VX_DF_IMAGE_RGB);
    OUT_remap = vxCreateImage(context_remap,width_remap,height_remap,VX_DF_IMAGE_RGB);
    
    remap_vx = vxCreateRemap(context_remap, width_remap,height_remap, width_remap,height_remap);

    for(int i = 0; i < 3; i++)
    {
        image_RGB_remap[i] = vxCreateImage(context_remap, width_remap, height_remap, VX_DF_IMAGE_U8);
        image_WRGB_remap[i] = vxCreateImage(context_remap, width_remap, height_remap, VX_DF_IMAGE_U8);
    }


    interpolation_remap[0] = VX_INTERPOLATION_NEAREST_NEIGHBOR;
    interpolation_remap[1] = VX_INTERPOLATION_BILINEAR;

    vx_uint32 x,y  = 0;

    for( x = 0; x<width_remap; x ++)
    {
        for(y = 0; y<height_remap; y ++)
        {
            if(x < map_x.cols && y < map_x.rows)
                vxSetRemapPoint(remap_vx, x, y, map_x.at<float>(y, x), map_y.at<float>(y, x));//dstx, dsty, srcx, srcy.srcx=mapx,srcy=mapy
            else
                vxSetRemapPoint(remap_vx, x, y, 0, 0);//dstx, dsty, srcx, srcy.srcx=mapx,srcy=mapy              
        }
    }

    node_remap[0] = vxChannelExtractNode(graph_remap, image_input_remap, VX_CHANNEL_R, image_RGB_remap[0]);
    node_remap[1] = vxChannelExtractNode(graph_remap, image_input_remap, VX_CHANNEL_G, image_RGB_remap[1]);
    node_remap[2] = vxChannelExtractNode(graph_remap, image_input_remap, VX_CHANNEL_B, image_RGB_remap[2]);
    node_remap[3] = vxRemapNode(graph_remap, image_RGB_remap[0], remap_vx, interpolation_remap[1], image_WRGB_remap[0]);
    node_remap[4] = vxRemapNode(graph_remap, image_RGB_remap[1], remap_vx, interpolation_remap[1], image_WRGB_remap[1]);
    node_remap[5] = vxRemapNode(graph_remap, image_RGB_remap[2], remap_vx, interpolation_remap[1], image_WRGB_remap[2]);
    node_remap[6] = vxChannelCombineNode(graph_remap, image_WRGB_remap[0], image_WRGB_remap[1], image_WRGB_remap[2], NULL, OUT_remap);
    
    vxMapImagePatch(image_input_remap,&rect_remap,0,&map_id_remap,&imgInfo_remap,(void**)&imgData_remap,VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST,0);
    vxMapImagePatch(OUT_remap,&rect_remap,0,&map_id_remap,&imgInfo_remap,(void**)&out_buff_remap,VX_READ_ONLY,VX_MEMORY_TYPE_HOST,0);

}


Mat vx_Remap_RGB(Mat input, bool Revers){

    vx_uint32 SIZE = width_remap*height_remap*3;
    memcpy(imgData_remap,(uchar *)input.data,sizeof(uchar)*SIZE);


//  FUNCHECK(vxProcessGraph(graph_remap));
    cout << vxProcessGraph(graph_remap) << "########"<<endl;

    vx_output_remap.data = (uchar *)out_buff_remap;

    if(!Revers)
        vx_output_remap = vx_output_remap(Rect(0, 0, 260, 180));
    else
        vx_output_remap = vx_output_remap(Rect(0, 0, 260, 180));
    
    return vx_output_remap;

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
