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
public:
    // Helper that prints the current configuration of ANativeWindow_Buffer
    static void debugANativeWindowBuffer(const ANativeWindow_Buffer& buffer){
        MLOGD<<"ANativeWindow_Buffer: W H "<<buffer.width<<" "<<buffer.height<<"Stride Format"<<buffer.stride<<" "<<buffer.format;
    }
    // Supports the most common ANativeWindow_Buffer image formats
    // No unnecessary memcpy's & correctly handle stride of ANativeWindow_Buffer
    // input uvc_frame_t frame has to be of type MJPEG
    void DecodeMJPEGtoANativeWindowBuffer(const unsigned char* mpegData,size_t mpegDataSize,const ANativeWindow_Buffer& nativeWindowBuffer){
        MEASURE_FUNCTION_EXECUTION_TIME
        //debugANativeWindowBuffer(nativeWindowBuffer);
        // We need to set the error manager every time else it will crash (I have no idea why )
        // https://stackoverflow.com/questions/11613040/why-does-jpeg-decompress-create-crash-without-error-message
        struct error_mgr jerr;
        dinfo.err = jpeg_std_error(&jerr.super);
        jerr.super.error_exit = _error_exit;

        jpeg_mem_src(&dinfo,mpegData,mpegDataSize);
        jpeg_read_header(&dinfo, TRUE);
        //MLOGD<<"Input color space is "<<dinfo.jpeg_color_space<<" num components "<<dinfo.num_components;
        //unsigned int BYTES_PER_PIXEL;
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
        //CLOGD("dinfo.output_components %d | %d",dinfo.output_components,dinfo.out_color_components);
        const unsigned int scanline_len = ((unsigned int)nativeWindowBuffer.stride) * BYTES_PER_PIXEL;
        JSAMPARRAY jsamparray[dinfo.output_height];
        for(int i=0;i<dinfo.output_height;i++){
            JSAMPROW row = (JSAMPROW)(((unsigned char*)nativeWindowBuffer.bits) + (i*scanline_len));
            jsamparray[i]=(JSAMPARRAY)row;
        }
        unsigned int scanline_count = 0;
        while (dinfo.output_scanline < dinfo.output_height){
            // JSAMPROW row = (JSAMPROW)(((unsigned char*)nativeWindowBuffer.bits) + (scanline_count * scanline_len));
            JSAMPROW row2= (JSAMPROW)jsamparray[scanline_count];
            auto lines_read=jpeg_read_scanlines(&dinfo,&row2, 8);
            // unfortunately reads only one line at a time CLOGD("Lines read %d",lines_read);
            scanline_count+=lines_read;
        }
        jpeg_finish_decompress(&dinfo);
    }

    void DecodeMJPEGtoEncoderBuffer(const unsigned char* mpegData,size_t mpegDataSize,void* bits,int stride){
        MEASURE_FUNCTION_EXECUTION_TIME
        //debugANativeWindowBuffer(nativeWindowBuffer);
        // We need to set the error manager every time else it will crash (I have no idea why )
        // https://stackoverflow.com/questions/11613040/why-does-jpeg-decompress-create-crash-without-error-message
        struct error_mgr jerr;
        dinfo.err = jpeg_std_error(&jerr.super);
        jerr.super.error_exit = _error_exit;

        jpeg_mem_src(&dinfo,mpegData,mpegDataSize);
        jpeg_read_header(&dinfo, TRUE);

        MLOGD<<"Input color space is "<<dinfo.jpeg_color_space<<" num components "<<dinfo.num_components<<" data precision "<<dinfo.data_precision;
        MLOGD<<"h samp factor"<<dinfo.comp_info[0].h_samp_factor<<"v samp factor "<<dinfo.comp_info[0].v_samp_factor;
        //unsigned int BYTES_PER_PIXEL;
        dinfo.out_color_space = JCS_YCbCr;
        float BYTES_PER_PIXEL=1.5f;
        //dinfo.raw_data_out=true;
        dinfo.dct_method = JDCT_IFAST;
        jpeg_start_decompress(&dinfo);
        dinfo.raw_data_out=true;

        if (dinfo.comp_info[0].h_samp_factor == 2)
        {
            if (dinfo.comp_info[0].v_samp_factor == 2)
            {
                MLOGD<<"V4L2_PIX_FMT_YUV420M";
            }
            else
            {
                MLOGD<<"V4L2_PIX_FMT_YUV422M";
            }
        }
        else
        {
            if (dinfo.comp_info[0].v_samp_factor == 1)
            {
                MLOGD<<"V4L2_PIX_FMT_YUV444M";
            }
            else
            {
                MLOGD<<" V4L2_PIX_FMT_YUV422RM";
            }
        }
        // libjpeg error ? - output_components is 3 ofr RGB_565 ?
        //CLOGD("dinfo.output_components %d | %d",dinfo.output_components,dinfo.out_color_components);
        //dinfo.

        const unsigned int scanline_len = ((unsigned int)stride) * BYTES_PER_PIXEL;
        JSAMPARRAY jsamparray[dinfo.output_height];
        for(int i=0;i<dinfo.output_height;i++){
            JSAMPROW row = (JSAMPROW)(((unsigned char*)bits) + (i*scanline_len));
            jsamparray[i]=(JSAMPARRAY)row;
        }
        unsigned int scanline_count = 0;
        while (dinfo.output_scanline < dinfo.output_height){
            // JSAMPROW row = (JSAMPROW)(((unsigned char*)nativeWindowBuffer.bits) + (scanline_count * scanline_len));
            JSAMPROW row2= (JSAMPROW)jsamparray[scanline_count];
            auto lines_read=jpeg_read_scanlines(&dinfo,&row2, 8);
            //JSAMPIMAGE row2= (JSAMPIMAGE)jsamparray[scanline_count];
            //auto lines_read=jpeg_read_raw_data(&dinfo,&row2, 8);
            // unfortunately reads only one line at a time CLOGD("Lines read %d",lines_read);
             //jpeg_read_raw_data()
            scanline_count+=lines_read;
        }
        //jpeg_read_raw_data()
        //
        jpeg_finish_decompress(&dinfo);

    }

    class NvBufferPlane{
    public:
        unsigned char data[640*480];
        size_t fmt_width=640;
        size_t fmt_height=480;
    };
    struct NvBuffer{
        NvBufferPlane planes[3];
    };

    void decodeToYUVXXXBuffer(NvBuffer& out_buff, unsigned char * in_buf,unsigned long in_buf_size){
        MEASURE_FUNCTION_EXECUTION_TIME
        // Error manager stuff
        struct error_mgr jerr;
        dinfo.err = jpeg_std_error(&jerr.super);
        jerr.super.error_exit = _error_exit;
        // end error manager
        uint32_t pixel_format = 0;

        dinfo.out_color_space = JCS_YCbCr;
        jpeg_mem_src(&dinfo, in_buf, in_buf_size);
        dinfo.out_color_space = JCS_YCbCr;

        jpeg_read_header(&dinfo, TRUE);
        dinfo.out_color_space = JCS_YCbCr;
        if(dinfo.jpeg_color_space!=dinfo.out_color_space){
            MLOGE<<"Wrong usage";
        }

        //out_buf = new NvBuffer(pixel_format, cinfo.image_width,
        //                       cinfo.image_height, 0);
        //out_buf->allocateMemory();

        dinfo.do_fancy_upsampling = FALSE;
        dinfo.do_block_smoothing = FALSE;
        dinfo.dct_method = JDCT_FASTEST;
        dinfo.raw_data_out = TRUE;
        jpeg_start_decompress (&dinfo);

        /* For some widths jpeglib requires more horizontal padding than I420
         * provides. In those cases we need to decode into separate buffers and then
         * copy over the data into our final picture buffer, otherwise jpeglib might
         * write over the end of a line into the beginning of the next line,
         * resulting in blocky artifacts on the left side of the picture. */
        if (dinfo.output_width % (dinfo.max_h_samp_factor * DCTSIZE)){
            MLOGD<<"decodeIndirect";
            //decodeIndirect(out_buf, pixel_format);
        }else{
            MLOGD<<"decodeDirect";
            decodeDirect(&out_buff);
        }

        jpeg_finish_decompress(&dinfo);
        //*buffer = out_buf;

        MLOGD<<"Succesfully decoded Buffer ";
    }

    void decodeDirect(NvBuffer *out_buf)
    {
        unsigned char **yuv[3];
        unsigned char *y[4 * DCTSIZE] = { NULL, };
        unsigned char *u[4 * DCTSIZE] = { NULL, };
        unsigned char *v[4 * DCTSIZE] = { NULL, };
        int v_samp_factor[3];
        unsigned char *base[3];
        unsigned char *last[3];
        int stride[3];

        yuv[0] = y;
        yuv[1] = u;
        yuv[2] = v;

        for (int i = 0; i < 3; i++){
            v_samp_factor[i] = dinfo.comp_info[i].v_samp_factor;
            stride[i] = out_buf->planes[i].fmt_width;
            base[i] = out_buf->planes[i].data;
            last[i] = base[i] + (stride[i] * (out_buf->planes[i].fmt_height - 1));
        }

        for (int i = 0; i < (int) dinfo.image_height; i += v_samp_factor[0] * DCTSIZE){
            for (int j = 0; j < (v_samp_factor[0] * DCTSIZE); ++j){
                /* Y */
                yuv[0][j] = base[0] + (i + j) * stride[0];

                /* U,V */
                // pixel_format == V4L2_PIX_FMT_YUV420M
                if (false)
                {
                    /* Y */
                    yuv[0][j] = base[0] + (i + j) * stride[0];
                    if ((yuv[0][j] > last[0]))
                        yuv[0][j] = last[0];
                    /* U */
                    if (v_samp_factor[1] == v_samp_factor[0]) {
                        yuv[1][j] = base[1] + ((i + j) / 2) * stride[1];
                    } else if (j < (v_samp_factor[1] * DCTSIZE)) {
                        yuv[1][j] = base[1] + ((i / 2) + j) * stride[1];
                    }
                    if ((yuv[1][j] > last[1]))
                        yuv[1][j] = last[1];
                    /* V */
                    if (v_samp_factor[2] == v_samp_factor[0]) {
                        yuv[2][j] = base[2] + ((i + j) / 2) * stride[2];
                    } else if (j < (v_samp_factor[2] * DCTSIZE)) {
                        yuv[2][j] = base[2] + ((i / 2) + j) * stride[2];
                    }
                    if ((yuv[2][j] > last[2]))
                        yuv[2][j] = last[2];
                }else{
                    yuv[1][j] = base[1] + (i + j) * stride[1];
                    yuv[2][j] = base[2] + (i + j) * stride[2];
                }
            }

            int lines = jpeg_read_raw_data (&dinfo, (JSAMPIMAGE) yuv, v_samp_factor[0] * DCTSIZE);
            if ((!lines)){
                MLOGD<<"jpeg_read_raw_data() returned 0";
            }
        }
    }

};

#endif //UVCCAMERA_MJPEGDECODEANDROID_HPP
