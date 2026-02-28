# doomgeneric.js
This is doomgeneric compiled to JavaScript using Emscripten, with bindings added to allow for sound and music support.

To use doomgeneric.js, use a script tag like this:

`<script type='text/javascript' src='https://grubbyplaya.github.io/doomgenericjs/doomgeneric/doomgeneric.js'></script>` 

Using `import` to import the Emscripten module is not supported at this time.

# Porting
## Generic
Implement these functions to get Doom running:
* DGJS_DrawFrame(framePtr, width, height): Draws a Doom frame. framePtr is a pointer to the framebuffer, which you will need to fetch from Module memory. This frame is in BGRA format, where blue is the first byte, followed by green, and so on. Alpha is always 0, so direct copying is not recommended.
* DGJS_SetTitle(title, titleLen): Allows you to fetch the IWAD name as a string. title is a pointer to a C string (charcode array), which you will need to fetch from module memory.
* DGJS_GetKey(): Returns a two-element array, containing a Doom key code at index 0 and its pressed state at index 1. When using JavaScript keycodes, you can use the `_toDoomKey` function provided by the Emscripten module to convert the keycode into what Doom expects.

## Audio
These functions have to be implemented to add sound and music support. Leaving them as stubs is fine unless a return value is expected. Functions with a return value are **bolded.**

### Music
The global variable DGJS_MusicType is used to pass a pointer to music data in either the MUS (false) or MIDI (true) formats.
* **DGJS_InitMusic()**: Initializes the music driver. Returns true by default and false if an error occured.
* **DGJS_RegisterSong(song, len)**: Passes a song to the music driver. It should return an identifier that is used by DGJS_UnRegisterSong and DGJS_PlaySong to identify the current song. `song` is passed as a pointer, so you will need to fetch the sound data from `Module` memory yourself.
* DGJS_UnRegisterSong(songId): Dereferences a song from the music driver.
* DGJS_PlaySong(songId, looping): Plays the registered song. If `looping` is true, the song should loop.
* DGJS_StopSong(): Stops the current song.
* DGJS_PauseSong(): Pauses the current song.
* DGJS_ResumeSong(): Resumes the current song.
* DGJS_SetMusicVolume(volume): Sets the song volume.
* DGJS_PollMusic(): Updates the song driver at a fixed interval.

### Sound effects
* DGJS_InitSound(): Initializes the sound driver.
* DGJS_ShutdownSound(): Shuts down the sound driver.
* DGJS_UpdateSound(): Updates the sound driver at a fixed interval.
* DGJS_UpdateSoundParams(channel, volume, pan): Sets the given sound channel's volume and panning.
* **DGJS_StartSound(soundId, channel, volume, pan)**: Plays the sound effect specified by `soundId` on a given channel, setting the channel's volume and panning. Should return `channel` by default, and -1 if an error occurs.
* DGJS_StopSound(channel): Stops the given sound channel.
* **DGJS_SoundIsPlaying(channel)**: Returns true if the given sound channel is playing, and false otherwise.
* DGJS_CacheSFX_PCM/Buzzer(data, len, soundId): Caches the sound data, which is `len` bytes large, using `soundId` as an identifier. `data` is passed as a pointer, so you will need to fetch the sound data from `Module` memory yourself.

# Examples

## Basic (WebAudio PCM backend, full MIDI support)
![](https://github.com/grubbyplaya/doomgenericjs/blob/main/examples/basic/screenshot.png?raw=true)
[Link](https://grubbyplaya.github.io/doomgenericjs/examples/basic/)

## AudioTag (HTML Audio PCM backend)
![](https://github.com/grubbyplaya/doomgenericjs/blob/main/examples/htmlaudio/screenshot.png?raw=true)
[Link](https://grubbyplaya.github.io/doomgenericjs/examples/htmlaudio/)

## Doompedia (Wikipedia user script, based on Basic)
![](https://github.com/grubbyplaya/doomgenericjs/blob/main/examples/doompedia/screenshot.png?raw=true)
[Link](https://en.wikipedia.org/wiki/User:Grubbyeditor/doompedia.js)

## p5.js (AudioBuffer PCM backend, Oscillator Buzzer backend)
![](https://github.com/grubbyplaya/doomgenericjs/blob/main/examples/p5js/screenshot.png?raw=true)
[Link](https://grubbyplaya.github.io/doomgenericjs/examples/p5js/)
