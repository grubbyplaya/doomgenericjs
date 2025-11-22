This is the code of Doompedia, a Wikipedia user script that uses doomgeneric.js to run Doom on any Wikipedia page.

```
$('#bodyContent').prepend('<div><canvas id="doomCanvas" width="320" height="200" tabindex="0" style="border:1px solid #000000;"></canvas><div><input type="file" id="wadSelector" accept=".wad"/>');

var keyQueue = [];
var doomInterval;
var oldTitle;

$('#doomCanvas').on("keydown", function(event) {
	keyQueue.push([_toDoomKey(event.keyCode), 1]);
	
	if (event.keyCode == 115) {
		$('#doomCanvas').remove();
		$('#wadSelector').remove();
		$('.firstHeading').text(oldTitle);
		clearInterval(doomInterval);
	}
});

$('#doomCanvas').on("keyup", function(event) {
	keyQueue.push([_toDoomKey(event.keyCode), 0]);
});

$("#wadSelector").on("change", function(event) {
	const file = event.target.files[0];
	if (file && file.size) {
		console.log(file.name);
    	const reader = new FileReader();
    	reader.onload = function(event) {
			$('#wadSelector').remove();
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
});

const canvasScreen = new ImageData(320, 200);
const canvasCtx = $("#doomCanvas")[0].getContext('2d');
function DGJS_DrawFrame(framePtr, frameWidth, frameHeight) {
	let framebuffer = Module.HEAPU8.subarray(framePtr, framePtr + frameWidth*frameHeight*4);
	for (let i = 0; i < frameWidth*frameHeight*4; i += 4) {
    	canvasScreen.data[i] = framebuffer[i + 2];
    	canvasScreen.data[i + 1] = framebuffer[i + 1];
    	canvasScreen.data[i + 2] = framebuffer[i];
    	canvasScreen.data[i + 3] = 255;
	}
	canvasCtx.putImageData(canvasScreen, 0, 0);
}

function DGJS_SetTitle(titlePtr, titleLen) {
	let titleChar = Module.HEAPU8.subarray(titlePtr, titlePtr + titleLen);
	let newTitle = String.fromCharCode(...titleChar);
	oldTitle = $('.firstHeading').text()
	$('.firstHeading').text(newTitle);
}

function DGJS_GetKey() {
	var key = keyQueue.pop();
	return key ? key : [0, 0];
}

// MIDi stuff
var DGJS_MusicType = true;
var songs = [];
var doomPlayer;

function DGJS_InitMusic() {
    return true;
}

function DGJS_RegisterSong(song, len) {
    const songData = Module.HEAPU8.subarray(song, song + len);
    songs[song] = songData;
    return song;
}

function DGJS_UnRegisterSong(songId) {
    songs.splice(songId);
}

function DGJS_PlaySong(songId, looping) {
    doomPlayer = new MIDIPlayer();
    doomPlayer.onload = function(song) {
        doomPlayer.play();
    }
    doomPlayer.openFile(songs[songId]);
    doomPlayer.autoReplay = looping;
}

function DGJS_StopSong() {
    doomPlayer.stop();
}

function DGJS_PauseSong() {
    doomPlayer.pause();
}

function DGJS_ResumeSong() {
    doomPlayer.play();
}

function DGJS_SetMusicVolume(volume) {}
function DGJS_PollMusic() {}

// PCM stuff
const audioCtx = new window.AudioContext();
var sounds = [];
var channels = new Array(16);

function DGJS_InitSound() {
    for (let i = 0; i < 16; i++) {
        channels[i] = {buffer: null, volume: null, pan: null, playing: false};
    }
    return true;
}

function DGJS_ShutdownSound() {}
function DGJS_UpdateSound() {}

function DGJS_UpdateSoundParams(channel, volume, pan) {
    channels[channel].volume.value = volume / 127;
    channels[channel].pan.value = (pan - 128) / 127;
}

function DGJS_StartSound(sfxlumpnum, channel, volume, pan) {
    var source = audioCtx.createBufferSource();
    var gainNode = audioCtx.createGain();
    var panning = audioCtx.createStereoPanner();
    source.buffer = sounds[sfxlumpnum];
    gainNode.gain.value = volume / 127;
    panning.pan.value = (pan - 128) / 127;
    source.connect(panning);
    panning.connect(gainNode);
    gainNode.connect(audioCtx.destination);
    channels[channel].buffer = source;
    channels[channel].volume = gainNode.gain;
    channels[channel].pan = panning.pan;
    channels[channel].playing = true;
    source.start();
    source.onended = () => {
        channels[channel].playing = false;
    };
    return channel;
}

function DGJS_StopSound(channel) {
    channels[channel].buffer.stop();
}

function DGJS_SoundIsPlaying(channel) {
    if (channels[channel])
        return channels[channel].playing;
    return false;
}

function DGJS_CacheSFX_PCM(dataPtr, len, sfxlumpnum) {
    const rawSound = Module.HEAPU8.subarray(dataPtr, dataPtr + len);
    const sampleRate = rawSound[3]<<8 | rawSound[2];
    const soundBuffer = audioCtx.createBuffer(1, rawSound.length - 8, sampleRate);
    const floatSound = soundBuffer.getChannelData(0);
    for (var i = 0x08; i < rawSound.length; i++) {
        floatSound[i - 8] = (rawSound[i] - 0x80) / 127;
    }
    sounds[sfxlumpnum] = soundBuffer;
}

function DGJS_CacheSFX_Buzzer(dataPtr, size, sfxlumpnum) {}

importScriptURI('https://grubbyplaya.github.io/doomgenericjs/examples/basic/doomgeneric.js');
importScriptURI('https://fraigo.github.io/javascript-midi-player/midiplayer/WebAudioFontPlayer.js');
importScriptURI('https://fraigo.github.io/javascript-midi-player/midiplayer/MIDIFile.js');
importScriptURI('https://fraigo.github.io/javascript-midi-player/midiplayer/MIDIPlayer.js');
```
