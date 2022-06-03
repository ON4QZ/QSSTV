#ifndef WAVEIO_H
#define WAVEIO_H

#include "appglobal.h"
#include <QFile>

/*! a WAVE format structure

		The canonical WAVE format starts with the RIFF header:
    \verbatim
0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk 
                               following this number.  This is the size of the 
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).

The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:

12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some 
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters

The "data" subchunk contains the size of the data and the actual sound:

36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this 
                               number.
44        *   Data             The actual sound data.

		\endverbatim 
*/


struct sWave
{
  char chunkID[4];					//!< Contains the letters "RIFF"
  int  chunkSize;						//!< 36 + SubChunk2Size
  char format[4];	 					//!< Contains the letters "WAVE"
  char subChunk1ID[4]; 			//!< Contains the letters "fmt "
  int  subChunk1Size;				//!< 16 for PCM
  short int audioFormat;		//!< PCM = 1 (i.e. Linear quantization)
  short int numChannels;		//!< Mono = 1, Stereo = 2, etc.
  unsigned int sampleRate;	//!< 8000, 44100, etc.
  unsigned int byteRate;		//!< == SampleRate * NumChannels * BitsPerSample/8
  short int blockAlign;			//!< == NumChannels * BitsPerSample/8
  short int bitsPerSample;	//!< 8 bits = 8, 16 bits = 16, etc.
  char subChunk2ID[4];			//!< Contains the letters "data"
  int  subChunk2Size;				//!< NumSamples * NumChannels * BitsPerSample/8
};

//! class for accessing .wav files
class wavIO
{
public:
  wavIO(unsigned int samplingR=BASESAMPLERATE);
	~wavIO();
	bool openFileForRead(QString fname,bool ask);
  bool openFileForWrite(QString fname, bool ask, bool isStereo);
  int  read (short int *dPtr, uint len);
  bool write(quint16 *dPtr, uint len, bool isStereo);
	void setSamplingrate(int sr) {samplingrate=sr;}
  int getNumberOfChannels(){return numberOfChannels;}
  void closeFile();

	/** return the number of samples in the opened file */
	unsigned int getNumberOfSamples()
		{
			return numberOfSamples;
		}
	/** close all opened files */
	void close()
		{
    if(inopf.isOpen())
      {
      write(NULL,0,true); // flush everything in case we are writing
			closeFile();
      }
		}
  private:
    sWave waveHeader;
    unsigned int numberOfSamples;
		unsigned int samplesRead;
		unsigned int samplingrate;
    int numberOfChannels;
    QFile inopf;
    void initHeader();
		bool writeHeader();
    bool checkString(char *str,const char *cstr);
		bool reading;
		bool writing;


};
#endif
