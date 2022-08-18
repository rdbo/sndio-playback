#include <stdio.h>
#include <stdlib.h>
#include <sndio.h>

void
print_par_info(struct sio_par *par)
{
	printf("\tBits per Sample: %u\n"
		"\tBytes per Sample: %u\n"
		"\tRecording Channels: %u\n"
		"\tPlayback Channels: %u\n"
		"\tRate: %u\n",
		par->bits, par->bps, par->rchan, par->pchan, par->rate
	);
}

int
main()
{
	struct {
		struct sio_hdl *handle;
		struct sio_par devpardat;
		struct sio_par pardat;
	} idev, odev;
	void	*audiobuf;
	size_t	audiobufsiz;

	idev.handle = sio_open(SIO_DEVANY, SIO_REC, 0);
	if (idev.handle == NULL) {
		perror("Unable to open input device handle");
		return -1;
	}

	odev.handle = sio_open(SIO_DEVANY, SIO_PLAY, 0);
	if (odev.handle == NULL) {
		perror("Unable to open output device handle");
		sio_close(idev.handle);
		return -1;
	}

	sio_getpar(idev.handle, &idev.devpardat);
	printf("Input Device Info:\n");
	print_par_info(&idev.devpardat);
	printf("\n");

	printf("Output Device Info: \n");
	sio_getpar(odev.handle, &odev.devpardat);
	print_par_info(&odev.devpardat);
	printf("\n");

	sio_initpar(&idev.pardat);
	sio_initpar(&odev.pardat);

	idev.pardat.bits = idev.devpardat.bits;
	idev.pardat.sig = idev.devpardat.sig;
	idev.pardat.le = idev.devpardat.le;
	idev.pardat.msb = idev.devpardat.msb;
	idev.pardat.bps = idev.devpardat.bps;
	idev.pardat.rate = 44100;
	idev.pardat.rchan = 2;

	odev.pardat.bits = idev.pardat.bits;
	odev.pardat.sig = idev.pardat.sig;
	odev.pardat.le = idev.pardat.le;
	odev.pardat.msb = idev.pardat.msb;
	odev.pardat.bps = idev.pardat.bps;
	odev.pardat.rate = idev.pardat.rate;
	odev.pardat.pchan = 2;

	sio_setpar(idev.handle, &idev.pardat);
	sio_setpar(odev.handle, &odev.pardat);

	sio_start(idev.handle);
	sio_start(odev.handle);

	// 1sec / 100 == 10ms interval
	audiobufsiz = idev.pardat.bps * idev.pardat.rchan * idev.pardat.rate / 100;
	audiobuf = malloc(audiobufsiz);
	if (audiobuf) {
		size_t i;

		printf("Recording started...\n");
		for (i = 0; i < 10 * 100; ++i) {
			sio_read(idev.handle, audiobuf, audiobufsiz);
			sio_write(odev.handle, audiobuf, audiobufsiz);
		}
		printf("Recording ended\n");
		free(audiobuf);
	} else {
		perror("Unable to allocate memory for the audio buffer");
		sio_stop(odev.handle);
		sio_stop(idev.handle);
		sio_close(odev.handle);
		sio_close(idev.handle);
		return -1;
	}

	sio_stop(odev.handle);
	sio_stop(idev.handle);
	sio_close(odev.handle);
	sio_close(idev.handle);

	return 0;
}
