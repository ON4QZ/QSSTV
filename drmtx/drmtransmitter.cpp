#include "drmtransmitter.h"
#include "configparams.h"
#include "drm.h"


// timing table;


int partTable[BWs][MODES][PROTECTIONS][QAMS]=
{
  {
    {{96,160,240},{67,140,201}},
    {{49,103,149},{96,160,240}},
    {{67,112,168},{49,83,124}}
  },
  {
    {{104,174,261},{78,163,235}},
    {{54,113,163},{104,174,261}},
    {{78,130,196},{54,90,135}}
  }
};


drmTransmitter::drmTransmitter()
{
  DRMTransmitter=NULL;
}

drmTransmitter::~drmTransmitter()
{

}


void drmTransmitter::init(QByteArray *ba, QString name, QString format, drmTxParams params)
{
  if(DRMTransmitter) delete DRMTransmitter;
  dataLength=ba->count();
  DRMTransmitter=new CDRMTransmitter;
//  int picSize;
  ERobMode RobMod;
  ESpecOcc BW ;
  CParameter::ESymIntMod eNewDepth;
  ECodScheme eNewScheme;
  CMSCProtLev eNewMSCPre;
  CService Service;
  DRMTransmitter->init_base();
//  DRMTransmitter.GetSoundOutInterface()->SetDev(nr_devout);
  DRMTransmitter->GetAudSrcEnc()->ClearPicFileNames();
  TransmParam = DRMTransmitter->GetParameters();
//  picSize=ba->size();
  DRMTransmitter->GetAudSrcEnc()->SetPicFileName(ba,name,format);
  switch (params.robMode)
    {
    case 0 :
      RobMod = RM_ROBUSTNESS_MODE_A;
    break;
    case 1 :
      RobMod = RM_ROBUSTNESS_MODE_B ;
    break ;
    default:
      RobMod = RM_ROBUSTNESS_MODE_E ;
    break;
    }
  switch (params.bandwith)
    {
    case 0 :
      BW = SO_0;
    break;
    default :
      BW = SO_1 ;
    break ;
    }
  TransmParam->InitCellMapTable(RobMod , BW);

  switch (params.interleaver)
    {
    case 0 :
      eNewDepth = CParameter::SI_LONG;
    break ;
    default :
      eNewDepth = CParameter::SI_SHORT;
    break;
    }
  TransmParam->SetInterleaverDepth(eNewDepth);
  switch (params.qam)
    {
    case 0 :
      eNewScheme = CS_1_SM ; //4Bit QAM
    break;
    case 1 :
      eNewScheme = CS_2_SM ; //16Bit QAM
    break ;
    default :
      eNewScheme = CS_3_SM; // 64Bit QAM
    break;
    }
  TransmParam->SetMSCCodingScheme(eNewScheme);
  switch (params.protection)
    {
    case 1 :
      eNewMSCPre.iPartB = 1;  //Norm
    break;
    default:
      eNewMSCPre.iPartB = 0 ; //High
    break ;
    }
  TransmParam->SetMSCProtLev(eNewMSCPre, false);
  Service.iServiceDescr=0;
  Service.iServiceID=0;
  Service.iLanguage=5;
  Service.strLabel=params.callsign.toLatin1().data();
  TransmParam->SetServiceParameters(0,Service);
  DRMTransmitter->Init();
  // calculate transmision time
  duration=(double)(numTxFrames)*0.4*1.005; // 1.005 ->some extra time for buffers
}

void drmTransmitter::start(bool startTx)
{

  if(startTx)
    {
      stopDRM=false;
      DRMTransmitter->Start();
    }
  else
    {
      stopDRM=true;
    }
}





