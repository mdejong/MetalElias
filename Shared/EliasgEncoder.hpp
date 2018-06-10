//
//  EliasgEncoder.hpp
//
//  Created by Mo DeJong on 11/19/17.
//  MIT Licensed
//
// Huffman encoder for input symbols limited to the valid byte range
// of (0, 255) inclusive. This leads to huffman table codes that are
// a maximum of 16 bits wide which can be processed efficiently.

#ifndef _EliasgEncoder_hpp
#define _EliasgEncoder_hpp

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

/*

class EliasgEncoder
{
private:
  // Record the bit location in the emitted huffman symbol
  // stream where a given symbol starts.
  vector<uint32_t> bitOffsetForSymbols;
    
public:
  
  EliasgEncoder();
  
  // Entry point for original byte to huffman encoding. The
  // header is always a fixed 256 byte canonical table.
  
  bool encode(const vector<uint8_t> & bytes,
              vector<uint8_t> & encodedBytes);

  // Generate a table of bit width offsets for N symbols, this generate loop
  // is optimzied so that it is much faster than generating codes directly.
    
  vector<uint32_t> generateBitOffsets(const vector<uint8_t> & symbols);
    
  vector<uint32_t> lookupBufferBitOffsets(const vector<uint32_t> & offsets);
  
};
 
*/

#endif // _EliasgEncoder_hpp
