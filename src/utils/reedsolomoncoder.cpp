#include "reedsolomoncoder.h"
#include <QFileInfo>
#include <QDebug>
#include "rs.h"
#include "configparams.h"

#define RSBSIZE 255
#define rse32 encode_rs
#define rsd32 eras_dec_rs
#define ENCODE 0
#define DECODE 1

QString rsTypeStr[RST4+1]=
{
  "",
  "rs1",
  "rs2",
  "rs3",
  "rs4"
};

reedSolomonCoder::reedSolomonCoder()
{
  zeroPositions=NULL;
  newZeroPositions=NULL;
}

reedSolomonCoder::~reedSolomonCoder()
{
  if(zeroPositions!=NULL) delete zeroPositions;
  if(newZeroPositions!=NULL) delete newZeroPositions;
}

void reedSolomonCoder::init()
{
  ec_buf.clear();  /* pointer to encoding/decoding buffer */
  tr_buf.clear();  /* pointer to transmit-buffer (fread/fwrite) */
  bk_buf.clear();  /* pointer to backup-buffer for resync */

  rs_bsize=0;
  rs_dsize=0;
  bep_size=0;
  sumOfFailures=0;
  uncorrectableFailures=0;
  if(zeroPositions!=NULL) delete zeroPositions;
  if(newZeroPositions!=NULL) delete newZeroPositions;
}


// if the extension is not rs1,rs2 or rs3 then newFilename is set to fn and return value is true

bool reedSolomonCoder::decode(QByteArray &ba,QString fn,QString &newFileName,QByteArray &baFile,QString extension,QList<int> &erasuresArray)
{

  int i,j;
  int startOfSegment,row;
  //  QByteArray *t;

  init();
  origFileName=fn;
  fpin.setFileName(fn);

  if(extension=="rs1") fileType=RST1;
  else if(extension=="rs2") fileType=RST2;
  else if(extension=="rs3") fileType=RST3;
  else if(extension=="rs4") fileType=RST4;
  else
    {
      return false;
    }
  tr_buf=ba;
  got=tr_buf.size();
  bep_size = got/RSBSIZE;
  if (got % RSBSIZE)
    {
      bep_size++ ;
      tr_buf=tr_buf.leftJustified(bep_size*RSBSIZE,'\0');
    }

  int rest=tr_buf.count()%64;
  if(rest!=0)
    {
      tr_buf=tr_buf.leftJustified(bep_size*RSBSIZE+(64-rest),'\0');


    }
  ec_buf.resize(bep_size*RSBSIZE);
  bk_buf.resize(bep_size*RSBSIZE);

  rs_bsize=RSBSIZE;
  switch (fileType)
    {
    case RST1: rs_dsize=RSDSIZERS1; break;
    case RST2: rs_dsize=RSDSIZERS2; break;
    case RST3: rs_dsize=RSDSIZERS3; break;
    case RST4: rs_dsize=RSDSIZERS4; break;
    case RSTNONE: return false;
    }
  init_rs(rs_dsize);
  // setup erasure info
  numMissing=0;
  if(erasuresArray.count()>=2)  // we have erasure positions
    {
      totalSegments=erasuresArray.at(0);
      segmentLength=erasuresArray.at(1);
      numMissing=erasuresArray.count()-2;
      if(zeroPositions) delete zeroPositions;
      if(newZeroPositions) delete newZeroPositions;
      zeroPositions=new int[segmentLength*(totalSegments+1)];
      newZeroPositions=new int[256*bep_size];
      for(i=0;i<(segmentLength*totalSegments);i++) zeroPositions[i]=-1;
    }
  else
    {
      errorOut() << "no erasure info";
      return false;
    }
  /* now label the erasures positions */
  for (i= 0 ; i < numMissing; i++)
    {
      startOfSegment = erasuresArray.at(i+2)*segmentLength ;  // +2 because of header in array
      for (j=0; j < segmentLength ; j++)
        {
          row = (startOfSegment +j ) / bep_size;
          /* if ( row < rs_dsize)  */
          zeroPositions[startOfSegment+j] = row;
        }
    }
  /*  distribute version pa0mbo for the indexes */
  int *pointzero=newZeroPositions;
  for (i=0; i < bep_size ; i++)
    {
      for (j=0; j < 255 ; j++)
        {
          *(pointzero++) = *(zeroPositions + j*bep_size + i);
        }
    }

  distribute((byte *)tr_buf.data(),(byte *)ec_buf.data(),bep_size,rs_bsize,DECODE);
  if(!decode_and_write())
    {
      //      fpout.close();
      return false;
    }
  //  fpin.close();
  //  fpout.close();
  //  tr_buf=ec_buf;
  if(uncorrectableFailures>0) return false;
  //  if(fpout.open(QIODevice::ReadOnly)<=0) return false;
  //  tr_buf=fpout.readAll();
  if (bep_size != (((unsigned char) tr_buf[1]) + ((unsigned char) tr_buf[2])*256 ))
    {
      errorOut()<< "problems with bep_size coded in file";
      errorOut() << "bep_size: " << bep_size <<" coded size: " << (((unsigned char) tr_buf[1]) + ((unsigned char) tr_buf[2])*256 ) ;
      errorOut() << "bep_sizeaa: " << bep_size <<" coded size: " << (((unsigned char) ba[1]) + ((unsigned char)ba[2])*256 ) ;
      return false;
    }
  coded_file_size = bep_size*rs_dsize - (int)tr_buf[0];
  strncpy(coded_file_ext, tr_buf.data()+3,3);
  coded_file_ext[3]=0;


  QFileInfo fileInfo(origFileName);
  QString baseName=rxDRMImagesPath+"/"+fileInfo.completeBaseName();
  baseName.append(".");
  baseName.append(coded_file_ext);

  tr_buf=tr_buf.right(tr_buf.count()-7);
  tr_buf=tr_buf.left(coded_file_size);

  newFileName=baseName;
  baFile=tr_buf;
  if(uncorrectableFailures>0)
    {
      return false;
    }
  return true;
}


