QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += svg widgets network xml

#CONFIG += c++11 enablescope
CONFIG += c++11 silent

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG += -O0 -Wno-implicit-fallthrough -Wno-psabi
QMAKE_CXXFLAGS_RELEASE-= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -Wno-implicit-fallthrough -Wno-psabi

INCLUDEPATH += config dispatch drmrx drmtx dsp editor logbook mainwidgets  rig scope sound sstv utils widgets xmlrpc videocapt
#QMAKE_LIBDIR += $$[QT_SYSROOT]/usr/local/lib

CONFIG += link_pkgconfig
PKGCONFIG += libopenjp2

TARGET = qsstv


SOURCES += main.cpp\
    drmtx/common/CDrmTransmitter.cpp \
    drmtx/common/DRMSignalIO.cpp \
    mainwindow.cpp \
    config/baseconfig.cpp \
    config/soundconfig.cpp \
    utils/dirdialog.cpp \
    utils/logging.cpp \
    utils/loggingparams.cpp \
    appglobal.cpp \
    utils/supportfunctions.cpp \
    sound/wavio.cpp \
    config/directoriesconfig.cpp \
    config/configdialog.cpp \
    sound/soundbase.cpp \
    sound/soundpulse.cpp \
    widgets/spectrumwidget.cpp \
    widgets/vumeter.cpp \
    widgets/fftdisplay.cpp \
    widgets/markerwidget.cpp \
    dsp/downsamplefilter.cpp \
    utils/arraydumper.cpp \
    sound/soundalsa.cpp \
    sound/calibration.cpp \
    dsp/synthes.cpp \
    dsp/filterparam.cpp \
    sound/waterfalltext.cpp \
    utils/macroexpansion.cpp \
    widgets/imageviewer.cpp \
    editor/editor.cpp \
    editor/editorscene.cpp \
    editor/editorview.cpp \
    editor/gradientdialog.cpp \
    editor/graphicitems.cpp \
    widgets/textdisplay.cpp \
    dispatch/dispatcher.cpp \
    drmrx/bits2bytes.cpp \
    drmrx/channeldecode.cpp \
    drmrx/crc16_bytewise.cpp \
    drmrx/crc8_c.cpp \
    drmrx/deinterleaver.cpp \
    drmrx/demodulator.cpp \
    drmrx/drm.cpp \
    drmrx/drmconstellationframe.cpp \
    drmrx/drmpsdframe.cpp \
    drmrx/drmrx.cpp \
    drmrx/drmstatusframe.cpp \
    drmrx/filter1.cpp \
    drmrx/filter1c.cpp \
    drmrx/fixform.cpp \
    drmrx/getfoffsint.cpp \
    drmrx/getmode.cpp \
    drmrx/getofdm.cpp \
    drmrx/getofdmsync.cpp \
    drmrx/getsymbolidx.cpp \
    drmrx/lubksb.cpp \
    drmrx/ludcmp.cpp \
    drmrx/mkfacmap.cpp \
    drmrx/mkmscmap.cpp \
    drmrx/msdhardfac.cpp \
    drmrx/msdhardmsc.cpp \
    drmrx/newfft.cpp \
    drmrx/nrutil.cpp \
    drmrx/psdcmean.cpp \
    drmrx/psdmean.cpp \
    drmrx/sourcedecoder.cpp \
    drmrx/viterbi_decode.cpp \
    drmtx/common/datadecoding/DABMOT.cpp \
    drmtx/common/datadecoding/DataDecoder.cpp \
    drmtx/common/datadecoding/MOTSlideShow.cpp \
    drmtx/common/FAC/FAC.cpp \
    drmtx/common/interleaver/BlockInterleaver.cpp \
    drmtx/common/interleaver/SymbolInterleaver.cpp \
    drmtx/common/matlib/MatlibSigProToolbox.cpp \
    drmtx/common/matlib/MatlibStdToolbox.cpp \
    drmtx/common/mlc/BitInterleaver.cpp \
    drmtx/common/mlc/ChannelCode.cpp \
    drmtx/common/mlc/ConvEncoder.cpp \
    drmtx/common/mlc/EnergyDispersal.cpp \
    drmtx/common/mlc/MLC.cpp \
    drmtx/common/mlc/QAMMapping.cpp \
    drmtx/common/ofdmcellmapping/CellMappingTable.cpp \
    drmtx/common/ofdmcellmapping/OFDMCellMapping.cpp \
    drmtx/common/SDC/SDCTransmit.cpp \
    drmtx/common/sourcedecoders/AudioSourceDecoder.cpp \
    drmtx/common/tables/TableFAC.cpp \
    drmtx/common/util/CRC.cpp \
    drmtx/common/util/Utilities.cpp \
    drmtx/common/csoundout.cpp \
    drmtx/common/DataIO.cpp \
    drmtx/common/OFDM.cpp \
    drmtx/common/Parameter.cpp \
    drmtx/bsrform.cpp \
    drmtx/drmparams.cpp \
    drmtx/drmtransmitter.cpp \
    drmtx/drmtx.cpp \
    logbook/logbook.cpp \
    mainwidgets/gallerywidget.cpp \
    mainwidgets/rxfunctions.cpp \
    mainwidgets/rxwidget.cpp \
    mainwidgets/txfunctions.cpp \
    mainwidgets/txwidget.cpp \
    rig/freqdisplay.cpp \
    rig/rigcontrol.cpp \
    sstv/modes/modeavt.cpp \
    sstv/modes/modebase.cpp \
    sstv/modes/modebw.cpp \
    sstv/modes/modegbr.cpp \
    sstv/modes/modegbr2.cpp \
    sstv/modes/modepd.cpp \
    sstv/modes/modergb.cpp \
    sstv/modes/moderobot1.cpp \
    sstv/modes/moderobot2.cpp \
    sstv/cw.cpp \
    sstv/sstvparam.cpp \
    sstv/sstvrx.cpp \
    sstv/sstvtx.cpp \
    sstv/syncprocessor.cpp \
    utils/qurlinfo.cpp \
    utils/reedsolomoncoder.cpp \
    utils/rs.cpp \
    xmlrpc/ipcmessage.cpp \
    xmlrpc/maiaFault.cpp \
    xmlrpc/maiaObject.cpp \
    xmlrpc/maiaXmlRpcClient.cpp \
    xmlrpc/maiaXmlRpcServer.cpp \
    xmlrpc/maiaXmlRpcServerConnection.cpp \
    xmlrpc/xmlinterface.cpp \
    config/operatorconfig.cpp \
    widgets/extviewer.cpp \
    config/guiconfig.cpp \
    utils/qftp.cpp \
    sound/resamplefilter.cpp \
    widgets/imagematrix.cpp \
    widgets/waterfallform.cpp \
    config/rigconfig.cpp \
    config/cwconfig.cpp \
    config/drmprofileconfig.cpp \
    config/ftpconfig.cpp \
    config/hybridconfig.cpp \
    config/repeaterconfig.cpp \
    config/waterfallconfig.cpp \
    utils/hybridcrypt.cpp \
    videocapt/cameradialog.cpp \
    videocapt/imagesettings.cpp \
    videocapt/v4l2control.cpp \
    videocapt/videocapture.cpp \
    sstv/visfskid.cpp \
    dsp/filters.cpp \
    dsp/filter.cpp \
    widgets/drmsegmentsview.cpp \
    widgets/xyzplaintextedit.cpp \
    utils/hexconvertor.cpp \
    config/frequencyselectwidget.cpp \
    editor/canvassizeform.cpp \
    utils/fftcalc.cpp \
    utils/jp2io.cpp \
    utils/color.cpp \
    widgets/testpatternselection.cpp \
    utils/filewatcher.cpp \
    utils/ftpthread.cpp \
    utils/ftpfunctions.cpp \
    editor/basegraphicitem.cpp \
    editor/templateviewer.cpp


