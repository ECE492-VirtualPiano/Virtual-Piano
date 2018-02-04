/*
    Entity name:    wav.h

    Author:         vineeshvs

    Date:           Jan 29, 2018

    Project:        ECE 492 Capstone Project - Virtual Piano

    Description:    This file is a header file for reading and writing wav files.

    Derived from:   vineeshvs
                    https://www.daniweb.com/programming/software-development/threads/340334/reading-audio-file-in-c
*/

#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;
    char     datachunk_id[4];
    uint32_t datachunk_size;
}WavHeader;

typedef struct {
    int16_t *data;
    int     size;
}Sample;

WavHeader *header;

void wavread(char *file_name, int16_t **samples)
{
    int fd;
    if (!file_name)
        errx(1, "Filename not specified");
    if ((fd = open(file_name, O_RDONLY)) < 1)
        errx(1, "Error opening file");
    if (!header)
        header = (WavHeader*)malloc(sizeof(WavHeader));
    if (read(fd, header, sizeof(WavHeader)) < sizeof(WavHeader))
        errx(1, "File broken: header");
    if (strncmp(header->chunk_id, "RIFF", 4) ||
        strncmp(header->format, "WAVE", 4))
        errx(1, "Not a wav file");
    if (header->audio_format != 1)
        errx(1, "Only PCM encoding supported");
    if (*samples) free(*samples);
    *samples = (int16_t*)malloc(header->datachunk_size);
    if (!*samples)
        errx(1, "Error allocating memory");
    if (read(fd, *samples, header->datachunk_size) < header->datachunk_size)
        errx(1, "File broken: samples");
    close(fd);
}

void wavwrite(char *file_name, int16_t *samples, int size)
{
    int fd;
    if (!file_name)
        errx(1, "Filename not specified");
    if (!samples)
        errx(1, "Samples buffer not specified");
    if ((fd = creat(file_name, 0666)) < 1)
        errx(1, "Error creating file");
    if (write(fd, header, sizeof(WavHeader)) < sizeof(WavHeader))
        errx(1, "Error writing header");
    if (write(fd, samples, size * sizeof(int16_t)) < size * sizeof(int16_t))
        errx(1, "Error writing samples");
    close(fd);
}