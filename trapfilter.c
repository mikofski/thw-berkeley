#include <stdio.h>
/***
 * Trapezoidal filter function for use with python high level analysis and
 * C binary data reader tools. Implementation is based on lecture notes given
 * by Valentin Jordanov.
 *
 * @author: Cameron Bates
 */

//These are needed unless you compiling in c99 mode
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


// defines a sample delay length must be greater than peaking + gap
#ifndef MAX_DELAY
#define MAX_DELAY 16384
#endif


double trapfilter(int *pulse, int peaking, int gap, int M, int len, int len_in,
                  double N, int bl, int pol) {
	/***
	 * primary function inputs are as follows
	 * int *pulse: array containing raw pulse(this will be replaced by the filetered pulse)
	 * int peaking: peaking time in samples of the trapezoidal filter
	 * int gap: gap time in samples of the trapezoidal filter
	 * int M: Decay correction factor ~ decay constant in samples
	 * int len: actual length of the raw pulse
	 * int len_in: shortened length of the raw pulse(improves speed)
     * double N: decay correction parameter 2. Set this to 0 and M to 1 to
     *     remove decay correction. Otherwise this should be 1
	 *
	 * This function returns the max of the trapezoidal filter output.
	 */


    // Initialize both delay buffers k-peaking. m- peaking + gap
    int delayk_buffer[MAX_DELAY];
    int delayk_index ;
    int shiftk ; //peaking time
    long datak_out; //delayed output
    int delaym_buffer[MAX_DELAY];
    int delaym_index ;
    int shiftm; //peaking time + gap time
    long datam_in; //input data
    long datam_out; //delayed output

    //output of shaper
    double slowshaperout;

    /*
    //This is no longer used because it was unstable for large ranges of peaking times
    //Peak Detect stuff
    long data_in, channel_pointer;
    int peak_detect, old_peak_detect;
    long comp_A, comp_B, noise_thresh;
    long peak, gain_scale;
    //long spectrum[MAX_CHANNELS]
    */


    //clear delay buffers
    int i = 0;
    for (i = 0; i < MAX_DELAY; i++){
               delayk_buffer[i] = 0;
               delaym_buffer[i] = 0;
    }
    delayk_index = 0;
    shiftk = peaking;
    delaym_index = 0;
    shiftm = peaking + gap;
    if (len_in < len){
      len = len_in;
    }
    /*
     * More peak detection stuff
    noise_thresh = noise_thresh_in*1000;
    //initialization
    peak = 0;
    //noise_thresh = //300000000*peaking/1200; good for Cs-137
    channel_pointer = 0;
    comp_B = - noise_thresh;
    old_peak_detect = TRUE ;
    gain_scale = 1000;
    */


    //initialize variables for trapezoidal filter
    int n = 0;
    i = 0;
    double max = 0;
    double accum1 = 0;
    double accum2 = 0;
    double adder2out;
    double adder3out;
    int input;

    while (i < len ){
        input = (pulse[i] - bl) * pol;


        //peaking time delay
        datak_out = delayk_buffer[delayk_index];
        delayk_buffer[delayk_index] = input;
        delayk_index++;
        if (delayk_index >= shiftk) delayk_index = 0;

        // input to second delay stage is output of first minus input (moving average 1)
        datam_in = input - datak_out;

        datam_out = delaym_buffer[delaym_index];
        delaym_buffer[delaym_index] = datam_in;
        delaym_index++;
        if (delaym_index >= shiftm) delaym_index = 0;

        // subtraction of second stage output from first delay output (moving average 2)
        adder2out = datam_in - datam_out;
        // first accumulator
        accum1 += adder2out;
        //decay correction
        adder3out = adder2out*M + accum1*N;
        //second accumulator
        accum2 += adder3out;
        //shaper output (scaled by peaking time)
        slowshaperout = accum2/(double)peaking;



        /*
        // Peak detection threshold is not linear for large changes in peaking time
        comp_A = slowshaperout - peak;
        if (comp_A > comp_B){
               peak_detect = TRUE;
               comp_B = - noise_thresh;
        }
        else{
               peak_detect = FALSE;
               comp_B = noise_thresh;
        }
        if ((comp_A >= 0 && peak_detect==TRUE) || (comp_A < 0 && peak_detect==FALSE)) peak = slowshaperout;
        if ( peak_detect == FALSE && old_peak_detect == TRUE){
            channel_pointer = peak/gain_scale;
            if (channel_pointer < 0) channel_pointer = 0;
            if (channel_pointer > 0){
                //printf("peak: %d \n",channel_pointer);
                return channel_pointer;
            }

        }
        old_peak_detect = peak_detect;
        */

        //determines max of the slow shaper output
        if (slowshaperout > max){
            max = slowshaperout;
        }
        // This can be used to see what the shaper output is
        pulse[i] = slowshaperout;
        i++;

    }
    return max;
}