HEADERS  += mainwindow.h \
    config/baseconfig.h \
    config/configparams.h \
    config/soundconfig.h \
    drmtx/common/CDrmTransmitter.h \
    drmtx/common/DRMSignalIO.h \
    utils/dirdialog.h \
    utils/logging.h \
    utils/loggingparams.h \
    appglobal.h \
    utils/supportfunctions.h \
    utils/vector.h \
    utils/buffermanag.h \
    sound/wavio.h \
    config/directoriesconfig.h \
    appdefs.h \
    config/configdialog.h \
    sound/soundbase.h \
    sound/soundpulse.h \
    widgets/spectrumwidget.h \
    widgets/vumeter.h \
    widgets/fftdisplay.h \
    widgets/markerwidget.h \
    dsp/downsamplefilter.h \
    dsp/nco.h \
    utils/macroexpansion.h \
    utils/arraydumper.h \
    sound/soundalsa.h \
    sound/calibration.h \
    dsp/synthes.h \
    dsp/filterparam.h \
    sound/waterfalltext.h \
    utils/macroexpansion.h \
    widgets/imageviewer.h \
    editor/editor.h \
    editor/editorscene.h \
    editor/editorview.h \
    editor/gradientdialog.h \
    editor/graphicitems.h \
    widgets/textdisplay.h \
    dispatch/dispatcher.h \
    dispatch/dispatchevents.h \
    drmrx/demodulator.h \
    drmrx/drm.h \
    drmrx/drmconstellationframe.h \
    drmrx/drmdefs.h \
    drmrx/drmproto.h \
    drmrx/drmpsdframe.h \
    drmrx/drmrx.h \
    drmrx/drmstatusframe.h \
    drmrx/fixform.h \
    drmrx/mkmap.h \
    drmrx/msd_hard.h \
    drmrx/msd_hard_sdc.h \
    drmrx/nrutil.h \
    drmrx/resamplefilter.h \
    drmrx/sourcedecoder.h \
    drmrx/structtemplates.h \
    drmrx/viterbi_decode.h \
    drmtx/common/datadecoding/DABMOT.h \
    drmtx/common/datadecoding/DataDecoder.h \
    drmtx/common/datadecoding/MOTSlideShow.h \
    drmtx/common/FAC/FAC.h \
    drmtx/common/interleaver/BlockInterleaver.h \
    drmtx/common/interleaver/SymbolInterleaver.h \
    drmtx/common/matlib/Matlib.h \
    drmtx/common/matlib/MatlibSigProToolbox.h \
    drmtx/common/matlib/MatlibStdToolbox.h \
    drmtx/common/mlc/BitInterleaver.h \
    drmtx/common/mlc/ChannelCode.h \
    drmtx/common/mlc/ConvEncoder.h \
    drmtx/common/mlc/EnergyDispersal.h \
    drmtx/common/mlc/MLC.h \
    drmtx/common/mlc/QAMMapping.h \
    drmtx/common/ofdmcellmapping/CellMappingTable.h \
    drmtx/common/ofdmcellmapping/OFDMCellMapping.h \
    drmtx/common/SDC/SDC.h \
    drmtx/common/sourcedecoders/AudioSourceDecoder.h \
    drmtx/common/tables/TableAMSS.h \
    drmtx/common/tables/TableCarMap.h \
    drmtx/common/tables/TableCarrier.h \
    drmtx/common/tables/TableDRMGlobal.h \
    drmtx/common/tables/TableFAC.h \
    drmtx/common/tables/TableMLC.h \
    drmtx/common/tables/TableQAMMapping.h \
    drmtx/common/util/Buffer.h \
    drmtx/common/util/CRC.h \
    drmtx/common/util/Modul.h \
    drmtx/common/util/Utilities.h \
    drmtx/common/csoundout.h \
    drmtx/common/DataIO.h \
    drmtx/common/GlobalDefinitions.h \
    drmtx/common/OFDM.h \
    drmtx/common/Parameter.h \
    drmtx/common/soundinterface.h \
    drmtx/bsrform.h \
    drmtx/config.h \
    drmtx/drmparams.h \
    drmtx/drmtransmitter.h \
    drmtx/drmtx.h \
    logbook/logbook.h \
    mainwidgets/gallerywidget.h \
    mainwidgets/rxfunctions.h \
    mainwidgets/rxwidget.h \
    mainwidgets/txfunctions.h \
    mainwidgets/txwidget.h \
    rig/freqdisplay.h \
    rig/rigcontrol.h \
    sstv/modes/modeavt.h \
    sstv/modes/modebase.h \
    sstv/modes/modebw.h \
    sstv/modes/modegbr.h \
    sstv/modes/modegbr2.h \
    sstv/modes/modepd.h \
    sstv/modes/modergb.h \
    sstv/modes/moderobot1.h \
    sstv/modes/moderobot2.h \
    sstv/modes/modes.h \
    sstv/cw.h \
    sstv/sstvparam.h \
    sstv/sstvrx.h \
    sstv/sstvtx.h \
    sstv/syncprocessor.h \
    utils/qurlinfo.h \
    utils/reedsolomoncoder.h \
    utils/rs.h \
    xmlrpc/ipcmessage.h \
    xmlrpc/maiaFault.h \
    xmlrpc/maiaObject.h \
    xmlrpc/maiaXmlRpcClient.h \
    xmlrpc/maiaXmlRpcServer.h \
    xmlrpc/maiaXmlRpcServerConnection.h \
    xmlrpc/xmlinterface.h \
    config/operatorconfig.h \
    widgets/extviewer.h \
    config/guiconfig.h \
    utils/qftp.h \
    sound/resamplefilter.h \
    widgets/imagematrix.h \
    widgets/waterfallform.h \
    config/rigconfig.h \
    config/cwconfig.h \
    config/drmprofileconfig.h \
    config/ftpconfig.h \
    config/hybridconfig.h \
    config/repeaterconfig.h \
    config/waterfallconfig.h \
    utils/hybridcrypt.h \
    videocapt/cameradialog.h \
    videocapt/imagesettings.h \
    videocapt/v4l2control.h \
    videocapt/videocapture.h \
    sstv/visfskid.h \
    dsp/filters.h \
    dsp/filter.h \
    widgets/drmsegmentsview.h \
    widgets/xyzplaintextedit.h \
    utils/hexconvertor.h \
    config/frequencyselectwidget.h \
    editor/canvassizeform.h \
    utils/fftcalc.h \
    utils/jp2io.h \
    utils/color.h \
    widgets/testpatternselection.h \
    utils/filewatcher.h \
    utils/ftpthread.h \
    utils/ftpevents.h \
    utils/ftpfunctions.h \
    editor/basegraphicitem.h \
    editor/templateviewer.h

