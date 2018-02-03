#include <stdio.h>
#include <math.h>
#include "wav.h"
#include "kiss_fft.h"

#define PI 3.1415926535897932384626


void pitchshift(Sample **samples, Sample **samples_t, int n);
void speedx(Sample **samples, Sample **samples_t, float factor);
void superposition(Sample **samples1, Sample **samples2, Sample **samples_t, int offset);
void stretch(Sample **samples, Sample **samples_t, float factor, int window_size, int h);

void pitchshift(Sample **samples, Sample **samples_t, int n)
{
	const int window_size = pow(2, 10);
	const int h = pow(2, 8);

	Sample *stretched = NULL;

	float factor = pow(2.0f, (1.0f * n / 12.0f));
	stretch(samples, &stretched, 1.0f / factor, window_size, h);

	Sample *tempsamples_t = (Sample*)malloc(sizeof(Sample));
	tempsamples_t->size = stretched->size - window_size;
	tempsamples_t->data = (int16_t*)malloc(tempsamples_t->size* sizeof(int16_t));

	for (int i = 0; i < tempsamples_t->size; ++i)
	{
		tempsamples_t->data[i] = stretched->data[i + window_size];
	}

	speedx(&tempsamples_t, samples_t, factor);

	// free(stretched->data);
	free(stretched);
	// free(tempsamples_t->data);
	free(tempsamples_t);
}

void speedx(Sample **samples, Sample **samples_t, float factor)
{
	if (*samples_t) 
	{
		// free((*samples_t)->data);
		free(*samples_t);
	}

	*samples_t = (Sample*)malloc(sizeof(Sample));
	(*samples_t)->size = (*samples)->size / factor;
	(*samples_t)->data = (int16_t*)calloc((*samples_t)->size, sizeof(int16_t));

	int res_i = 0;

	for (float step = 0; step < (*samples)->size; step += factor)
	{
		int i = (int)round(step);
		(*samples_t)->data[res_i++] = (*samples)->data[i];
	}
}

void superposition(Sample **samples1, Sample **samples2, Sample **samples_t, int offset)
{
	if(*samples_t)
	{
		// free((*samples_t)->data);
		free(*samples_t);
	}

	*samples_t = (Sample*)malloc(sizeof(Sample));

	(*samples_t)->size = offset + (((*samples2)->size > (*samples1)->size - offset) ? (*samples2)->size : (*samples1)->size - offset);
	(*samples_t)->data = (int16_t*)calloc((*samples_t)->size, sizeof(int16_t));

	// int16_t max1 = 0, max2 = 0;

	// for (int i = 0; i < (*samples1)->size; ++i)
	// {
	// 	max1 = abs((*samples1)->data[i]) > max1 ? abs((*samples1)->data[i]) : max1;
	// }

	// for (int i = 0; i < (*samples2)->size; ++i)
	// {
	// 	max2 = abs((*samples2)->data[i]) > max1 ? abs((*samples2)->data[i]) : max2;
	// }

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

		// if (i >= offset && i < (*samples1)->size)
		// {
		// 	(*samples_t)->data[i] /= 2;
		// }
	}
}

