#ifndef SAMPLEDCLIENT_H
#define SAMPLEDCLIENT_H

#include "jackclient.h"
#include "audioprocessor.h"
#include <QVector>

/**
  This class simplifies the creation of Jack clients which have a fixed number
  of audio inputs and outputs.

  There are two possible uses for this class:

  1.) Direct use (without subclassing) with a given AudioProcessor instance.
      The AudioProcessor object carries information about the number of inputs
      and outputs and also implements the actual audio processing.
      For this you should use the public constructor.

  2.) Indirect use by subclassing and reimplementing one of the processAudio()
      methods. For this you should use the protected constructor.
  */

class AudioProcessorClient : public JackClient
{
public:
    /**
      Use this constructor if you want to use this class directly with an
      existing AudioProcessor instance.

      @param clientName the name of the Jack client (see JackClient::JackClient)
      @param audioProcessor an instance of the AudioProcessor class which does
        the actual audio processing. This must be a pointer to a valid object.
      */
    AudioProcessorClient(const QString &clientName, AudioProcessor *audioProcessor);
    /**
      The destructor calls close() to close the Jack client. This is done because
      destroying a running Jack client is not safe (as the Jack process thread is
      still running). It is safe to call close() also in a derived class' destructor
      (the client will be closed only once, at the first call to close()).
      */
    virtual ~AudioProcessorClient();

    /**
      @return the AudioProcessor instance doing the actual processing, or 0
        if it has no AudioProcessor (i.e., a subclass does the
        processing by reimplementing the necessary methods).
      */
    AudioProcessor * getAudioProcessor();

protected:
    /**
      Constructor for subclasses that do not want to use a AudioProcessor,
      but reimplement the respective methods such as to do the AudioProcessor's
      work themselves.

      Methods to reimplement:
      - processAudio(const double*, double*, jack_nframes_t) OR
      - processAudio(jack_nframes_t, jack_nframes_t)

      @param clientName the name of the Jack client (see JackClient::JackClient)
      @param inputPortNames the desired audio input port names, at the same time determining
        the input port count
      @param outputPortnames the desired audio output port names, at the same time determining
        the output port count
      */
    AudioProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames);

    /**
      This implementation creates the Jack input and output ports as specified
      directly in the constructor or indirectly by the AudioProcessor instance given
      in the constructor.

      It also sets the sample rate of the contained AudioProcessor (if there is any)
      to the Jack sample rate.

      @return true, if all ports could be created, false otherwise
      */
    virtual bool init();
    /**
      This is called regularly when the client is running. It runs in a separate
      thread, the Jack process thread. This implementation gets the audio
      buffers and then calls processAudio(jack_nframes_t, jack_nframes_t).

      If you need to reimplement this method, you'll have to call getPortBuffers(jack_nframes_t),
      otherwise calling one of the processAudio() methods will fail.

      Typically you don't need to reimplement this method, but reimplement
      one of the processAudio() methods instead.

      You must not make any non real-times (e.g. blocking) calls in this method.

      @param nframes the audio buffer size in frames as given by the Jack server
      @param currently this implementation always returns true
      */
    virtual bool process(jack_nframes_t nframes);
    /**
      Processes a given range of audio frames.
      This implementation just calls processAudio(const double*, double*) for
      every audio frame in the given range.

      Reimplementing this method is currently not advised, because the necessary
      input and output ports are not accessible in subclasses yet.

      @param start the index of the first frame in the audio buffer which should be processed
      @param end the index after the last frame in the audio buffer which should be processed
      */
    virtual void processAudio(jack_nframes_t start, jack_nframes_t end);
    /**
      Process a single audio frame. This implementation just calls the corresponding
      method on the contained AudioProcessor instance. If you used the protected
      constructor without an AudioProcessor instance you have to reimplement this
      method.

      Read the input audio samples from the given input array and write the processing
      results to the given output array.

      @param inputs an array of input audio samples. The size of the array corresponds
        to the number of audio inputs
      @param outputs an array of output audio samples. The size of the array corresponds
        to the number of audio outputs
      @param time the Jack time of the frame to be processed. The absolute value has no
        meaning, however the values will increase monotonically and (if no xruns occur)
        increase only by one between successive calls
      */
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    /**
      If you override process(jack_nframes_t nframes) call this function
      before you call one of the processAudio() methods.

      @param nframes the audio buffer size as given by the Jack server in the
        call to process(jack_nframes_t nframes)
      */
    void getPortBuffers(jack_nframes_t nframes);

private:
    AudioProcessor *audioProcessor;
    QStringList inputPortNames, outputPortNames;
    QVector<jack_port_t*> inputPorts, outputPorts;
    QVector<jack_default_audio_sample_t*> inputBuffers, outputBuffers;
    QVector<double> inputs, outputs;
};

#endif // SAMPLEDCLIENT_H