FORMS += mainwindow.ui \
    config/guiconfig.ui \
    config/soundconfig.ui \
    utils/loggingform.ui \
    config/directoriesconfig.ui \
    config/configdialog.ui \
    widgets/spectrumwidget.ui \
    sound/calibration.ui \
    editor/editorform.ui \
    editor/gradientform.ui \
    editor/textform.ui \
    widgets/textdisplay.ui \
    config/operatorconfig.ui \
    drmrx/drmconstellationframe.ui \
    drmrx/drmpsdframe.ui \
    drmrx/drmstatusframe.ui \
    widgets/extviewer.ui \
    widgets/sweepform.ui \
    widgets/waterfallform.ui \
    config/rigconfig.ui \
    config/cwconfig.ui \
    config/drmprofileconfig.ui \
    config/ftpconfig.ui \
    config/hybridconfig.ui \
    config/repeaterconfig.ui \
    config/waterfallconfig.ui \
    drmtx/bsrform.ui \
    mainwidgets/gallerywidget.ui \
    mainwidgets/rxwidget.ui \
    mainwidgets/txwidget.ui \
    widgets/freqform.ui \
    rig/freqdisplay.ui \
    videocapt/cameradialog.ui \
    videocapt/imagesettings.ui \
    widgets/drmsegmentsview.ui \
    drmrx/fixform.ui \
    config/frequencyselectwidget.ui \
    editor/canvassizeform.ui \
    widgets/testpatternselection.ui \
    editor/templateviewer.ui


