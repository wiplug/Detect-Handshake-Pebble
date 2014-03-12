#include <pebble.h>
#include "kiss_fftr.h"

/*
PEBBLE LIB DOES NOT CONTAIN THE FOLLOWING REQUIRED DEPENDENCIES:

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>

*/

#define WINDOW_SIZE 14
#define SAMPLE_RATE ACCEL_SAMPLING_10HZ

 
//For displaying content on the watch face.
//Although the window might not be used to display content, its declaration is required.
Window *my_window;

//A renderable graphics element. Content is rendered on a particular layer within the window.
TextLayer *text_layer;

//This handler will be called when enough samples have been received to fill one batch of data
void accel_data_handler(AccelData *data, uint32_t num_samples) {

        //Compute FFT

            //Determine the computationally optimal array size for the input signal
            int OPTIMAL_SIZE =  kiss_fft_next_fast_size((int)num_samples);
            
        /*  THE FOLLOWING CODE HAS STANDARD DEPENDENCIES THAT PEBBLE 
            DOES NOT INCLUDE IN THE SDK WHICH WE NEED:
            
            //Allocate memory for the variables and set configuration vars
            //0=forward transform, NULL,NULL=do not malloc
            //kiss_fftr_cfg configBuffer = kiss_fftr_alloc(OPTIMAL_SIZE, 0, 0, 0);
            kiss_fft_cfg  configBuffer = kiss_fft_alloc(OPTIMAL_SIZE, 0, 0, 0);

            //Store AccelData into const kiss_fft_scalar *timedata for FFT
            const kiss_fft_scalar timedata[OPTIMAL_SIZE]; //TODO: store data;

            //Create output array
            kiss_fft_cpx freqdata[(OPTIMAL_SIZE/2)+1];

            //Compute FFT
            //kiss_fftr(configBuffer,timedata,freqdata);

        */

        //If peak frequency between 1.5 and 4Hz (2.5Hz avg), display "Handshake Detected" for 3 seconds.
            //...code
}

//Initialization of parameters for the mobile app
void handle_init(void) {
  
    //Create a window to display content
	my_window = window_create();
    //Push window onto the window stack
    window_stack_push(my_window, true /* true means use default window load animation */);
    //Create a rendering layer for window
    Layer *window_layer = window_get_root_layer(my_window);
    //Get window layer dimensions/bounds
    GRect bounds = layer_get_frame(window_layer);
  
    //Create a text layer
    text_layer = text_layer_create((GRect){ .origin = { 0, 30 }, .size = bounds.size });
  
    //Subscribe/initialize the batch accelerometer data processing with 14 samples 
    //per batch and pass to accel_data_handler for processing
    accel_data_service_subscribe(WINDOW_SIZE, &accel_data_handler);
  
    //Set accelerometer sampling rate to 10Hz (important for window size).
    //If sampling rate is increased, then batch size must be increased to 
    //capture whole handshake within the window.s
    accel_service_set_sampling_rate(SAMPLE_RATE);
  
  //TODO: Can peek at accelerometer service for individual value. Good for detecting onset of handshake.
}

//Deinitialization (mem free, etc.) of initialization parameters upon quit 
void handle_deinit(void) {
	  text_layer_destroy(text_layer);
	  window_destroy(my_window);
      accel_data_service_unsubscribe();
}


//Entry point for the Pebble app
int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
