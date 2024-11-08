#include "soundbase.h"
#include "logging.h"
#include "configparams.h"
#include "arraydumper.h"

#include <QAudioInput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QDebug>
#include <QApplication>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

const QString captureStateStr[soundBase::CPEND+1] = {
    "Capture Init",
    "Capture Starting",
    "Capture Running",
    "Capture Calibrate Starting",
    "Capture Calibrate Wait",
    "Capture Calibrate",
    "Capture End"
};

const QString playbackStateStr[soundBase::PBEND+1] = {
    "Playback Init",
    "Playback Starting",
    "Playback Running",
    "Playback Calibrate 1",
    "Playback Calibrate 2",
    "Playback End"
};

soundBase::soundBase(QObject *parent) : QThread(parent) {
    captureState = CPINIT;
    playbackState = PBINIT;
    downsampleFilterPtr = new downsampleFilter(DOWNSAMPLE_SIZE, true);
}

soundBase::~soundBase() {
    delete downsampleFilterPtr;
    stopSoundThread();
}

void soundBase::run() {
    stopThread = false;
    while (!stopThread) {
        if ((captureState == CPINIT) && (playbackState == PBINIT)) {
            msleep(100);
            continue;
        }

        if (!soundDriverOK) {
            qDebug() << "Sound driver not OK, resetting sound";
            stopSoundThread();  // Reset sound if driver state is not OK
            msleep(100);
            continue;
        }

        switch (captureState) {
            case CPINIT:
                break;
            case CPSTARTING:
                startCapture();
                switchCaptureState(CPRUNNING);
                break;
            case CPRUNNING:
                // Capture handled in processAudioInput via QAudioInput signals
                msleep(10); // Slight delay to prevent tight looping
                break;
            case CPEND:
                switchCaptureState(CPINIT);
                break;
            default:
                break;
        }

        switch (playbackState) {
            case PBINIT:
                break;
            case PBSTARTING:
                preparePlayback();
                flushPlayback();
                prebuf = true;
                switchPlaybackState(PBRUNNING);
                addToLog("playback started", LOGSOUND);
                break;
            case PBRUNNING:
                if (play() == 0) {
                    addToLog("playback stopped", LOGSOUND);
                    waitPlaybackEnd();
                    switchPlaybackState(PBINIT);
                }
                msleep(10);
                break;
            case PBEND:
                switchPlaybackState(PBINIT);
                break;
            default:
                break;
        }
    }
}

bool soundBase::startCapture() {
    QAudioFormat format;
    format.setSampleRate(48000);  // Adjust sample rate as needed
    format.setChannelCount(2);    // Stereo capture, or 1 for mono
    format.setSampleSize(16);     // 16-bit audio samples
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultInputDevice();
    if (!deviceInfo.isFormatSupported(format)) {
        qWarning() << "Default format not supported - using nearest.";
        format = deviceInfo.nearestFormat(format);
    }

    audioInput = new QAudioInput(deviceInfo, format, this);
    audioDevice = audioInput->start();
    if (!audioDevice) {
        qWarning() << "Failed to start audio capture device";
        return false;
    }

    connect(audioDevice, &QIODevice::readyRead, this, &soundBase::processAudioInput);
    qDebug() << "Audio capture started using QAudioInput";
    return true;
}

void soundBase::processAudioInput() {
    if (!audioDevice) return;

    QByteArray audioData = audioDevice->readAll();
    const qint16* samples = reinterpret_cast<const qint16*>(audioData.constData());
    int sampleCount = audioData.size() / sizeof(qint16);

    downsampleFilterPtr->downSample4(samples, sampleCount);  // Adjust for your processing code
    volume = downsampleFilterPtr->avgVolumeDb;
    rxBuffer.putNoCheck(downsampleFilterPtr->filteredDataPtr(), RX_BUFFER_SIZE);
    rxVolumeBuffer.putNoCheck(downsampleFilterPtr->getVolumePtr(), RX_BUFFER_SIZE);
}

void soundBase::stopSoundThread() {
    if (audioInput) {
        audioInput->stop();
        delete audioInput;
        audioInput = nullptr;
        audioDevice = nullptr;
        qDebug() << "QAudioInput stopped and cleaned up.";
    }
    stopThread = true;
}

void soundBase::idleTX() {
    playbackState = PBINIT;
}

void soundBase::idleRX() {
    captureState = CPINIT;
}

void soundBase::switchCaptureState(ecaptureState cs) {
    addToLog(QString("Switching from captureState %1 to %2")
             .arg(captureStateStr[captureState])
             .arg(captureStateStr[cs]), LOGSOUND);
    captureState = cs;
}

void soundBase::switchPlaybackState(eplaybackState ps) {
    addToLog(QString("Switching from playbackState %1 to %2")
             .arg(playbackStateStr[playbackState])
             .arg(playbackStateStr[ps]), LOGSOUND);
    playbackState = ps;
}

int soundBase::play() {
    unsigned int numFrames;
    int framesWritten;

    if (prebuf && txBuffer.count() < (DOWNSAMPLE_SIZE * 8)) {
        return 0;
    }
    if ((numFrames = txBuffer.count()) >= DOWNSAMPLE_SIZE) {
        numFrames = DOWNSAMPLE_SIZE;
    }
    if (numFrames > 0) {
        framesWritten = write(numFrames);
    }
    txBuffer.copyNoCheck(tempTXBuffer, numFrames);
    addToLog(QString("frames written: %1").arg(framesWritten), LOGSOUND);

    return numFrames;
}