void reedSolomonCoder::distribute(byte *src, byte *dst, int rows, int cols, int reverse)
{
  unsigned int i,j,rc,ri,rl;
  rc=rows*cols;
  ri=0;
  rl = reverse ? cols : rows;


  for(i=0;i<rc;i+=64) // ON4QZ changed from original- > using union gives problems with allignement
    {
      for(j=0;j<64 && i+j<rc;j++)
        {
          *(dst+ri)=*(src+i+j);
          ri+=rl; /* next position in dst */
          if(ri>=rc)
            {
              ri-=rc-1;  /* go around and add one. */
            }
        }
    }

}



/* decode buffer and write to fpout */

bool reedSolomonCoder::decode_and_write()
{

  int i, j;
  int nr_erasures;
  int eras_pos[255];
  tr_buf.clear();

  for(i=0;i<bep_size;i++)
    {
      /* get the erasure positions and their number */
      nr_erasures =0;
      for (j=0; j < rs_bsize ; j++)
        {
          if (newZeroPositions[i*rs_bsize + j] != -1)
            {
              eras_pos[nr_erasures++]= newZeroPositions[i*rs_bsize+j] ;
              if(nr_erasures>(rs_bsize-rs_dsize))
                {
                  return false;
                }
            }
        }
      if(nr_erasures>(rs_bsize-rs_dsize)) nr_erasures=rs_bsize-rs_dsize-1;
      int failure=rsd32(((byte *)ec_buf.data()+(i*rs_bsize)),eras_pos, nr_erasures);
      if (failure>0)
        {
          sumOfFailures+=failure;
        }
      else if (failure==-1)
        {
          uncorrectableFailures++;
          return false;
        }
      tr_buf.append(ec_buf.data()+i*rs_bsize,rs_dsize);
    }
  return true;
}

bool reedSolomonCoder::encode(QByteArray &ba,QString extension,eRSType rsType)
{
  int i,j;
  unsigned char dataByte;
  QByteArray temp;
  QString suffix=extension.leftJustified(3,0);
  tr_buf=ba;
  fileType=rsType;
  rs_bsize=RSBSIZE;
  switch (fileType)
    {
    case RST1: rs_dsize=RSDSIZERS1; break;
    case RST2: rs_dsize=RSDSIZERS2; break;
    case RST3: rs_dsize=RSDSIZERS3; break;
    case RST4: rs_dsize=RSDSIZERS4; break;
    case RSTNONE: return false;
    }
  init_rs(rs_dsize);
  got = tr_buf.size();
  chunks = (got+7) / rs_dsize ;
  if (((got+7) % rs_dsize ) > 0) chunks++ ;
  bep_size=chunks;
  //  ec_buf.resize(bep_size*RSBSIZE);
  ec_buf.clear();
  bk_buf.resize(bep_size*RSBSIZE);

  dataByte = (unsigned char) ( rs_dsize - ( got % rs_dsize)) ; /* surplus in filelength */
  ec_buf.append(dataByte);
  dataByte = (unsigned char) ( chunks % 256) ;
  ec_buf.append(dataByte);
  dataByte = (unsigned char) (chunks/256) ;
  ec_buf.append(dataByte);
  ec_buf.append(suffix.toLatin1().at(0));
  ec_buf.append(suffix.toLatin1().at(1));
  ec_buf.append(suffix.toLatin1().at(2));
  dataByte=0;
  ec_buf.append(dataByte);
  ec_buf.append(tr_buf.left(rs_dsize-7));
  ec_buf.resize(ec_buf.count()+RSBSIZE-rs_dsize);
  rse32(((byte *)ec_buf.data()),((byte *)ec_buf.data()+(rs_dsize)));
  for (i=1;i<bep_size;i++)
    {
      temp=tr_buf.mid(i*rs_dsize-7,rs_dsize);
      if(temp.count()==0) break;
      ec_buf.append(temp);
      if(temp.count()<rs_dsize)
        {
          for(j=0;j<(rs_dsize-temp.count());j++)
            {
              ec_buf.append((char)0);
            }
        }
      ec_buf.resize(ec_buf.count()+RSBSIZE-rs_dsize);
      rse32(((byte *)ec_buf.data()+i*rs_bsize),((byte *)ec_buf.data()+i*rs_bsize+rs_dsize));
    }
  ba.resize(ec_buf.count());
  distribute((byte *)ec_buf.data(),(byte *)ba.data(),bep_size,rs_bsize,ENCODE);
  return true;
}

