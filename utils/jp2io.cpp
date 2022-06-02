#include "jp2io.h"
#include "color.h"
#include "appglobal.h"
#include "loggingparams.h"

#include <errno.h>
#include <string.h>

#include <QFile>
#include <QDir>
#include <QDebug>

#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"


static size_t opj_strnlen_s(const char *src, size_t max_len)
{
  size_t len;

  if (src == NULL) {
      return 0U;
    }
  for (len = 0U; (*src != '\0') && (len < max_len); src++, len++);
  return len;
}

static int opj_strcpy_s(char* dst, size_t dst_size, const char* src)
{
  size_t src_len = 0U;
  if ((dst == NULL) || (dst_size == 0U)) {
      return EINVAL;
    }
  if (src == NULL) {
      dst[0] = '\0';
      return EINVAL;
    }
  src_len = opj_strnlen_s(src, dst_size);
  if (src_len >= dst_size) {
      return ERANGE;
    }
  memcpy(dst, src, src_len);
  dst[src_len] = '\0';
  return 0;
}


/**
sample error callback expecting a FILE* client object
*/
static void error_callback(const char *msg, void *client_data) {
  (void)client_data;
  Q_UNUSED( msg )
  addToLog(QString("[ERROR] %1").arg(msg),LOGIMAG);
}
/**
sample warning callback expecting a FILE* client object
*/
static void warning_callback(const char *msg, void *client_data) {
  (void)client_data;
  Q_UNUSED( msg )
  addToLog(QString("[WARNING] %1").arg(msg),LOGIMAG);
}
/**
sample debug callback expecting no client object
*/
static void info_callback(const char *msg, void *client_data) {
  (void)client_data;
  Q_UNUSED( msg )
  addToLog(QString("[INFO] %1").arg(msg),LOGIMAG);
}

/*!
 * Divide an integer by a power of 2 and round upwards.
 */
// -------------------- int_ceildivpow2() ------------------------------------
static inline int int_ceildivpow2(int a, int b) {
  return (a + (1 << b) - 1) >> b;
}
// ---------------------------------------------------------------------------

jp2IO::jp2IO()
{

}

void  jp2IO::init()
{
  jp2Image = NULL;
  l_stream = NULL;				/* Stream */
  l_codec = NULL;				/* Handle to a decompressor */
  cstr_index = NULL;
}

bool jp2IO::check(QString fileName)
{
  quint64 size;
  char data[12];
  QFile fi(fileName);
  set_default_parameters(&parameters); // set decoding parameters to default values
  if(!fi.open(QIODevice::ReadOnly)) return false;
  size=fi.read(data,12);
  fi.close();
  if(size!=12)
    {
      return false;
    }
  if (memcmp(data, JP2_RFC3745_MAGIC, 12) == 0 || memcmp(data, JP2_MAGIC, 4) == 0)
    {
      magicFormat = JP2_CFMT;
      magicStr = ".jp2";
    }
  else if (memcmp(data, J2K_CODESTREAM_MAGIC, 4) == 0)
    {
      magicFormat  = J2K_CFMT;
      magicStr = ".j2k or .jpc or .j2c";
    }
  else return false;
  parameters.decod_format=magicFormat;
  return true;
}

void jp2IO::set_default_parameters(opj_decompress_parameters* parameters)
{
  if (parameters)
    {
      memset(parameters, 0, sizeof(opj_decompress_parameters));

      /* default decoding parameters (command line specific) */
      parameters->decod_format = -1;
      parameters->cod_format = -1;

      /* default decoding parameters (core) */
      opj_set_default_decoder_parameters(&(parameters->core));
    }
}

void jp2IO::destroy_parameters(opj_decompress_parameters* parameters)
{
  if (parameters)
    {
      if (parameters->precision)
        {
          free(parameters->precision);
          parameters->precision = NULL;
        }
    }
}

void jp2IO::slotStart()
{
  bool result;
  result=check(threadFilename);
  if(result)
    {
      *threadImage=decode(threadFilename).copy();
    }
  emit done(result,fromCache);
}



