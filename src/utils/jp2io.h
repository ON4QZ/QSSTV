#ifndef JP2READER_H
#define JP2READER_H
#include "openjpeg.h"

#include <QString>
#include <QImage>

#define J2K_CFMT 0
#define JP2_CFMT 1
#define JPT_CFMT 2

typedef enum opj_prec_mode
{
  OPJ_PREC_MODE_CLIP,
  OPJ_PREC_MODE_SCALE
} opj_precision_mode;

typedef struct opj_prec
{
  OPJ_UINT32         prec;
  opj_precision_mode mode;
}opj_precision;

// used in encode

/**@name RAW component encoding parameters */
/*@{*/
typedef struct raw_comp_cparameters {
    /** subsampling in X direction */
    int dx;
    /** subsampling in Y direction */
    int dy;
    /*@}*/
} raw_comp_cparameters_t;


/**@name RAW image encoding parameters */
/*@{*/
typedef struct raw_cparameters {
  /** width of the raw image */
  int rawWidth;
  /** height of the raw image */
  int rawHeight;
    /** number of components of the raw image */
  int rawComp;
    /** bit depth of the raw image */
    int rawBitDepth;
    /** signed/unsigned raw image */
    OPJ_BOOL rawSigned;
    /** raw components parameters */
    raw_comp_cparameters_t *rawComps;
  /*@}*/
} raw_cparameters_t;






typedef struct opj_decompress_params
{
  opj_dparameters_t core;  // core library parameters
  char infile[OPJ_PATH_LEN]; // input file name
  char outfile[OPJ_PATH_LEN]; // output file name
  int decod_format; // input file format 0: J2K, 1: JP2, 2: JPT
  int cod_format; // output file format 0: PGX, 1: PxM, 2: BMP
  char indexfilename[OPJ_PATH_LEN]; // index file name
  OPJ_UINT32 DA_x0; // Decoding area left boundary
  OPJ_UINT32 DA_x1; // Decoding area right boundary
  OPJ_UINT32 DA_y0; // Decoding area up boundary
  OPJ_UINT32 DA_y1; // Decoding area bottom boundary
  OPJ_BOOL m_verbose; // Verbose mode
  OPJ_UINT32 tile_index; // tile number of the decoded tile
  OPJ_UINT32 nb_tile_to_decode; // Nb of tile to decode
  opj_precision* precision;
  OPJ_UINT32     nb_precision;
  int force_rgb; // force output colorspace to RGB
  int upsample;// upsample components according to their dx/dy values
  int split_pnm;// split output components to different files
}opj_decompress_parameters;




class jp2IO: public QObject
{
  Q_OBJECT
public:
  jp2IO();
  bool check(QString fileName);
  QImage decode(QString fileName);
  QByteArray encode(QImage qimage, QImage &newImage, int &fileSize, int compressionRatio=0 );
  void setParams(QImage *im,QString filename,bool tFromCache)
  {
    threadImage=im;
    threadFilename=filename;
    fromCache=tFromCache;
  }
public slots:
  void slotStart();

signals:
  void done(bool,bool);
private:
  int magicFormat;
  QString magicStr;
  opj_decompress_parameters parameters;			/* decompression parameters */
  opj_cparameters_t cparameters;
  opj_image_t* jp2Image;
  opj_stream_t *l_stream ;				/* Stream */
  opj_codec_t* l_codec;				/* Handle to a decompressor */
  opj_codestream_index_t* cstr_index;
  void set_default_parameters(opj_decompress_parameters* parameters);
  void destroy_parameters(opj_decompress_parameters* parameters);
  opj_image_t *convert_gray_to_rgb(opj_image_t* original);
  void init();
  // used in encode (compress)
  raw_cparameters_t raw_cp;
  bool createImage(QImage qimage);
  QImage *threadImage;
  QString threadFilename;
  bool fromCache;
};

#endif // JP2READER_H
