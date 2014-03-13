/*
 *  Title : detect_handshake.c
 *  Author: Chris Garry
 *  Date  : 3/12/2014
 *
 *  Description:
 *              Handshake gesture recognition using Dynamic Time Warping (DTW)
 *              or the Fast Fourier Transform (FFT).
 *              Prints a message to the display when a Pebble Smart Watch 
 *              user engages a handshake.
 */

#include <pebble.h>
#include "fft_lib/kiss_fftr.h" //FOR FFT

#define WINDOW_SIZE 14 //Accerometer sample window size to capture a full handshake
#define SAMPLE_RATE ACCEL_SAMPLING_10HZ
#define DIMENSION 30   //FOR DTW: Dimension of the accelerometer space
#define MAX_DISTANCE 1 //FOR DTW: Threshold distance between reference handshake and candidate
#define DTW 0
#define FFT 1
#define MODE DTW //Choose gesture recognition technique


//For displaying content on the watch face.
//Although the window might not be used to display content, its declaration is required.
Window *my_window;

//A renderable graphics element. Content is rendered on a particular layer within the window.
TextLayer *text_layer;

// FFT LIBRARY:
    
    int kiss_fft_next_fast_size();

// DTW LIBRARY:

    //The reference handshake to compare accelerometer data to for DTW
    int **handshake_ref;
    int handshake_ref_size;

    //FOR DTW: Allocate space for accelerometer data matrix (each row is an [x,y,z])
    int** allocAccBuf(int len){
        int** ret = (int**)malloc(sizeof(int*)*len);
        int i;
        for( i = 0; i < len; i++)
            ret[i] = (int*)malloc(sizeof(int)*DIMENSION);
        return ret;
    }

    //FOR DTW: Free the space of an accelerometer data matrix
    void releaseAccBuf(int** p, int len) {
        int i;
        for( i = 0; i < len; i++)
            free(p[i]);
        free(p);
    }

    //FOR DTW: Dynamic Time Warping Distance: Return integer distance between two discrete time signals.
    int DTWdistance(int **sample1, int length1, int **sample2, int length2, int i, int j, int *table) {
        
        if( i < 0 || j < 0)
            return 100000000;
        int tableWidth = length2;
        int localDistance = 0;
        int k;
        for( k = 0; k < DIMENSION; k++)
            localDistance += ((sample1[i][k]-sample2[j][k])*(sample1[i][k]-sample2[j][k]));
        
        int sdistance, s1, s2, s3;
        
        if( i == 0 && j == 0) {
            if( table[i*tableWidth+j] < 0)
                table[i*tableWidth+j] = localDistance;
            return localDistance;
        } else if( i==0) {
            if( table[i*tableWidth+(j-1)] < 0)
                sdistance = DTWdistance(sample1, length1, sample2, length2, i, j-1, table);
            else
                sdistance = table[i*tableWidth+j-1];
        } else if( j==0) {
            if( table[(i-1)*tableWidth+ j] < 0)
                sdistance = DTWdistance(sample1, length1, sample2, length2, i-1, j, table);
            else
                sdistance = table[(i-1)*tableWidth+j];  
        } else {
            if( table[i*tableWidth+(j-1)] < 0)
                s1 = DTWdistance(sample1, length1, sample2, length2, i, j-1, table);
            else
                s1 = table[i*tableWidth+(j-1)];
            if( table[(i-1)*tableWidth+ j] < 0)
                s2 = DTWdistance(sample1, length1, sample2, length2, i-1, j, table);
            else
                s2 = table[(i-1)*tableWidth+ j];
            if( table[(i-1)*tableWidth+ j-1] < 0)
                s3 = DTWdistance(sample1, length1, sample2, length2, i-1, j-1, table);
            else
                s3 = table[(i-1)*tableWidth+ j-1];
            sdistance = s1 < s2 ? s1:s2;
            sdistance = sdistance < s3 ? sdistance:s3;
        }
        table[i*tableWidth+j] = localDistance + sdistance;
        return table[i*tableWidth+j];
    }

//This handler will be called when enough samples have been received to fill one batch of data
void accel_data_handler(AccelData *data, uint32_t num_samples) {

    int i, j, distance, *table, **sample, OPTIMAL_SIZE;

    switch(MODE){

        case DTW:
            //DTW
                //Compute DTW Distance

                    //Extract candidate sample from AccelData struct
                        sample = allocAccBuf(WINDOW_SIZE);
                        for(i = 0; i < (int)num_samples; i++){
                            sample[i][0] = (int)data[i].x;
                            sample[i][1] = (int)data[i].y;
                            sample[i][2] = (int)data[i].z;
                        }

                    //Initialize DTW table
                        table = (int*) malloc(num_samples * handshake_ref_size*sizeof(int));
                        for( j = 0; j < WINDOW_SIZE*handshake_ref_size; j++)
                            table[j] = -1;

                    //Compute DTW Distance between candidate sample and reference
                        distance = DTWdistance(sample,WINDOW_SIZE, handshake_ref, handshake_ref_size,WINDOW_SIZE-1,handshake_ref_size-1, table);

                //If DTW Distance between candidate sample and reference is small, handshake detected
                    
                    //TODO: Determine optimal MAX_DISTANCE
                    if(distance < MAX_DISTANCE){
                        //print handshake detected for three seconds, then clear screen
                    }

                    //Clean up
                        free(table);
                        releaseAccBuf(sample, WINDOW_SIZE);
                        releaseAccBuf(handshake_ref, WINDOW_SIZE);
            break;

        case FFT: 
            //FFT
                //Compute ONE-DIMENSIONAL REAL FFT

                    
                    //TODO: THE FOLLOWING CODE HAS DEPENDENCIES THAT PEBBLE 
                    //DOES NOT INCLUDE IN THE SDK, SO BUILD WILL FAIL. SEE fft_lib/TODO.txt
                    
                        /*

                        //Determine the computationally optimal array size for the input signal size
                        OPTIMAL_SIZE =  kiss_fft_next_fast_size((int)num_samples);

                        //Allocate memory for the variables and set configuration vars
                        //arguments: (array size, is_inverse_fft, malloc config, malloc config)
                        kiss_fftr_cfg configBuffer = kiss_fftr_alloc(OPTIMAL_SIZE, 0, 0, 0);

                        //Store sample into kiss_fft_scalar *timedata 
                        kiss_fft_scalar timedata[OPTIMAL_SIZE]; 
                        for(i=0;i<OPTIMAL_SIZE; i++){
                            timedata[i] = 0;
                            if(i<WINDOW_SIZE)
                                timedata[i] = (kiss_fft_scalar)data[i].y;
                        }

                        //Create output array
                        kiss_fft_cpx freqdata[(OPTIMAL_SIZE/2)+1];


                        //Compute FFT
                        kiss_fftr(configBuffer,timedata,freqdata);
                        */

                //If peak frequency between 1.5 and 4Hz, display "Handshake Detected" for 3 seconds.
                    //...TODO: code

            break;

        default:
            break;
    }

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

    if(MODE == DTW){
        //Initialize reference handshake
        handshake_ref = allocAccBuf(WINDOW_SIZE);
        handshake_ref_size = WINDOW_SIZE;
    } 
  
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
