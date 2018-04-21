CC=g++

all: writepic test
	$(CC) writepic.o test.o -std=c++11 `pkg-config --cflags --libs opencv` -llas
writepic: 
	$(CC) $@.cc -c -ggdb -std=c++11 `pkg-config --cflags --libs opencv`
test:
	$(CC) $@.cc -c -ggdb -std=c++11 -llas
#writepic: 
#	$(CC) -ggdb -o $@.o -I/usr/local/include/opencv -I/usr/local/include  -L/usr/local/lib/i386-linux-gnu -lopencv_dnn -lopencv_shape -lopencv_ml -lopencv_superres -lopencv_stitching -lopencv_videostab -lopencv_video -lopencv_photo -lopencv_viz -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_flann -lopencv_core $@.cc
