class Snowplough {
  lastResult = 0;
  sharedBuffers = {
    audio: new SharedArrayBuffer(4096),
    state: new SharedArrayBuffer(Int32Array.BYTES_PER_ELEMENT * 1),
  };
  audioSharedArray = new Float32Array(this.sharedBuffers.audio);
  stateSharedArray = new Int32Array(this.sharedBuffers.state);

  constructor() {
    importScripts("https://ccoreilly.github.io/snowman/snowboy_wasm.js");
    self.addEventListener("message", (event) => this.handleMessage(event));
    this.load("common.resources", "snowboy.umdl");
  }
  async load(resUrl, modelUrl) {
    const storagePath = "/snowman";
    const resPath = storagePath + "/" + resUrl.replace(/[\W]/g, "_");
    const modelPath = storagePath + "/" + modelUrl.replace(/[\W]/g, "_");
    return new Promise((resolve, reject) =>
      LoadSnowman().then((loaded) => {
        this.Snowman = loaded;
        resolve(true);
      })
    )
      .then(() => {
        console.debug("Setting up persistent storage at " + storagePath);
        this.Snowman.FS.mkdir(storagePath);
        this.Snowman.FS.mount(this.Snowman.IDBFS, {}, storagePath);
        return this.Snowman.syncFilesystem(true);
      })
      .then(() => {
        const fullModelUrl = new URL(
          modelUrl,
          location.href.replace(/^blob:/, "")
        );
        console.debug(`Downloading ${fullModelUrl} to ${modelPath}`);
        return this.Snowman.download(fullModelUrl, modelPath);
      })
      .then(() => {
        const fullResUrl = new URL(resUrl, location.href.replace(/^blob:/, ""));
        console.debug(`Downloading ${fullResUrl} to ${resPath}`);
        return this.Snowman.download(fullResUrl, resPath);
      })
      .then(() => {
        console.debug(`Syncing filesystem`);

        return this.Snowman.syncFilesystem(false);
      })
      .then(() => {
        console.debug(`Creating detector`);
        this.detector = new this.Snowman.SnowboyDetect(resPath, modelPath);
        console.log(
          `Detector created! sampleRate: ${this.detector.SampleRate()} numChannels: ${this.detector.NumChannels()} bitsPerSample: ${this.detector.BitsPerSample()}`
        );
        console.log(
          `Detector created! sensitivity: ${this.detector.GetSensitivity()} numHotwords: ${this.detector.NumHotwords()}`
        );
        this.detector.SetAudioGain(5);
        this.detector.ApplyFrontend(false);
        // this.vad = new this.Snowman.SnowboyVad(resPath);
      })
      .then(() => {
        postMessage({ action: "load", result: true });
        return true;
      });
  }

  handleMessage(event) {
    const message = event.data;
    if (message.action === "init") {
      postMessage({
        action: "shareBuffers",
        sharedBuffers: this.sharedBuffers,
      });
      this.waitForSharedArrayData();
    }
  }

  _allocateBuffer(size) {
    if (this._bufferAddr !== null && this._bufferSize === size) {
      return;
    }
    this._freeBuffer();
    this._bufferAddr = this.Snowman._malloc(size);
    console.debug(
      `DetectionWorker: allocated buffer with address ${this._bufferAddr}`
    );
    this._bufferSize = size;
    console.debug(
      `DetectionWorker: allocated buffer of ${this._bufferSize} bytes`
    );
  }
  _freeBuffer() {
    if (this._bufferAddr === null) {
      return;
    }
    this.Snowman._free(this._bufferAddr);
    console.debug(`DetectionWorker: freed buffer of ${this._bufferSize} bytes`);
    this._bufferAddr = null;
    this._bufferSize = null;
  }
  processAudioChunk(data) {
    const requiredSize = data.length * data.BYTES_PER_ELEMENT;
    this._allocateBuffer(requiredSize);
    this.Snowman.HEAPF32.set(data, this._bufferAddr / data.BYTES_PER_ELEMENT);

    const result = this.detector.RunDetection(
      this._bufferAddr,
      data.length,
      false
    );
    // const result = this.vad.RunVad(this._bufferAddr, data.length, false);

    return { action: "result", result };
  }
  waitForSharedArrayData() {
    while (Atomics.wait(this.stateSharedArray, 0, 0) === "ok") {
      const data = this.audioSharedArray;
      const result = this.processAudioChunk(data);

      if (this.lastResult !== result.result) {
        postMessage(result);
      }
      this.lastResult = result;
      Atomics.store(this.stateSharedArray, 0, 0);
    }
  }
}

const plough = new Snowplough();
