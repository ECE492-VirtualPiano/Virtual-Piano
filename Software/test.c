#include "piano.h"

/*
	Name: 			void pitchshiftTest()
	
	Description: 	Output a transformed waveform for testing
*/ 
void pitchshiftTest()
{
	// Print the information of the wav file read
    int16_t *samples = NULL;
    wavread("c_chopped.wav", &samples);
    printf("No. of channels: %d\n",     header->num_channels);
    printf("Sample rate:     %d\n",     header->sample_rate);
    printf("Bit rate:        %dkbps\n", header->byte_rate*8 / 1000);
    printf("Bits per sample: %d\n\n",     header->bps);
    printf("Sample 0:        %d\n", samples[0]);
    printf("Sample 1:        %d\n", samples[1]);
    printf("Sample 2:        %d\n", samples[2]);

    // Create the Sample object to hold the waveform
    Sample *samples_c = (Sample*)malloc(sizeof(Sample));
    samples_c->data = samples;
    samples_c->size = header->datachunk_size / sizeof(int16_t);

    // Output Sample waveform
    Sample *samples_t = NULL;

    // Shift pitches to get different sounds
    pitchshift(&samples_c, &samples_t, 1);

    // Write the result waveform into a wav file
    header->datachunk_size = samples_t->size * sizeof(int16_t);
    wavwrite("../Wavfiles/test_c.wav", samples_t->data, samples_t->size);

    // Free the memories
    free(header);
    free(samples_c->data);
    free(samples_c);
    free(samples_t->data);
    free(samples_t);

}

int main() 
{
	pitchshiftTest();
	return 0;
}