#ifdef JACK
#include <jack/jack.h>

#include "pineapple.h"

jack_nframes_t sr; // The current sample rate
jack_port_t *output_port;
typedef jack_default_audio_sample_t sample_t;

void j_error(const char *desc);
int j_process(jack_nframes_t nframes, void *arg);
int j_srate(jack_nframes_t nframes, void *arg);
void j_shutdown(void *arg);

/* initialize JACK audio */
u8 j_init(void){
	jack_client_t *client;
	const intptr_t **ports;

	fprintf(stderr, "Trying jack....\n");

	// tell the JACK server to call error() whenever it
	//experiences an error.  Notice that this callback is
	// global to this process, not specific to each client.
	// 
	// This is set here so that it can catch errors in the
	// connection process
	jack_set_error_function(j_error);

	// try to become a client of the JACK server

	if((client = jack_client_new("pineappletracker")) == 0){
		fprintf(stderr, "jack server not running?\n");
		return 1;
	}

	// tell the JACK server to call `process()' whenever
	// there is work to be done.

	jack_set_process_callback(client, j_process, 0);

	// tell the JACK server to call `jack_shutdown()' if
	// it ever shuts down, either entirely, or if it
	// just decides to stop calling us.

	jack_on_shutdown(client, j_shutdown, 0);

	// display the current sample rate. once the client is activated 
	// (see below), you should rely on your own sample rate
	// callback (see above) for this value.
	fprintf(stderr, "engine sample rate: %d\n", jack_get_sample_rate (client));

	sr=jack_get_sample_rate(client);

	output_port = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	
	// tell the JACK server that we are ready to roll

	if(jack_activate(client)){
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	// connect the ports
	if((ports = jack_get_ports(client, NULL, NULL,
					JackPortIsPhysical|JackPortIsInput)) == NULL){
		fprintf(stderr, "Cannot find any physical playback ports\n");
		return 1;
	}

	int i=0;
	while(ports[i]!=NULL){
		if(jack_connect(client, jack_port_name (output_port), ports[i]))
			fprintf(stderr, "cannot connect output ports\n");
		i++;
	}

	return 0;
}

int j_process(jack_nframes_t nframes, void *arg){
	// grab our output buffer
	sample_t *out = (sample_t *) jack_port_get_buffer(output_port, nframes);

	// For each required sample
	for(jack_nframes_t i=0; i<nframes; i++){
		out[i] = (sample_t) interrupthandler();
	}
	return 0;
}

void j_error(const char *desc){
	fprintf(stderr, "JACK error: %s\n", desc);
}

void j_shutdown(void *arg){
	exit(1);
}
#endif // JACK
