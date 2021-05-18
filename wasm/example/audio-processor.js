class AudioProcessor extends AudioWorkletProcessor {
  audioArrayIndex = 0;
  constructor(options) {
    super(options);
    console.debug(`AudioWorkletProcessor sampleRate ${sampleRate}`);
    this.port.onmessage = this.processMessage.bind(this);
  }

  processMessage(event) {
    if (event.data.action === "shareBuffers") {
      this.audioSharedArray = new Float32Array(event.data.sharedBuffers.audio);
      this.stateSharedArray = new Int32Array(event.data.sharedBuffers.state);
    }
  }

  process(inputs, outputs, parameters) {
    const data = inputs[0][0];
    if (data && this.stateSharedArray) {
      // AudioBuffer samples are represented as floating point numbers between -1.0 and 1.0 whilst
      // a signed int16 is between -32768 and 32767
      const audioArray = data.map((value) => value * 0x8000);

      this.audioSharedArray.set(audioArray, this.audioArrayIndex * 128);
      if (this.audioArrayIndex > 6) {
        Atomics.notify(this.stateSharedArray, 0, 1);
        this.audioArrayIndex = 0;
      } else {
        this.audioArrayIndex++;
      }
    }
    return true;
  }
}

registerProcessor("audio-processor", AudioProcessor);
