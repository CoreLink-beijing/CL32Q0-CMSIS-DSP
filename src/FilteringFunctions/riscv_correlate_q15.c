/* ----------------------------------------------------------------------   
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.   
*   
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*   
* Project: 	    CMSIS DSP Library   
* Title:		arm_correlate_q15.c   
*   
* Description:	Correlation of Q15 sequences. 
*   
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the 
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.  
* -------------------------------------------------------------------- */

#include "riscv_math.h"

/**   
 * @ingroup groupFilters   
 */

/**   
 * @addtogroup Corr   
 * @{   
 */

/**   
 * @brief Correlation of Q15 sequences. 
 * @param[in] *pSrcA points to the first input sequence.   
 * @param[in] srcALen length of the first input sequence.   
 * @param[in] *pSrcB points to the second input sequence.   
 * @param[in] srcBLen length of the second input sequence.   
 * @param[out] *pDst points to the location where the output result is written.  Length 2 * max(srcALen, srcBLen) - 1.   
 * @return none.   
 *   
 * @details   
 * <b>Scaling and Overflow Behavior:</b>   
 *   
 * \par   
 * The function is implemented using a 64-bit internal accumulator.   
 * Both inputs are in 1.15 format and multiplications yield a 2.30 result.   
 * The 2.30 intermediate results are accumulated in a 64-bit accumulator in 34.30 format.   
 * This approach provides 33 guard bits and there is no risk of overflow.   
 * The 34.30 result is then truncated to 34.15 format by discarding the low 15 bits and then saturated to 1.15 format.   
 *   
 * \par   
 * Refer to <code>riscv_correlate_fast_q15()</code> for a faster but less precise version of this function for Cortex-M3 and Cortex-M4. 
 *
 * \par    
 * Refer the function <code>riscv_correlate_opt_q15()</code> for a faster implementation of this function using scratch buffers.
 * 
 */

void riscv_correlate_q15(
  q15_t * pSrcA,
  uint32_t srcALen,
  q15_t * pSrcB,
  uint32_t srcBLen,
  q15_t * pDst)
{

  q15_t *pIn1 = pSrcA;                           /* inputA pointer               */
  q15_t *pIn2 = pSrcB + (srcBLen - 1u);          /* inputB pointer               */
  q63_t sum;                                     /* Accumulators                  */
  uint32_t i = 0u, j;                            /* loop counters */
  uint32_t inv = 0u;                             /* Reverse order flag */
  uint32_t tot = 0u;                             /* Length */

  /* The algorithm implementation is based on the lengths of the inputs. */
  /* srcB is always made to slide across srcA. */
  /* So srcBLen is always considered as shorter or equal to srcALen */
  /* But CORR(x, y) is reverse of CORR(y, x) */
  /* So, when srcBLen > srcALen, output pointer is made to point to the end of the output buffer */
  /* and a varaible, inv is set to 1 */
  /* If lengths are not equal then zero pad has to be done to  make the two   
   * inputs of same length. But to improve the performance, we include zeroes   
   * in the output instead of zero padding either of the the inputs*/
  /* If srcALen > srcBLen, (srcALen - srcBLen) zeroes has to included in the   
   * starting of the output buffer */
  /* If srcALen < srcBLen, (srcALen - srcBLen) zeroes has to included in the  
   * ending of the output buffer */
  /* Once the zero padding is done the remaining of the output is calcualted  
   * using convolution but with the shorter signal time shifted. */

  /* Calculate the length of the remaining sequence */
  tot = ((srcALen + srcBLen) - 2u);

  if(srcALen > srcBLen)
  {
    /* Calculating the number of zeros to be padded to the output */
    j = srcALen - srcBLen;

    /* Initialise the pointer after zero padding */
    pDst += j;
  }

  else if(srcALen < srcBLen)
  {
    /* Initialization to inputB pointer */
    pIn1 = pSrcB;

    /* Initialization to the end of inputA pointer */
    pIn2 = pSrcA + (srcALen - 1u);

    /* Initialisation of the pointer after zero padding */
    pDst = pDst + tot;

    /* Swapping the lengths */
    j = srcALen;
    srcALen = srcBLen;
    srcBLen = j;

    /* Setting the reverse flag */
    inv = 1;

  }

  /* Loop to calculate convolution for output length number of times */
  for (i = 0u; i <= tot; i++)
  {
    /* Initialize sum with zero to carry on MAC operations */
    sum = 0;

    /* Loop to perform MAC operations according to convolution equation */
    for (j = 0u; j <= i; j++)
    {
      /* Check the array limitations */
      if((((i - j) < srcBLen) && (j < srcALen)))
      {
        /* z[i] += x[i-j] * y[j] */
        sum += ((q31_t) pIn1[j] * pIn2[-((int32_t) i - j)]);
      }
    }
    /* Store the output in the destination buffer */
    if(inv == 1)
      *pDst-- = (q15_t) __SSAT((sum >> 15u), 16u);
    else
      *pDst++ = (q15_t) __SSAT((sum >> 15u), 16u);
  }


}

/**   
 * @} end of Corr group   
 */
