#include  <time.h>
#include  "../Synthesizer/piano.h"

#include  <os.h>

kiss_fft_cfg cfg, cfg_i;

void pitchshiftSpeedTest(Sample *inputSample, int semitone);
void speedxSpeedTest(Sample *testSample, float factor);
void stretchSpeedTest(Sample *inputSample, float factor);
void superpositionSpeedTest(Sample *inputSample1, Sample *inputSample2, int offset);
void pitchshiftFrequencyTest();
void pitchshiftStackUsageTest(INT8U task_prio, Sample **inputSample, Sample **outputSoundSample, int semitone);
void stackTest(INT8U task_prio);