OTHER_FILES += \
    icons/fileopen.png \
    icons/filesave.png \
    icons/mgc.raw \
    icons/mgc2.raw \
    icons/camera.png \
    icons/colorfill.png \
    icons/colorline.png \
    icons/doubleleft.png \
    icons/doubleright.png \
    icons/doubletone.png \
    icons/edit.png \
    icons/eraser.png \
    icons/fcircle.png \
    icons/filenew.png \
    icons/frect.png \
    icons/gradient.png \
    icons/image.png \
    icons/left.png \
    icons/line.png \
    icons/qsstv.png \
    icons/qsstvsplash.png \
    icons/replay.png \
    icons/start.png \
    icons/stop.png \
    icons/sweep.png \
    icons/text.png \
    icons/tone.png \
    icons/transparency.png \
    icons/whatsthis.png \
    icons/binary.png



contains(QMAKE_HOST.arch, arm.*):{
        message(Compiling for arm processor -raspberry pi-)

    }
  else {
       message(Compiling for x86)
       }



isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
target.path = $$PREFIX/bin

shortcutfiles.files =qsstv.desktop
shortcutfiles.path = $$PREFIX/share/applications/
data.files += icons/qsstv.png
data.path=$$PREFIX/share/icons/hicolor/42x42/apps/

RESOURCES += \
    qsstv.qrc


INSTALLS += target

LIBS +=  -lasound \
         -lpulse \
         -lpulse-simple \
         -lfftw3f \
         -lfftw3 \
         -lhamlib \
         -lv4l2 \
         -lv4lconvert \
         -lrt


if(CONFIG(debug,debug|release)|CONFIG(enablescope)){
message(including scope)
greaterThan(QT_MAJOR_VERSION,5){
message(QT6)
INCLUDEPATH += /usr/local/qwt-qt6/include
LIBS += -L/usr/local/qwt-qt6/lib -lqwt
}
else{
message(QT5)
INCLUDEPATH += /usr/local/qwt-qt5/include
LIBS += -L/usr/local/qwt-qt5/lib -lqwt
}



DEFINES +=ENABLESCOPE

SOURCES +=      scope/scopeoffset.cpp \
                scope/scopeview.cpp \
                scope/scopeplot.cpp
HEADERS  += scope/scopeoffset.h \
                scope/scopeview.h \
                scope/scopeplot.h
FORMS   += scope/scopeoffset.ui \
                scope/plotform.ui
}