QImage jp2IO::decode(QString fileName)
{
  QImage qimage;
  int width;  // image width and height
  int wr, hr;         // taken from OpenJPEG
  init();

  l_stream = opj_stream_create_default_file_stream(fileName.toLatin1(),1);
  if (!l_stream)
    {
      addToLog( QString("ERROR -> failed to create the stream from the file %1").arg(parameters.infile),LOGALL);
      destroy_parameters(&parameters);
      return qimage;
    }

  /* decode the JPEG2000 stream */
  /* ---------------------- */

  switch(parameters.decod_format)
    {
    case J2K_CFMT:	/* JPEG-2000 codestream */
      {
        /* Get a decoder handle */
        l_codec = opj_create_decompress(OPJ_CODEC_J2K);
        break;
      }
    case JP2_CFMT:	/* JPEG 2000 compressed image data */
      {
        /* Get a decoder handle */
        l_codec = opj_create_decompress(OPJ_CODEC_JP2);
        break;
      }
    case JPT_CFMT:	/* JPEG 2000, JPIP */
      {
        /* Get a decoder handle */
        l_codec = opj_create_decompress(OPJ_CODEC_JPT);
        break;
      }
    default:
      //      fprintf(stderr, "skipping file..\n");
      destroy_parameters(&parameters);
      opj_stream_destroy(l_stream);
    }

  /* catch events using our callbacks and give a local context */
  opj_set_info_handler(l_codec, info_callback,00);
  opj_set_warning_handler(l_codec, warning_callback,00);
  opj_set_error_handler(l_codec, error_callback,00);

  //  t = opj_clock();

  /* Setup the decoder decoding parameters using user parameters */
  if ( !opj_setup_decoder(l_codec, &(parameters.core)) ){
      addToLog( "ERROR -> opj_decompress: failed to setup the decoder",LOGALL);
      destroy_parameters(&parameters);
      opj_stream_destroy(l_stream);
      opj_destroy_codec(l_codec);
      return qimage;
    }


  /* Read the main header of the codestream and if necessary the JP2 boxes*/
  if(! opj_read_header(l_stream, l_codec, &jp2Image))
    {
      addToLog( "ERROR -> opj_decompress: failed to read the header",LOGALL);
      destroy_parameters(&parameters);
      opj_stream_destroy(l_stream);
      opj_destroy_codec(l_codec);
      opj_image_destroy(jp2Image);
      return qimage;
    }

  if (!parameters.nb_tile_to_decode)
    {
      /* Optional if you want decode the entire image */
      if (!opj_set_decode_area(l_codec, jp2Image, (OPJ_INT32)parameters.DA_x0,
                               (OPJ_INT32)parameters.DA_y0, (OPJ_INT32)parameters.DA_x1, (OPJ_INT32)parameters.DA_y1))
        {
          addToLog( "ERROR -> opj_decompress: failed to set the decoded area",LOGALL);
          destroy_parameters(&parameters);
          opj_stream_destroy(l_stream);
          opj_destroy_codec(l_codec);
          opj_image_destroy(jp2Image);
          return qimage;
        }

      /* Get the decoded image */
      if (!(opj_decode(l_codec, l_stream, jp2Image) && opj_end_decompress(l_codec,	l_stream)))
        {
          addToLog( "ERROR -> opj_decompress: failed to decode image!",LOGALL);
          destroy_parameters(&parameters);
          opj_destroy_codec(l_codec);
          opj_stream_destroy(l_stream);
          opj_image_destroy(jp2Image);
          return qimage;
        }
    }

  /* Close the byte stream */
  opj_stream_destroy(l_stream);

  if( jp2Image->color_space != OPJ_CLRSPC_SYCC
      && jp2Image->numcomps == 3 && jp2Image->comps[0].dx == jp2Image->comps[0].dy
      && jp2Image->comps[1].dx != 1 )
    jp2Image->color_space = OPJ_CLRSPC_SYCC;
  else if (jp2Image->numcomps <= 2)
    jp2Image->color_space = OPJ_CLRSPC_GRAY;

  if(jp2Image->color_space == OPJ_CLRSPC_SYCC){
      color_sycc_to_rgb(jp2Image);
    }

  else if(jp2Image->color_space == OPJ_CLRSPC_EYCC){
      color_esycc_to_rgb(jp2Image);
    }

  if(jp2Image->icc_profile_buf)
    {
      free(jp2Image->icc_profile_buf);
      jp2Image->icc_profile_buf = NULL; jp2Image->icc_profile_len = 0;
    }

  /* Force RGB output */
  /* ---------------- */
  if (parameters.force_rgb)
    {
      switch (jp2Image->color_space) {
        case OPJ_CLRSPC_SRGB:
        break;
        case OPJ_CLRSPC_GRAY:
          jp2Image = convert_gray_to_rgb(jp2Image);
        break;
        default:
          fprintf(stderr, "ERROR -> opj_decompress: don't know how to convert image to RGB colorspace!\n");
          opj_image_destroy(jp2Image);
          jp2Image = NULL;
        break;
        }
      if (jp2Image == NULL) {
          fprintf(stderr, "ERROR -> opj_decompress: failed to convert to RGB image!\n");
          destroy_parameters(&parameters);
          opj_destroy_codec(l_codec);
          return qimage;
        }
    }
  // decode image width and height
  width = jp2Image->comps[0].w;
  wr = int_ceildivpow2(jp2Image->comps[0].w, jp2Image->comps[0].factor);
  //  height = jp2Image->comps[0].h;
  hr = int_ceildivpow2(jp2Image->comps[0].h, jp2Image->comps[0].factor);

  // alloc memory for a new image
  qimage=QImage(wr, hr, QImage::Format_RGB32);

  // get a pointer to the first pixel data in result image
  QRgb *bits = (QRgb *)qimage.bits();
  for(int i = 0; i < wr * hr; i++) {
      unsigned char R, G, B;

      // calculate coords
      int x = (i) % (wr);
      //     int y = height - (i) / (wr) - 1;
      int y = (i) / (wr);
#define idx( _x, _y ) ( _x + _y * width )

      R = jp2Image->comps[0].data[ idx( x, y ) ];
      G = jp2Image->comps[1].data[ idx( x, y ) ];
      B = jp2Image->comps[2].data[ idx( x, y ) ];

      // fill the bit with read pallete
      bits[i] = qRgb(R, G, B);
    }
  destroy_parameters(&parameters);
  opj_destroy_codec(l_codec);
  opj_image_destroy(jp2Image);
  return qimage;
}

