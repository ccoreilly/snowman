let source;
let audioProcessor;

async function init() {
  const resultsContainer = document.getElementById("detection-result");

  resultsContainer.textContent = "Loading...";

  const sampleRate = 16000;
  const mediaStream = await navigator.mediaDevices.getUserMedia({
    video: false,
    audio: {
      echoCancellation: true,
      noiseSuppression: true,
      channelCount: 1,
      sampleRate,
      sampleSize: 16,
    },
  });

  const audioContext = new AudioContext({ sampleRate });
  await audioContext.audioWorklet.addModule("audio-processor.js");
  audioProcessor = new AudioWorkletNode(audioContext, "audio-processor", {
    channelCount: 1,
    numberOfInputs: 1,
    numberOfOutputs: 1,
  });

  const worker = new Worker("http://localhost:5000/worker.js");
  worker.onmessage = (ev) => {
    const message = ev.data;
    switch (message.action) {
      case "shareBuffers":
        audioProcessor.port.postMessage(message);
        return;
      case "load":
        console.log(`Snowboy loaded: ${message.result}`);
        if (message.result) {
          worker.postMessage({ action: "init" });
        }
        return;
      case "result":
        resultsContainer.textContent = message.result;
    }
  };

  source = audioContext.createMediaStreamSource(mediaStream);

  resultsContainer.textContent = "Ready";
}

window.onload = () => {
  const trigger = document.getElementById("trigger");
  const stopper = document.getElementById("stopper");

  trigger.onmouseup = () => {
    trigger.disabled = true;
    stopper.disabled = false;
    if (!source) {
      init().then(() => source.connect(audioProcessor));
    } else {
      source.connect(audioProcessor);
    }
  };

  stopper.onmouseup = () => {
    trigger.disabled = false;
    stopper.disabled = true;
    source.disconnect(audioProcessor);
  };
};
