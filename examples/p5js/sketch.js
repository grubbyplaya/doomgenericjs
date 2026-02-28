function setup() {
  createCanvas(640, 400, WEBGL);
  pixelDensity(0.5);
  framebuffer = createFramebuffer();
}

const keyTable = new Array(256);

function draw() {
  image(framebuffer, -320, -200);
  for (let i = 0; i < 256; i++) {
    if (keyTable[i] && !keyIsDown(i)) {
      keyQueue.push([_toDoomKey(i), 0]);
      keyTable[i] = 0;
    }
  }
}

var keyQueue = [];
var framebuffer;

function DGJS_DrawFrame(framePtr, frameWidth, frameHeight) {
  let doomFrame = Module.HEAPU8.subarray(framePtr, framePtr + frameWidth*frameHeight*4);
  framebuffer.loadPixels();
  for (let i = 0; i < doomFrame.length; i += 4) {
    framebuffer.pixels[i] = doomFrame[i + 2];
    framebuffer.pixels[i + 1] = doomFrame[i + 1];
    framebuffer.pixels[i + 2] = doomFrame[i];
    framebuffer.pixels[i + 3] = 255;
  }
  framebuffer.updatePixels();
}

function DGJS_GetKey() {
  var keys = keyQueue.pop();
  return keys ? keys : [0, 0];
}

function keyPressed() {
  keyQueue.push([_toDoomKey(keyCode), 1]);
  keyTable[keyCode] = 1;
}

function keyReleased() {
  keyQueue.push([_toDoomKey(keyCode), 0]);
  keyTable[keyCode] = 0;
}

function DGJS_SetTitle(ptr, len) {
  
}

function openWad(event) {
    const file = event.target.files[0];
    if (file && file.size) {
        console.log(file.name);
        const reader = new FileReader();
        reader.onload = function(event) {
            usePCM = document.getElementById("soundOptions").checked;
            document.getElementById('wadSelector').remove();
            document.getElementById('soundOptions').remove();
            document.getElementById('soundLabel').remove();

            const uint8File = new Uint8Array(event.target.result);
            let stream = FS.open("/" + (file.name).toLowerCase(), "w+");
            FS.write(stream, uint8File, 0, file.size, 0);
            FS.close(stream);

            Module.ccall(
                'doomgeneric_Create',
                'void',
                ['number', 'number'],
                [0, 0]
            );

            wadLoaded=true;
            doomInterval = setInterval(_doomgeneric_Tick, 0);
        };
        reader.readAsArrayBuffer(file);
    }
}
// MIDi stuff
var songs = [];
var DGJS_MusicType = false;

function DGJS_InitMusic() {
    return true;
}

function DGJS_RegisterSong(song, len) {
    return song;
}

function DGJS_UnRegisterSong(songId) {}
function DGJS_PlaySong(songId, looping) {}
function DGJS_StopSong() {}
function DGJS_PauseSong() {}
function DGJS_ResumeSong() {}
function DGJS_SetMusicVolume(volume) {}
function DGJS_PollMusic() {}

// PCM stuff
var sounds = [];
const channels = new Array(16);

var usePCM = false;
var lastLoadedSound, currentSound, soundIndex, buzzerInterval;
const doomFreqs = [
  175.00, 180.02, 185.01, 190.02, 196.02, 202.02, 208.01, 214.02, 220.02, 226.02, 233.04, 240.02, 247.03, 254.03, 262.00, 269.03, 277.03, 285.04, 294.03, 302.07, 311.04, 320.05, 330.06, 339.06, 349.08, 359.06, 370.09, 381.08, 392.10, 403.10, 415.01, 427.05, 440.12, 453.16, 466.08, 480.15, 494.07, 508.16, 523.09, 539.16, 554.19, 571.17, 587.19, 604.14, 622.09, 640.11, 659.21, 679.10, 698.17, 719.21, 740.18, 762.41, 784.47, 807.29, 831.48, 855.32, 880.57, 906.67, 932.17, 960.69, 988.55, 1017.20, 1046.64, 1077.85, 1109.93, 1141.79, 1175.54, 1210.12, 1244.19, 1281.61, 1318.43, 1357.42, 1397.16, 1439.30, 1480.37, 1523.85, 1569.97, 1614.58, 1661.81, 1711.87, 1762.45, 1813.34, 1864.34, 1921.38, 1975.46, 2036.14, 2093.29, 2157.64, 2217.80, 2285.78, 2353.41, 2420.24, 2490.98, 2565.97, 2639.77
];

function DGJS_InitSound() {
  for (let i = 0; i < channels.length; i++) {
    channels[i] = usePCM ? loadSound("") : new p5.Oscillator('square'); 
  }
  return true;
}

function DGJS_ShutdownSound() {}

function DGJS_UpdateSound() {}

function DGJS_UpdateSoundParams(channel, volume, pan) {
    if (!usePCM)
        return;

    channels[channel].setVolume(volume / 127);
    channels[channel].pan((pan - 128) / 127);
}

function stepBuzzerSound() {
  if (!usePCM && currentSound && soundIndex) {
    channels[0].freq(doomFreqs[currentSound[soundIndex++]], 0);
    if (soundIndex > currentSound.length) {
      clearInterval(buzzerInterval);
      channels[0].isPlaying = false;
      channels[0].stop();
    }
  }
}

function DGJS_StartSound(sfxlumpnum, channel, volume, pan) {
  if (usePCM) {
    channels[channel].setBuffer([sounds[sfxlumpnum][0]]);
    channels[channel].setVolume(volume / 127);
    channels[channel].pan((pan - 128) / 127);
    channels[channel].rate(sounds[sfxlumpnum][2] / channels[channel].sampleRate());
    channels[channel].play();
    return channel;
  } else {
    currentSound = sounds[sfxlumpnum][1];
    soundIndex = 4;
    channels[0].freq(doomFreqs[currentSound[soundIndex++]], 0);
    channels[0].amp(1, 0);
    channels[0].start();
    buzzerInterval = setInterval(stepBuzzerSound, 1000/140);
  }
}

function DGJS_StopSound(channel) {
  if (!usePCM)
    channel = 0;

  channels[channel].stop();
}

function DGJS_SoundIsPlaying(channel) {
    if (!usePCM)
        return channels[0].isPlaying;

    if (channels[channel])
        return channels[channel].isPlaying();
    return false;
}

function DGJS_CacheSFX_PCM(dataPtr, len, sfxlumpnum) {
    const rawSound = Module.HEAPU8.subarray(dataPtr, dataPtr + len);
    const sampleRate = rawSound[3]<<8 | rawSound[2];

    const floatSound = new Array(rawSound.length - 8);

    for (var i = 0x08; i < rawSound.length; i++) {
        floatSound[i - 8] = (rawSound[i] - 0x80) / 127;
    }
    
    sounds[sfxlumpnum] = [floatSound, [], sampleRate];
    lastLoadedSound = sfxlumpnum;
}

function DGJS_CacheSFX_Buzzer(dataPtr, len, sfxlumpnum) {
    const rawSound = Module.HEAPU8.subarray(dataPtr, dataPtr + len);
    sounds[lastLoadedSound][1] = rawSound;
}