opj_image_t *jp2IO::convert_gray_to_rgb(opj_image_t* original)
{
  OPJ_UINT32 compno;
  opj_image_t* l_new_image = NULL;
  opj_image_cmptparm_t* l_new_components = NULL;

  l_new_components = (opj_image_cmptparm_t*)malloc((original->numcomps + 2U) * sizeof(opj_image_cmptparm_t));
  if (l_new_components == NULL) {
      fprintf(stderr, "ERROR -> opj_decompress: failed to allocate memory for RGB image!\n");
      opj_image_destroy(original);
      return NULL;
    }

  l_new_components[0].bpp  = l_new_components[1].bpp  = l_new_components[2].bpp  = original->comps[0].bpp;
  l_new_components[0].dx   = l_new_components[1].dx   = l_new_components[2].dx   = original->comps[0].dx;
  l_new_components[0].dy   = l_new_components[1].dy   = l_new_components[2].dy   = original->comps[0].dy;
  l_new_components[0].h    = l_new_components[1].h    = l_new_components[2].h    = original->comps[0].h;
  l_new_components[0].w    = l_new_components[1].w    = l_new_components[2].w    = original->comps[0].w;
  l_new_components[0].prec = l_new_components[1].prec = l_new_components[2].prec = original->comps[0].prec;
  l_new_components[0].sgnd = l_new_components[1].sgnd = l_new_components[2].sgnd = original->comps[0].sgnd;
  l_new_components[0].x0   = l_new_components[1].x0   = l_new_components[2].x0   = original->comps[0].x0;
  l_new_components[0].y0   = l_new_components[1].y0   = l_new_components[2].y0   = original->comps[0].y0;

  for(compno = 1U; compno < original->numcomps; ++compno) {
      l_new_components[compno+2U].bpp  = original->comps[compno].bpp;
      l_new_components[compno+2U].dx   = original->comps[compno].dx;
      l_new_components[compno+2U].dy   = original->comps[compno].dy;
      l_new_components[compno+2U].h    = original->comps[compno].h;
      l_new_components[compno+2U].w    = original->comps[compno].w;
      l_new_components[compno+2U].prec = original->comps[compno].prec;
      l_new_components[compno+2U].sgnd = original->comps[compno].sgnd;
      l_new_components[compno+2U].x0   = original->comps[compno].x0;
      l_new_components[compno+2U].y0   = original->comps[compno].y0;
    }

  l_new_image = opj_image_create(original->numcomps + 2U, l_new_components, OPJ_CLRSPC_SRGB);
  free(l_new_components);
  if (l_new_image == NULL) {
      fprintf(stderr, "ERROR -> opj_decompress: failed to allocate memory for RGB image!\n");
      opj_image_destroy(original);
      return NULL;
    }

  l_new_image->x0 = original->x0;
  l_new_image->x1 = original->x1;
  l_new_image->y0 = original->y0;
  l_new_image->y1 = original->y1;

  l_new_image->comps[0].factor        = l_new_image->comps[1].factor        = l_new_image->comps[2].factor        = original->comps[0].factor;
  l_new_image->comps[0].alpha         = l_new_image->comps[1].alpha         = l_new_image->comps[2].alpha         = original->comps[0].alpha;
  l_new_image->comps[0].resno_decoded = l_new_image->comps[1].resno_decoded = l_new_image->comps[2].resno_decoded = original->comps[0].resno_decoded;

  memcpy(l_new_image->comps[0].data, original->comps[0].data, original->comps[0].w * original->comps[0].h * sizeof(OPJ_INT32));
  memcpy(l_new_image->comps[1].data, original->comps[0].data, original->comps[0].w * original->comps[0].h * sizeof(OPJ_INT32));
  memcpy(l_new_image->comps[2].data, original->comps[0].data, original->comps[0].w * original->comps[0].h * sizeof(OPJ_INT32));

  for(compno = 1U; compno < original->numcomps; ++compno) {
      l_new_image->comps[compno+2U].factor        = original->comps[compno].factor;
      l_new_image->comps[compno+2U].alpha         = original->comps[compno].alpha;
      l_new_image->comps[compno+2U].resno_decoded = original->comps[compno].resno_decoded;
      memcpy(l_new_image->comps[compno+2U].data, original->comps[compno].data, original->comps[compno].w * original->comps[compno].h * sizeof(OPJ_INT32));
    }
  opj_image_destroy(original);
  return l_new_image;
}

