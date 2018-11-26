#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_api.h>
#include <VX/vx_khr_cnn.h>

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vx_api.h"

#define OBJCHECK(objVX) if(!(objVX)) { printf("[%s : %d] %s\n",__FILE__, __LINE__, "obj create error.");return -1; }
#define FUNCHECK(funRet) if(VX_SUCCESS!=(funRet)) { printf("[%s : %d] %s\n",__FILE__, __LINE__, "function error.");vxReleaseContext(&context);return -1;}



int vx_perspective_version(Mat input1, Mat output )
{


	vx_status status = VX_FAILURE;
	vx_context context=vxCreateContext();
	OBJCHECK(context);
    vx_graph graph = vxCreateGraph(context);
    OBJCHECK(graph);

	vx_uint32 WIDTH = input1.rows;
	vx_uint32 HEIGHT = input1.cols;
	vx_uint32 SIZE = WIDTH*HEIGHT;

    vx_uint8 *buf_in = (vx_uint8*)malloc(SIZE*sizeof(vx_uint8));
    vx_uint8 *buf_out = (vx_uint8*)malloc(SIZE*sizeof(vx_uint8));

	int cnt_px = 0;
	

	buf_in = (uchar*)input1.data;

	vx_image IN = vxCreateImage(context,HEIGHT,WIDTH,VX_DF_IMAGE_U8);
	OBJCHECK(IN);
	vx_image OUT = vxCreateImage(context,HEIGHT,WIDTH,VX_DF_IMAGE_U8);
	OBJCHECK(OUT);

	vx_imagepatch_addressing_t imgInfo = VX_IMAGEPATCH_ADDR_INIT;
	vx_uint8* imgData = 0;
    vx_rectangle_t rect = {0,0,HEIGHT,WIDTH};
    vx_map_id map_id = 0;

    status = vxMapImagePatch(IN,&rect,0,&map_id,&imgInfo,(void**)&imgData,VX_WRITE_ONLY,VX_MEMORY_TYPE_HOST,0);
	FUNCHECK(status);
    memcpy(imgData,buf_in,sizeof(vx_uint8)*SIZE);
    status = vxUnmapImagePatch(IN,map_id); //match with vxUnmapImagePatch()
    FUNCHECK(status);
    imgData = NULL;

	vx_interpolation_type_e interpolation[] = {
    VX_INTERPOLATION_NEAREST_NEIGHBOR,
    VX_INTERPOLATION_BILINEAR};
    vx_float32 a = 2.4593, b = 2.7701, c = -675.43, d = 0.5245,
            e = 7.74, f = -1075.5, g = 4.36e-04, h = 9.78e-03, i = -0.3306;	

	vx_float32 perspective_mat[3][3] = {
        {a, d, g}, // 'x' coefficients
        {b, e, h}, // 'y' coefficients
        {c, f, i}, // 'offsets'
    };

	vx_matrix wrap_perspective_matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
    FUNCHECK(vxCopyMatrix(wrap_perspective_matrix , perspective_mat, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));



	vx_node node = vxWarpPerspectiveNode(graph, IN, wrap_perspective_matrix, interpolation[0], OUT);
    OBJCHECK(node);
    FUNCHECK(vxReleaseNode(&node));
	
    FUNCHECK(vxVerifyGraph(graph));
    FUNCHECK(vxProcessGraph(graph));

    FUNCHECK(vxReleaseMatrix(&wrap_perspective_matrix));

    status = vxMapImagePatch(OUT,&rect,0,&map_id,&imgInfo,(void**)&imgData,VX_READ_ONLY,VX_MEMORY_TYPE_HOST,0);
    FUNCHECK(status);
 	
	memcpy((uchar*)output.data,imgData,sizeof(vx_uint8)*SIZE);

    status = vxUnmapImagePatch(OUT,map_id);
	FUNCHECK(status);
    imgData = NULL;

    vxReleaseContext(&context);

	return 0;
}
