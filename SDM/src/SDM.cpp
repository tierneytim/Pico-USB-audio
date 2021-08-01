#include "SDM.h"

SDM::SDM() {
}

 // oversample X 32(Direct form 2)
  uint32_t SDM::o4_os32_df2(int16_t sig) {

    uint32_t out = 0;
    int32_t d = -65536 - sig;   
    for (int j = 0; j < 32; j++) {
      
      // direct form 2 feedback
      int32_t wn = d +4*(w[0]+w[2])-6*w[1]-w[3];
      int32_t etmp = -3271 * w[0] + 3986 * w[1] - 2187 * w[2] + 455 * w[3]+wn*1024;
   
      // update previous values
      w[3] = w[2];
      w[2] = w[1];
      w[1] = w[0];
      w[0]= wn;
      
      // checks if current error minises sum of squares error
      if (etmp < 0) {
       w[0] += 131072;
       out += (1 << j);
      }
    }
   
    return out;
  }
 

 
  
  // 64 X oversampling, direct form 2 
/*   void SDM::o4_os64_df2(int16_t sig,uint32_t* output) {

    //out64[0] = 0;
    //out64[1] = 0;
    
    int32_t d = -98304 - sig; 
    for(int i =0;i<2;i++){
		
		 uint32_t out =0;
		for (int j = 0; j < 32; j++) {
      
			// direct form 2 feedback
			int32_t wn64 = d +4*(w64[0]+w64[2])-6*w64[1]-w64[3];
			int32_t etmp = -3271 * w64[0] + 3986 * w64[1] - 2187 * w64[2] + 455 * w64[3]+wn64*1024;
   
			// update previous values
			w64[3] = w64[2];
			w64[2] = w64[1];
			w64[1] = w64[0];
			w64[0]= wn64;
      
			// checks if current error minises sum of squares error
			if (etmp < 0) {
				w64[0] += 196608;
				out += (1 << j);
			}
		}
		output[i]=out;
	}
	
  }
 */  
  
  
    // oversample X 32(Direct form 1)
  uint32_t SDM::o4_os32(int16_t sig) {

    uint32_t out = 0;
    int32_t d = -98304 - sig;   
    int32_t d128 = d*128;
    int32_t etmp;
    for (int j = 0; j < 32; j++) {
      // the feedback is composed of deltas(digital output - digital input) and errors
      // this could probably be more efficient and is the bottleneck
    
     etmp = d128 -409 * buff[4] + 498 * buff[5]- 273 * buff[6]  + 57 * buff[7]+(buff[0] + buff[2]) * 4 - 6 * buff[1] - buff[3];
     
     
      buff[7] = buff[6];
      buff[6] = buff[5];
      buff[5] = buff[4];
      buff[4] = d;
      buff[3] = buff[2];
      buff[2] = buff[1];
      buff[1] = buff[0];
      buff[0] = etmp;
      

      
      // checks if current error minises sum of squares error
      // if not it changes the deltas and errors.
      if (etmp < 0) {
       buff[4] += 196608;
       buff[0] += 25165824;
       out += (1 << j);
      }
      
    }
   
    return out;
  }
