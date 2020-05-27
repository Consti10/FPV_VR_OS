//
// Created by geier on 07/05/2020.
//

#ifndef UVCCAMERA_MJPEGDECODEANDROID_HPP
#define UVCCAMERA_MJPEGDECODEANDROID_HPP

#include "HuffTables.hpp"
#include <jni.h>
#include <android/native_window_jni.h>
#include <setjmp.h>
#include <AndroidLogger.hpp>
#include <TimeHelper.hpp>
#include <vector>
#include "MyColorSpaces.hpp"

// Since I only need to support android it is cleaner to write my own conversion function.
// inspired by the uvc_mjpeg_to_rgbx .. functions
// Including this file adds dependency on Android and libjpeg-turbo
// now class since then I can reuse the jpeg_decompress_struct dinfo member (and do not need to re-allocate & init)
class MJPEGDecodeAndroid{
public:
    MJPEGDecodeAndroid(){
        struct error_mgr jerr;
        dinfo.err = jpeg_std_error(&jerr.super);
        jerr.super.error_exit = _error_exit;
        jpeg_create_decompress(&dinfo);
    }
    ~MJPEGDecodeAndroid(){
        jpeg_destroy_decompress(&dinfo);
    }
private:
   struct jpeg_decompress_struct dinfo;
    //  error handling (must be set !)
    struct error_mgr {
        struct jpeg_error_mgr super;
        jmp_buf jmp;
    };
    static void _error_exit(j_common_ptr dinfo) {
        struct error_mgr *myerr = (struct error_mgr *) dinfo->err;
        char err_msg[1024];
        (*dinfo->err->format_message)(dinfo, err_msg);
        err_msg[1023] = 0;
        MLOGD<<"LIBJPEG ERROR %s"<<err_msg;
        longjmp(myerr->jmp, 1);
    }
    // 'Create array with pointers to an array'
    static std::vector<uint8_t*> convertToPointers(uint8_t* array1d, size_t heightInPx, size_t scanline_width){
        std::vector<uint8_t*> ret(heightInPx);
        for(int i=0; i < heightInPx; i++){
            ret[i]=array1d+i*scanline_width;
        }
        return ret;
    }
    // We need to set the error manager every time else it will crash (I have no idea why )
    // https://stackoverflow.com/questions/11613040/why-does-jpeg-decompress-create-crash-without-error-message
    void setErrorManager(){
        struct error_mgr jerr;
        dinfo.err = jpeg_std_error(&jerr.super);
        jerr.super.error_exit = _error_exit;
    }
public:
    // Helper that prints the current configuration of ANativeWindow_Buffer
    static void debugANativeWindowBuffer(const ANativeWindow_Buffer& buffer){
        MLOGD<<"ANativeWindow_Buffer: W H "<<buffer.width<<" "<<buffer.height<<" format "<<buffer.format<<" Stride "<<buffer.stride;
    }
    // Supports the most common ANativeWindow_Buffer image formats
    // No unnecessary memcpy's & correctly handle stride of ANativeWindow_Buffer
    void DecodeMJPEGtoANativeWindowBuffer(const unsigned char* mpegData,size_t mpegDataSize,const ANativeWindow_Buffer& nativeWindowBuffer){
        debugANativeWindowBuffer(nativeWindowBuffer);
        MEASURE_FUNCTION_EXECUTION_TIME
        setErrorManager();

        jpeg_mem_src(&dinfo,mpegData,mpegDataSize);
        jpeg_read_header(&dinfo, TRUE);
        //MLOGD<<"Input color space is "<<dinfo.jpeg_color_space<<" num components "<<dinfo.num_components<<" data precision "<<dinfo.data_precision;
        //MLOGD<<"h samp factor"<<dinfo.comp_info[0].h_samp_factor<<"v samp factor "<<dinfo.comp_info[0].v_samp_factor;
        //MLOGD<<"min recommended height of scanline buffer. "<<dinfo.rec_outbuf_height;
        //For decompression, the JPEG file's color space is given in jpeg_color_space,
        //and this is transformed to the output color space out_color_space.
        // See jdcolor.c
        //MLOGD<<"jpeg color space is "<<dinfo.jpeg_color_space<<" libjpeg guessed following output colorspace "<<dinfo.out_color_space;
        // unsigned int scale_num, scale_denom : We do not need scaling since the HW composer does this job for us
        //MLOGD<<"boolean do_fancy_upsampling (default true) "<<dinfo.do_fancy_upsampling<<" boolean do_block_smoothing (default true) "<<dinfo.do_block_smoothing;
        //If your interest is merely in bypassing color conversion, we recommend
        //that you use the standard interface and simply set jpeg_color_space =
        //in_color_space (or jpeg_color_space = out_color_space for decompression).
        //
        unsigned int BYTES_PER_PIXEL;
        if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM || nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM){
            dinfo.out_color_space = JCS_EXT_RGBA;
            BYTES_PER_PIXEL=4;
        }else if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM){
            dinfo.out_color_space = JCS_EXT_RGB;
            BYTES_PER_PIXEL=3;
        }else if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM){
            dinfo.out_color_space = JCS_RGB565;
            BYTES_PER_PIXEL=2;
        }else if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420){
            dinfo.out_color_space = JCS_YCbCr;
            BYTES_PER_PIXEL=2;
        }else{
            MLOGD<<"Unsupported image format";
            return;
        }
        dinfo.dct_method = JDCT_IFAST;
        jpeg_start_decompress(&dinfo);

        // libjpeg error ? - output_components is 3 ofr RGB_565 ?
        // create the array of pointers that takes stride of nativeWindowBuffer into account
        // Especially when using RGB (24 bit) stride != image height
        const unsigned int scanline_len = ((unsigned int)nativeWindowBuffer.stride) * BYTES_PER_PIXEL;
        const auto tmp=convertToPointers((uint8_t*)nativeWindowBuffer.bits,dinfo.output_height,scanline_len);
        unsigned int scanline_count = 0;
        while (dinfo.output_scanline < dinfo.output_height){
            JSAMPROW row2= (JSAMPROW)tmp[scanline_count];
            auto lines_read=jpeg_read_scanlines(&dinfo,&row2, 8);
            // unfortunately reads only one line at a time CLOGD("Lines read %d",lines_read);
            scanline_count+=lines_read;
        }
        jpeg_finish_decompress(&dinfo);
    }


    void decodeToYUVXXXBuffer(MyColorSpaces::YUV422Planar<640,480>& out_buff, unsigned char * in_buf, unsigned long in_buf_size){
        MEASURE_FUNCTION_EXECUTION_TIME
        setErrorManager();
        uint32_t pixel_format = 0;

        dinfo.out_color_space = JCS_YCbCr;
        jpeg_mem_src(&dinfo, in_buf, in_buf_size);
        dinfo.out_color_space = JCS_YCbCr;

        jpeg_read_header(&dinfo, TRUE);
        dinfo.out_color_space = JCS_YCbCr;
        if(dinfo.jpeg_color_space!=dinfo.out_color_space){
            MLOGE<<"Wrong usage";
        }
        dinfo.dct_method = JDCT_FASTEST;
        dinfo.raw_data_out = TRUE;
        jpeg_start_decompress (&dinfo);

        decodeDirect(out_buff);
        //JSAMPIMAGE jsampimage;
        //std::vector<uint8_t> decodedData(640*480*16/8);
        //uint8_t* data=decodedData.data();

        jpeg_finish_decompress(&dinfo);
        //*buffer = out_buf;

        MLOGD<<"Succesfully decoded Buffer ";
    }

    void decodeDirect(MyColorSpaces::YUV422Planar<640,480>& out_buf){
        unsigned char **yuv[3];
        unsigned char *y[4 * DCTSIZE] = { NULL, };
        unsigned char *u[4 * DCTSIZE] = { NULL, };
        unsigned char *v[4 * DCTSIZE] = { NULL, };
        //int v_samp_factor[3];

        auto y2=convertToPointers(&out_buf.planeY[0][0],480,640);
        auto u2=convertToPointers(&out_buf.planeU[0][0],480,320);
        auto v2=convertToPointers(&out_buf.planeV[0][0],480,320);

        //for(int i=0;i<3;i++){
        //    MLOGD<<i<<"h samp factor"<<dinfo.comp_info[i].h_samp_factor<<"v samp factor "<<dinfo.comp_info[i].v_samp_factor;
        //}

        auto max_v_samp_factor=dinfo.comp_info[0].v_samp_factor;
        size_t scanline_count=0;
        for (int i = 0; i < (int) dinfo.image_height; i += max_v_samp_factor * DCTSIZE){
            //jpeg_read_raw_data() returns one MCU row per call, and thus you must pass a
            //buffer of at least max_v_samp_factor*DCTSIZE scanlines
            const auto SOME_SIZE=max_v_samp_factor * DCTSIZE;
            yuv[0] = &y2[scanline_count];
            yuv[1] = &u2[scanline_count];
            yuv[2] = &v2[scanline_count];
            auto lines_read = jpeg_read_raw_data (&dinfo, (JSAMPIMAGE) yuv, SOME_SIZE);
            // lines read is always 8
            MLOGD<<"lines read "<<lines_read;
            scanline_count+=lines_read;
        }
    }

};

#endif //UVCCAMERA_MJPEGDECODEANDROID_HPP
