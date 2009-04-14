/* vi:set ts=4 sts=4 sw=4: */
#include <stdio.h>

#include <SDL/SDL.h>
#include <jack/jack.h>

#ifndef D_WINDOWS
#include <err.h>
#endif

#include "stuff.h"

/* JACK */
/*jack_port_t *output_port;

typedef jack_default_audio_sample_t sample_t;

// The current sample rate
jack_nframes_t sr;

int process(jack_nframes_t nframes, void *arg){
	// grab our output buffer
	sample_t *out = (sample_t *) jack_port_get_buffer 
			(output_port, nframes);

	// For each required sample
	for(jack_nframes_t i=0; i<nframes; i++){
		out[i] = interrupthandler();

	}
	return 0;
}

int srate(jack_nframes_t nframes, void *arg){
	printf("the sample rate is now %lu/sec\n", nframes);
	sr=nframes;
	return 0;
}

void error(const char *desc){
	fprintf(stderr, "JACK error: %s\n", desc);
}

void jack_shutdown(void *arg){
	exit(1);
}*/

/* SDL */
void audiocb(void *userdata, Uint8 *buf, int len){
	int i;

	for(i = 0; i < len; i++){
		buf[i] = interrupthandler();
	}
}

int main(int argc, char **argv){
	/*
	 * SDL
	 */
	SDL_AudioSpec requested, obtained;

	if(SDL_Init( SDL_INIT_AUDIO ) < 0){
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);

	requested.freq = 16000;
	requested.format = AUDIO_U8;
	requested.samples = 256;
	requested.callback = audiocb;
	requested.channels = 1;

	// comment this out to run on grace
	//if(SDL_OpenAudio(&requested, &obtained) == -1){
	//	err(1, "SDL_OpenAudio");
	//}

    // Actually if we don't do error checking it just works on grace and
	//     // locally with sound :3
	SDL_OpenAudio(&requested, &obtained);

	fprintf(stderr, "freq %d\n", obtained.freq);
	fprintf(stderr, "samples %d\n", obtained.samples);

	if(argc != 2){
		loadfile("untitled.song");
	}else{
		loadfile(argv[1]);
	}

	initchip();
	initgui();

	SDL_PauseAudio(0);

	guiloop();

	return 0;

	/*
	 * JACK
	 */
	/*jack_client_t *client;
	const char **ports;

	if(argc != 2){
		loadfile("untitled.song");
	}else{
		loadfile(argv[1]);
	}

	// tell the JACK server to call error() whenever it
	//experiences an error.  Notice that this callback is
	// global to this process, not specific to each client.
	// 
	// This is set here so that it can catch errors in the
	// connection process
	jack_set_error_function (error);

	// try to become a client of the JACK server

	if ((client = jack_client_new ("pineappletracker")) == 0) {
		fprintf (stderr, "jack server not running?\n");
		return 1;
	}

	// tell the JACK server to call `process()' whenever
	// there is work to be done.

	jack_set_process_callback (client, process, 0);

	// tell the JACK server to call `srate()' whenever
	// the sample rate of the system changes.

	jack_set_sample_rate_callback (client, srate, 0);

	// tell the JACK server to call `jack_shutdown()' if
	// it ever shuts down, either entirely, or if it
	// just decides to stop calling us.

	jack_on_shutdown (client, jack_shutdown, 0);

	// display the current sample rate. once the client is activated 
	// (see below), you should rely on your own sample rate
	// callback (see above) for this value.
	printf ("engine sample rate: %lu\n", jack_get_sample_rate (client));


	sr=jack_get_sample_rate(client);

	output_port = jack_port_register (client, "output", 
					 JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	// tell the JACK server that we are ready to roll

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	// connect the ports
	if ((ports = jack_get_ports (client, NULL, NULL, 
				   JackPortIsPhysical|JackPortIsInput)) == NULL) {
		fprintf(stderr, "Cannot find any physical playback ports\n");
		exit(1);
	}

	int i=0;
	while(ports[i]!=NULL){
		if (jack_connect (client, jack_port_name (output_port), ports[i])) {
			fprintf (stderr, "cannot connect output ports\n");
		}
		i++;
	}

	guiloop();

	//free (ports);
	//jack_client_close (client);
	
	exit (0);*/
}
