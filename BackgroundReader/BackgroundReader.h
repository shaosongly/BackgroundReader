#pragma once

#include "resource.h"
#include <highgui.h>
#include <cxcore.h>
#include <cvaux.h>
#include <string>

using namespace std;
#define CHANNEL 3

typedef struct CvBackgroundData//背景数据（负样本数据）
{
    int    count;//负样本数量
    char** filename;//所有负样本的路径
    int    last;//保留
    int    round;//保留
    CvSize winsize;//样本尺寸
} CvBackgroundData;

typedef struct CvBackgroundReader
{
    CvMat   src;
    CvMat   img;
    CvPoint offset;
    float   scale;
    float   scalefactor;
    float   stepfactor;
    CvPoint point;
} CvBackgroundReader;

/*
 * Background reader
 * Created in each thread
 */
CvBackgroundReader* cvbgreader = NULL;
CvBackgroundData* cvbgdata = NULL;


static
CvBackgroundData* icvCreateBackgroundData( const char* filename, CvSize winsize )
{
    CvBackgroundData* data = NULL;

    const char* dir = NULL;    
    char full[200];
    char* imgfilename = NULL;
    size_t datasize = 0;
    int    count = 0;
    FILE*  input = NULL;
    char*  tmp   = NULL;
    int    len   = 0;

    assert( filename != NULL );
    
    dir = strrchr( filename, '\\' );
    if( dir == NULL )
    {
        dir = strrchr( filename, '/' );
    }
    if( dir == NULL )
    {
        imgfilename = &(full[0]);
    }
    else
    {
        strncpy( &(full[0]), filename, (dir - filename + 1) );
        imgfilename = &(full[(dir - filename + 1)]);
    }

    input = fopen( filename, "r" );
    if( input != NULL )
    {
        count = 0;
        datasize = 0;
        
        /* count */
        while( !feof( input ) )
        {
            *imgfilename = '\0';
            if( !fscanf( input, "%s", imgfilename ))
                break;
            len = (int)strlen( imgfilename );
            if( len > 0 )
            {
                if( (*imgfilename) == '#' ) continue; /* comment */
                count++;
                datasize += sizeof( char ) * (strlen( &(full[0]) ) + 1);
            }
        }
        if( count > 0 )
        {
            //rewind( input );
            fseek( input, 0, SEEK_SET );
            datasize += sizeof( *data ) + sizeof( char* ) * count;
            data = (CvBackgroundData*) cvAlloc( datasize );
            memset( (void*) data, 0, datasize );
            data->count = count;
            data->filename = (char**) (data + 1);
            data->last = 0;
            data->round = 0;
            data->winsize = winsize;
            tmp = (char*) (data->filename + data->count);
            count = 0;
            while( !feof( input ) )
            {
                *imgfilename = '\0';
                if( !fscanf( input, "%s", imgfilename ))
                    break;
                len = (int)strlen( imgfilename );
                if( len > 0 )
                {
                    if( (*imgfilename) == '#' ) continue; /* comment */
                    data->filename[count++] = tmp;
                    strcpy( tmp, &(full[0]) );
                    tmp += strlen( &(full[0]) ) + 1;
                }
            }
        }
        fclose( input );
    }
    return data;
}

static
void icvReleaseBackgroundData( CvBackgroundData** data )
{
    assert( data != NULL && (*data) != NULL );

    cvFree( data );
}

static
CvBackgroundReader* icvCreateBackgroundReader()
{
    CvBackgroundReader* reader = NULL;

    reader = (CvBackgroundReader*) cvAlloc( sizeof( *reader ) );
    memset( (void*) reader, 0, sizeof( *reader ) );
	if(CHANNEL==3)
	{
		reader->src = cvMat( 0, 0, CV_8UC3, NULL );
		reader->img = cvMat( 0, 0, CV_8UC3, NULL );
	}
	else
	{
		reader->src = cvMat( 0, 0, CV_8UC1, NULL );
		reader->img = cvMat( 0, 0, CV_8UC1, NULL );
	}
    reader->offset = cvPoint( 0, 0 );
    reader->scale       = 1.0F;
    reader->scalefactor = 1.4142135623730950488016887242097F;
    reader->stepfactor  = 0.5F;
    reader->point = reader->offset;

    return reader;
}

static
void icvReleaseBackgroundReader( CvBackgroundReader** reader )
{
    assert( reader != NULL && (*reader) != NULL );

    if( (*reader)->src.data.ptr != NULL )
    {
        cvFree( &((*reader)->src.data.ptr) );
    }
    if( (*reader)->img.data.ptr != NULL )
    {
        cvFree( &((*reader)->img.data.ptr) );
    }

    cvFree( reader );
}