void stretch(Sample **samples, Sample **samples_t, float factor, int window_size, int h)
{
	if(*samples_t)
	{
		// free((*samples_t)->data);
		free(*samples_t);
	} 
	*samples_t = (Sample*)malloc(sizeof(Sample));
	(*samples_t)->size = (*samples)->size / factor + window_size;
	(*samples_t)->data = (int16_t*)calloc((*samples_t)->size, sizeof(int16_t));

	float *phase = (float*)calloc(window_size, sizeof(float));
	float *hanning_window = (float*)calloc(window_size, sizeof(float));
	for (int i = 0; i < window_size; ++i)
	{
		hanning_window[i] = 0.5 * (1 - cos(2 * PI * i / (window_size - 1)));
	}

	float *result = (float*)calloc((*samples_t)->size, sizeof(float));

    kiss_fft_cfg cfg = kiss_fft_alloc(window_size, 0, NULL, NULL);
    kiss_fft_cfg cfg_i = kiss_fft_alloc(window_size, 1, NULL, NULL);

	for (float step = 0; step < (*samples)->size - (window_size + h); step += h * factor)
	{
		int16_t *a1 = (int16_t*)malloc(window_size * sizeof(int16_t));
		int16_t *a2 = (int16_t*)malloc(window_size * sizeof(int16_t));

		int index = 0;
		for (int j = (int)step; j < (int)step + window_size; ++j)
		{
			a1[index] = (*samples)->data[j];
			a2[index] = (*samples)->data[j + h];
			index++;
		}

	    kiss_fft_cpx *s1_in = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));
	    kiss_fft_cpx *s1_out = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));
	    
	    kiss_fft_cpx *s2_in = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));
	    kiss_fft_cpx *s2_out = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));

	    for (int i = 0; i < window_size; ++i)
	    {
	    	s1_in[i].r = a1[i] * hanning_window[i];
	    	s2_in[i].r = a2[i] * hanning_window[i];
	    }

	    kiss_fft(cfg, s1_in, s1_out);
	 	kiss_fft(cfg, s2_in, s2_out);

	    for (int i = 0; i < window_size; ++i)
	    {
	    	/*
	    	  Complex division s2 / s1

	    	  (s2_r + s2_i * i)   (s2_r + s2_i * i) * (s1_r - s1_i * i)    
	    	  ----------------- = -------------------------------------
	    	  (s1_r + s1_i * i)   (s1_r + s1_i * i) * (s1_r - s1_i * i)      
	    	
			    (s1_r * s2_r + s1_i * s2_i) + (s1_r * s2_i - s1_i * s2_r)i
			  = ----------------------------------------------------------
			                         s1_r^2 + s1_i^2
			*/

	    	float res_i = s2_out[i].i * s1_out[i].r - s2_out[i].r * s1_out[i].i;
	    	float res_r = s2_out[i].r * s1_out[i].r + s2_out[i].i * s1_out[i].i;

			/*
			  Perform arctan2(s2, s1) to get the angle in four quadrants between
			  the two complex numbers. 
			*/	    	
	    	float atan_v = atan2(res_i, res_r);

	    	phase[i] = fmod(phase[i] + atan_v, 2.0f * PI);
	    }

		kiss_fft_cpx *a2_rephased = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));

		kiss_fft_cpx *s2_rephased = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));

		for (int i = 0; i < window_size; ++i) 
		{
			float abs_v = sqrt(pow(s2_out[i].r, 2) + pow(s2_out[i].i, 2));
			s2_rephased[i].r = abs_v * cos(phase[i]);
			s2_rephased[i].i = abs_v * sin(phase[i]); 
		}

		kiss_fft(cfg_i, s2_rephased, a2_rephased);

		int i2 = (int)(step / factor);
		for (int i = 0; i < window_size; ++i)
		{
			result[i + i2] += hanning_window[i] * a2_rephased[i].r;
		}

		free(a1);
		free(a2);
		free(s1_in);
		free(s1_out);
		free(s2_in);
		free(s2_out);
		free(s2_rephased);
		free(a2_rephased);
	}

	float max = 0;
	for (int i = 0; i < (*samples_t)->size; ++i)
	{
		max = fabs(result[i]) > max ? fabs(result[i]) : max;
	}

	// float max_og = abs((*samples)->data[0]);
	// for (int i = 0; i < (*samples)->size; ++i)
	// {
	// 	max_og = abs((*samples)->data[i]) > max_og ? abs((*samples)->data[i]) : max_og;
	// }

	for (int i = 0; i < (*samples_t)->size; ++i)
	{
		float value = (pow(2, 14) * result[i] / max);
		(*samples_t)->data[i] = (int16_t)value;
	}

	// free(result);
	// free(phase);
	// free(hanning_window);
	free(cfg);
	free(cfg_i);
}

void magic()
{
	int window_size = header->datachunk_size / sizeof(int16_t);
    kiss_fft_cfg cfg = kiss_fft_alloc(window_size, 0, NULL, NULL);
    kiss_fft_cfg cfg_i = kiss_fft_alloc(window_size, 1, NULL, NULL);

    // kiss_fft_cpx *s1_in = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));
    // kiss_fft_cpx *s1_out = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));
   	// kiss_fft_cpx *s1_rephased = (kiss_fft_cpx*)malloc(window_size * sizeof(kiss_fft_cpx));

    // for (int i = 0; i < window_size; ++i)
    // {
    // 	s1_in[i].r = (*samples)[i];
    // }

    //kiss_fft(cfg, s1_in, s1_out);

    //kiss_fft(cfg_i, s1_out, s1_rephased);

 	free(cfg);
 	free(cfg_i);
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

    Sample *samples_c = (Sample*)malloc(sizeof(Sample));
    samples_c->data = samples;
    samples_c->size = header->datachunk_size / sizeof(int16_t);

    magic();

 //    cfg = kiss_fft_alloc(pow(2, 10), 0, NULL, NULL);
	// cfg_i = kiss_fft_alloc(pow(2, 10), 1, NULL, NULL);

	// free(cfg);
	// free(cfg_i);

 //    cfg = kiss_fft_alloc(pow(2, 10), 0, NULL, NULL);
	// cfg_i = kiss_fft_alloc(pow(2, 10), 1, NULL, NULL);

    Sample *samples0 = NULL, *samples1 = NULL, *samples_t = NULL;   

    pitchshift(&samples_c, &samples_t, 0);

    for (int semitone = 1; semitone < 3; semitone++)
    {
    	Sample *samples_temp = samples_t;
    	pitchshift(&samples_c, &samples0, semitone);

	    // for (int i = 0; i < samples0->size; ++i)
	    // {
	    // 	printf("%i\n", samples0->data[i]);
	    // }

	    header->datachunk_size = samples0->size * sizeof(int16_t);
	    char file[10];
	    sprintf(file, "%d.wav", semitone);
    	wavwrite(file, samples0->data, samples0->size);	

    	printf("%i\n", samples0->size);
    	printf("%i, %i\n", semitone, samples_t->size);

    	int size = samples_t->size;
    	samples_t = NULL;

    	superposition(&samples_temp, &samples0, &samples_t, size);
    	// free(samples_temp->data);
    	free(samples_temp);
    }    

    header->datachunk_size = samples_t->size * sizeof(int16_t);
    wavwrite("track2.wav", samples_t->data, samples_t->size);

    // free(header);
    // free(samples_c->data);
    // free(samples_c);
    // free(samples_t->data);
    // free(samples_t);
    // free(samples0->data);
    // free(samples0);
}
