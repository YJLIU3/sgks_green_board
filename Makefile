cc = arm-linux-g++ -w -O3 -std=c++11 `pkg-config --cflags --libs opencv`
#INCLUDES = -I/opt/sgks/arm-linux/arm-buildroot-linux-gnueabi/sysroot/usr/include/mpi
LIBS = -L/opt/sgks/arm-linux/arm-buildroot-linux-gnueabi/sysroot/usr/lib  -lpthread -lstdc++ -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=vfpv4-d16 -mfloat-abi=softfp -lz -lrt -lm -ldl -lCLC -lLLVM_viv -lOpenVX -lOpenVXU -lVSC -lGAL -fopenmp
CFLAGS = -w -O3 -g -pipe -Wl,-rpath=. -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=vfpv4-d16 -mfloat-abi=softfp -D__LINUX__ -D__cplusplus -DUSE_GETOUTLINES_VXC -DUSE_GETLINES_VXC -DUSE_NONEZEROPOS_VXC -DLINUX
OPT = -O3
USER_LIB_PATH = /home/lyj/OutFile/lib
objects = DuadPanorama.o panorama.o gpu2d.o render.o fftm.o cv_vx.o

edit:$(objects)
	$(cc) $(LIBS) -o edit  $(objects) $(USER_LIB_PATH)/libmpi.a $(USER_LIB_PATH)/libimage.a

fftm.o: fftm.cpp fftm.hpp
	$(cc) -c  $(LIBS) fftm.cpp 

DuadPanorama.o: DuadPanorama.cpp panorama.h parameter.h 
	$(cc) -c  $(LIBS) DuadPanorama.cpp 
panorama.o: panorama.cpp panorama.h parameter.h 
	$(cc) -c $(INCLUDES) panorama.cpp

gpu2d.o: gpu2d.cpp gpu2d.h 
	$(cc) $(CFLAGS) -c gpu2d.cpp
render.o: render.cpp galUtil.h ringbuffer.hpp gpu2d.h
	$(cc) $(CFLAGS) -c render.cpp

cv_vx.o: cv_vx.cpp cv_vx.h
	$(cc) -lOpenVX -lOpenVXU -lCLC -lGAL -lVSC -lLLVM_viv  -c cv_vx.cpp

#VX_SGKS.o: VX_SGKS.c VX_SGKS.h
#	arm-linux-g++ -g -mtune=cortex-a7 -std=c++11 -c -L./ VX_SGKS.c VX_SGKS.h `pkg-config --cflags --libs opencv` -lOpenVX -lOpenVXU -lCLC -lGAL -lVSC -lLLVM_viv  -lstdc++ -lrt

clean:
	rm edit $(objects)