static
void icvGetNextFromBackgroundData( CvBackgroundData* data,
                                   CvBackgroundReader* reader )
{
    IplImage* img = NULL;
    size_t datasize = 0;
    int round = 0;
    int i = 0;
    CvPoint offset = cvPoint(0,0);

    assert( data != NULL && reader != NULL );

    if( reader->src.data.ptr != NULL )
    {
        cvFree( &(reader->src.data.ptr) );
        reader->src.data.ptr = NULL;
    }
    if( reader->img.data.ptr != NULL )
    {
        cvFree( &(reader->img.data.ptr) );
        reader->img.data.ptr = NULL;
    }

    {
        for( i = 0; i < data->count; i++ )
        {
            round = data->round;

//#ifdef CV_VERBOSE 
//            printf( "Open background image: %s\n", data->filename[data->last] );
//#endif /* CV_VERBOSE */
			cout<<data->filename[data->last]<<endl;
            if(CHANNEL==3)
				img = cvLoadImage( data->filename[data->last++], CV_LOAD_IMAGE_COLOR );
			else
				img = cvLoadImage( data->filename[data->last++], CV_LOAD_IMAGE_GRAYSCALE );
            if( !img )
                continue;
            data->round += data->last / data->count;
            data->round = data->round % (data->winsize.width * data->winsize.height);
            data->last %= data->count;

            offset.x = round % data->winsize.width;
            offset.y = round / data->winsize.width;

            offset.x = MIN( offset.x, img->width - data->winsize.width );
            offset.y = MIN( offset.y, img->height - data->winsize.height );
            
            if( img != NULL && img->depth == IPL_DEPTH_8U  && offset.x >= 0 && offset.y >= 0 )
            {
                break;
            }
            if( img != NULL )
                cvReleaseImage( &img );
            img = NULL;
        }
    }
    if( img == NULL )
    {
        /* no appropriate image */

#ifdef CV_VERBOSE
        printf( "Invalid background description file.\n" );
#endif /* CV_VERBOSE */

        assert( 0 );
        exit( 1 );
    }
    datasize = sizeof( uchar ) * img->width * img->height*CHANNEL;
	if(CHANNEL==3)
		reader->src = cvMat( img->height, img->width, CV_8UC3, (void*) cvAlloc( datasize ) );
	else
		reader->src = cvMat( img->height, img->width, CV_8UC1, (void*) cvAlloc( datasize ) );
    cvCopy( img, &reader->src, NULL );
    cvReleaseImage( &img );
    img = NULL;

    //reader->offset.x = round % data->winsize.width;
    //reader->offset.y = round / data->winsize.width;
    reader->offset = offset;
    reader->point = reader->offset;
    reader->scale = MAX(
        ((float) data->winsize.width + reader->point.x) / ((float) reader->src.cols),
        ((float) data->winsize.height + reader->point.y) / ((float) reader->src.rows) );
    if(CHANNEL==3)
		reader->img = cvMat( (int) (reader->scale * reader->src.rows + 0.5F),(int) (reader->scale * reader->src.cols + 0.5F), CV_8UC3, (void*) cvAlloc( datasize ) );
	else
		reader->img = cvMat( (int) (reader->scale * reader->src.rows + 0.5F),(int) (reader->scale * reader->src.cols + 0.5F), CV_8UC1, (void*) cvAlloc( datasize ) );
    cvResize( &(reader->src), &(reader->img) );
}

static 
void icvGetBackgroundImage( CvBackgroundData* data,
                            CvBackgroundReader* reader,
                            CvMat* img )
{
    CvMat mat;

    assert( data != NULL && reader != NULL && img != NULL );
    //assert( CV_MAT_TYPE( img->type ) == CV_8UC1 );
    assert( img->cols == data->winsize.width );
    assert( img->rows == data->winsize.height );

    if( reader->img.data.ptr == NULL )
    {
        icvGetNextFromBackgroundData( data, reader );
    }
	if(CHANNEL==3)
		mat = cvMat( data->winsize.height, data->winsize.width, CV_8UC3 );
	else
		mat = cvMat( data->winsize.height, data->winsize.width, CV_8UC1 );

    cvSetData( &mat, (void*) (reader->img.data.ptr + reader->point.y * reader->img.step
                              + reader->point.x * sizeof( uchar )*CHANNEL), reader->img.step );

    cvCopy( &mat, img, 0 );
	//reader 水平方向移动
    if( (int) ( reader->point.x + (1.0F + reader->stepfactor ) * data->winsize.width )
            < reader->img.cols )
    {
        reader->point.x += (int) (reader->stepfactor * data->winsize.width);
    }
    else
    {
        reader->point.x = reader->offset.x;
		//reader 竖直方向移动
        if( (int) ( reader->point.y + (1.0F + reader->stepfactor ) * data->winsize.height )
                < reader->img.rows )
        {
            reader->point.y += (int) (reader->stepfactor * data->winsize.height);
        }
        else
        {
            reader->point.y = reader->offset.y;
            reader->scale *= reader->scalefactor;//缩放系数逐渐变大
            if( reader->scale <= 1.0F )//如果缩放比例小于1，则层层缩小原图
            {
				if(CHANNEL==3)
					reader->img = cvMat( (int) (reader->scale * reader->src.rows),(int) (reader->scale * reader->src.cols),CV_8UC3, (void*) (reader->img.data.ptr) );
				else
					reader->img = cvMat( (int) (reader->scale * reader->src.rows),(int) (reader->scale * reader->src.cols),CV_8UC1, (void*) (reader->img.data.ptr) );
                cvResize( &(reader->src), &(reader->img) );
            }
            else//否则扫描下一幅图像
            {
                icvGetNextFromBackgroundData( data, reader );
            }
        }
    }
}

static
int icvInitBackgroundReaders( const char* filename, CvSize winsize )
{
    if( cvbgdata == NULL && filename != NULL )
    {
        cvbgdata = icvCreateBackgroundData( filename, winsize );
    }

    if( cvbgdata )
    {
		if( cvbgreader == NULL )
        {
			cvbgreader = icvCreateBackgroundReader();
		}
    }
    return (cvbgdata != NULL);
}


//* icvDestroyBackgroundReaders: Finish backgournd reading process
static
void icvDestroyBackgroundReaders()
{
    //* release background reader in each thread 
    {
        #ifdef CV_OPENMP
        #pragma omp critical(c_release_bg_data)
        #endif /* CV_OPENMP */
        {
            if( cvbgreader != NULL )
            {
                icvReleaseBackgroundReader( &cvbgreader );
                cvbgreader = NULL;
            }
        }
    }

    if( cvbgdata != NULL )
    {
        icvReleaseBackgroundData( &cvbgdata );
        cvbgdata = NULL;
    }
}