QByteArray jp2IO::encode(QImage qimage, QImage &newImage,int &fileSize, int compressionRatio)
{
  QByteArray byteArray;
  QString fn=QString("%1/%2").arg(QDir::tempPath()).arg("qsstv.tmp");
  //  char *of=fn.toLatin1().data();
  bool success;
  init();
  fileSize=0;
  opj_set_default_encoder_parameters(&cparameters);
  cparameters.cp_disto_alloc = 1;
  createImage(qimage);
  cparameters.cod_format=JP2_CFMT;
  if(opj_strcpy_s(cparameters.outfile, sizeof(cparameters.outfile), fn.toLatin1().data()) != 0)
    {
      return byteArray;
    }
  raw_cp.rawBitDepth = 0;
  raw_cp.rawComp = 0;
  raw_cp.rawComps = 0;
  raw_cp.rawHeight = 0;
  raw_cp.rawSigned = 0;
  raw_cp.rawWidth = 0;
  //  cparameters.tcp_mct = (char) 255; // This will be set later according to the input image or the provided option
  cparameters.tcp_mct = (jp2Image->numcomps >= 3) ? 1 : 0;

  cparameters.tcp_rates[0] = compressionRatio;	/* MOD antonin : losslessbug */
  cparameters.tcp_numlayers++;
  cparameters.cp_disto_alloc = 1;
  l_codec = opj_create_compress(OPJ_CODEC_JP2);
  /* catch events using our callbacks and give a local context */
  opj_set_info_handler(l_codec, info_callback,00);
  opj_set_warning_handler(l_codec, warning_callback,00);
  opj_set_error_handler(l_codec, error_callback,00);
  /* open a byte stream for writing and allocate memory for all tiles */


  if (! opj_setup_encoder(l_codec, &cparameters, jp2Image))
    {
      addToLog("failed to encode image: opj_setup_encoder",LOGIMAG);
      opj_destroy_codec(l_codec);
      opj_image_destroy(jp2Image);
      return byteArray;
    }

  l_stream = opj_stream_create_default_file_stream(cparameters.outfile,OPJ_FALSE);
  if (! l_stream)
    {
      opj_destroy_codec(l_codec);
      opj_image_destroy(jp2Image);
      return byteArray;
    }
  /* encode the image */
  success = opj_start_compress(l_codec,jp2Image,l_stream);
  if (!success)
    {
      addToLog("failed to encode image: opj_start_compress",LOGIMAG);
    }
  success = success && opj_encode(l_codec, l_stream);
  if (!success)
    {
      addToLog("failed to encode image: opj_encode",LOGIMAG);
    }
  success = success && opj_end_compress(l_codec, l_stream);
  if (!success)
    {
      addToLog("failed to encode image: opj_end_compress",LOGIMAG);
    }

  if (!success)
    {
      opj_stream_destroy(l_stream);
      opj_destroy_codec(l_codec);
      opj_image_destroy(jp2Image);
      addToLog("failed to encode image",LOGIMAG);
      remove(parameters.outfile);
      return byteArray;
    }

  opj_stream_destroy(l_stream);
  opj_destroy_codec(l_codec);
  opj_image_destroy(jp2Image);

  /* free user parameters structure */
  if(cparameters.cp_comment)   free(cparameters.cp_comment);
  if(cparameters.cp_matrice)   free(cparameters.cp_matrice);
  if(raw_cp.rawComps) free(raw_cp.rawComps);
  QFile fi(fn);
  if(fi.open(QIODevice::ReadOnly))
    {
      byteArray=fi.readAll();
      fileSize=byteArray.count();
    }
  fi.close();
  check(fn);
  newImage=decode(fn);
  fi.remove();
  remove(parameters.outfile);
  return byteArray;
}

