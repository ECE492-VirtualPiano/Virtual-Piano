#include <stdio.h>
#include <math.h>
#include "wav.h"

void speedx(Sample **samples, Sample **samples_t, double factor)
{
	printf("%lu\n", (int)(header->datachunk_size / (sizeof(int16_t) * factor)) * sizeof(int16_t));

	// Allocate memeory for the transformed samples. The 
	if(*samples_t) 
	{
		free(*samples_t);
	}

	*samples_t = (Sample*)malloc(sizeof(Sample));
	(*samples_t)->data = (int16_t*)malloc((int)(header->datachunk_size / factor));

	int res_i = 0;

	for(double step = 0; step < header->datachunk_size / (factor * sizeof(int16_t)); step += factor)
	{
		int i = (int)round(step);
		(*samples_t)->data[res_i++] = (*samples)->data[i];
	}

	(*samples_t)->size = res_i;
}

void superposition(Sample **samples1, Sample **samples2, Sample **samples_t, int offset)
{
	if(*samples_t)
	{
		free(*samples_t);
	}

	*samples_t = (Sample*)malloc(sizeof(Sample));

	(*samples_t)->size = offset + (((*samples2)->size > (*samples1)->size - offset) ? (*samples2)->size : (*samples1)->size - offset);
	(*samples_t)->data = (int16_t*)malloc((*samples_t)->size * sizeof(int16_t));

	printf("%i, %i, %i\n", (*samples1)->size, (*samples2)->size, (*samples_t)->size);

	for (int i = 0; i < (*samples_t)->size; ++i)
	{
		(*samples_t)->data[i] = 0;

		if (i < (*samples1)->size)
		{
			(*samples_t)->data[i] += (*samples1)->data[i];
		}

		if (i >= offset && i < offset + (*samples2)->size)
		{
			(*samples_t)->data[i] += (*samples2)->data[i - offset];
		}
	}
}

void stretch(Sample **samples, Sample **samples_t, double factor, int window_size, int h)
{
	double *phase = (double*)calloc(window_size * sizeof(double));
	double *hanning_window = (double*)malloc(window_size * sizeof(double));
	for (int i = 0; i < window_size; ++i)
	{
		hanning_window[i] = 0.5 * (1 - cos(2 * PI * i / (window_size - 1)));
	}
	double *result = (double*)calloc((*samples)->size / factor + window_size);

	for (double step = 0; step < (*samples)->size - (window_size + h); step += h * factor)
	{
		int16_t *a1 = (int16_t*)malloc(window_size * sizeof(int16_t));
		int16_t *a2 = (int16_t*)malloc(window_size * sizeof(int16_t));

		for (int j = (int)step; j < (int)step + window_size; ++j)
		{
			a1[j] = (*samples)->data[j];
			a2[j + h] = (*samples)->data[j + h];
		}

				
	}
}

void main()
{
    int16_t *samples = NULL;
    wavread("c_chopped.wav", &samples);
    printf("No. of channels: %d\n",     header->num_channels);
    printf("Sample rate:     %d\n",     header->sample_rate);
    printf("Bit rate:        %dkbps\n", header->byte_rate*8 / 1000);
    printf("Bits per sample: %d\n\n",     header->bps);
    printf("Sample 0:        %d\n", samples[0]);
    printf("Sample 1:        %d\n", samples[1]);
    printf("Sample 2:        %d\n", samples[2]);
    // Modify the header values & samples before writing the new file

    Sample *samples1 = (Sample*)malloc(sizeof(Sample));
    samples1->data = samples;
    samples1->size = header->datachunk_size / sizeof(int16_t);

    Sample *samples2 = NULL, *samples_t = NULL;
    double factor = 2;
    speedx(&samples1, &samples2, factor);
    superposition(&samples1, &samples2, &samples_t, 0);

    wavwrite("track2.wav", samples_t->data, samples_t->size);
    free(header);
    free(samples);
}
