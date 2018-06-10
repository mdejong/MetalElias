//
//  EliasgUtil.hpp
//
//  Created by Mo DeJong on 11/19/17.
//  MIT Licensed
//
// Huffman encoder for input symbols limited to the valid byte range
// of (0, 255) inclusive. This leads to huffman table codes that are
// a maximum of 16 bits wide which can be processed efficiently.

#ifndef EliasgUtil_hpp
#define EliasgUtil_hpp

#include <cstdint>
#include <vector>

// This header is pure C and can be included in either Objc or C++
#include "VariableBitWidthSymbol.h"

using namespace std;

/*

class EliasgUtil {

public:

  // Unoptimized serial decode logic. Note that this logic
  // assumes that huffBuff contains +2 bytes at the end
  // of the buffer to account for read ahead.
  
  static void
  decodeBits(
               int numSymbolsToDecode,
               uint8_t *bitBuff,
               int bitBuffN,
               uint8_t *outBuffer,
               uint32_t *bitOffsetTable);

  // Given an input buffer, huffman encode the input values and generate
  // output that corresponds to
  
  static void
  encodeBits(
                uint8_t* inBytes,
                int inNumBytes,
                vector<uint8_t> & outCodes,
                vector<uint32_t> & outBlockBitOffsets,
                int width,
                int height,
                int blockDim);
  
  static vector<int8_t>
  encodeSignedByteDeltas(const vector<int8_t> & bytes);
  
  static vector<int8_t>
  decodeSignedByteDeltas(const vector<int8_t> & deltas);

};

*/
 
#endif // EliasgUtil_hpp