bool jp2IO::createImage(QImage qimage)
{
  int i;
  int numcmpts=3;
  opj_image_cmptparm_t cmptparm[4];	/* maximum of 4 components */
  memset(&cmptparm[0], 0, sizeof(cmptparm));
  for(i = 0; i < 4; i++)
    {
      cmptparm[i].prec = 8;
      cmptparm[i].bpp  = 8;
      cmptparm[i].sgnd = 0;
      cmptparm[i].dx   = (OPJ_UINT32)cparameters.subsampling_dx;
      cmptparm[i].dy   = (OPJ_UINT32)cparameters.subsampling_dy;
      cmptparm[i].w    = qimage.width();
      cmptparm[i].h    = qimage.height();
    }

  jp2Image = opj_image_create(numcmpts, &cmptparm[0],OPJ_CLRSPC_SRGB);
  if(!jp2Image) return false;
  if (numcmpts == 4)
    {
      jp2Image->comps[3].alpha = 1;
    }

  /* set image offset and reference grid */
  jp2Image->x0 = (OPJ_UINT32)cparameters.image_offset_x0;
  jp2Image->y0 = (OPJ_UINT32)cparameters.image_offset_y0;
  jp2Image->x1 =	jp2Image->x0 + (qimage.width()  - 1U) * (OPJ_UINT32)cparameters.subsampling_dx + 1U;
  jp2Image->y1 = jp2Image->y0 + (qimage.height() - 1U) * (OPJ_UINT32)cparameters.subsampling_dy + 1U;


  int width  = jp2Image->comps[0].w;
  int height = jp2Image->comps[0].h;

  QRgb *bits;
  QRgb color;
  int index = 0;
  int x,y;
  bits= (QRgb *)qimage.bits();
  for(y = 0; y < height; y++)
    {
      for(x = 0; x < width; x++)
        {
          color=bits[x+y* width];
          jp2Image->comps[0].data[index] = (OPJ_INT32)qRed(color);	/* R */
          jp2Image->comps[1].data[index] = (OPJ_INT32)qGreen(color);	/* G */
          jp2Image->comps[2].data[index] = (OPJ_INT32)qBlue(color);	/* B */
          index++;
        }
    }
  return true;
}

