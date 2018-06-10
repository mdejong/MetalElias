// C++ impl of huffman utility functions
//  MIT Licensed

#include "EliasgUtil.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

//#include "EliasgUtilEncoder.hpp"

#include <assert.h>

using namespace std;

/*

// Unoptimized serial decode logic. Note that this logic
// assumes that huffBuff contains +2 bytes at the end
// of the buffer to account for read ahead.

void
EliasgUtil::decodeBits(
                               int numSymbolsToDecode,
                               uint8_t *bitBuff,
                               int bitBuffN,
                               uint8_t *outBuffer,
                               uint32_t *bitOffsetTable)
{
  uint16_t inputBitPattern = 0;
  unsigned int numBitsRead = 0;
  
  const int debugOut = 0;
  const int debugOutShowEmittedSymbols = 0;
  
  int symbolsLeftToDecode = numSymbolsToDecode;
  int symboli = 0;
  int bufferBitOffset = 0;
  
  int outOffseti = 0;
  
  for ( ; symbolsLeftToDecode > 0; symbolsLeftToDecode--, symboli++ ) {
    // Gather a 16 bit pattern by reading 2 or 3 bytes.
    
    if (debugOut) {
      printf("decode symbol number %5d : numBitsRead %d\n", symboli, numBitsRead);
    }
    
    const unsigned int numBytesRead = (numBitsRead / 8);
    const unsigned int numBitsReadMod8 = (numBitsRead % 8);
    
    // Read 3 bytes where a partial number of bits
    // is used from the first byte, then all the
    // bits in the second pattern are used, followed
    // by a partial number of bits from the 3rd byte.
#if defined(DEBUG)
    assert((numBytesRead+2) < huffBuffN);
#endif // DEBUG
    
    unsigned int b0 = huffBuff[numBytesRead];
    unsigned int b1 = huffBuff[numBytesRead+1];
    unsigned int b2 = huffBuff[numBytesRead+2];
    
    if (debugOut) {
      printf("read byte %5d : pattern %s\n", numBytesRead, get_code_bits_as_string(b0, 16).c_str());
      printf("read byte %5d : pattern %s\n", numBytesRead+1, get_code_bits_as_string(b1, 16).c_str());
      printf("read byte %5d : pattern %s\n", numBytesRead+2, get_code_bits_as_string(b2, 16).c_str());
    }
    
    // Prepare the input bytes using shifts so that the results always
    // fit into 16 bit intermediate registers.
    
    // Left shift the already consumed bits off left side of b0
    b0 <<= numBitsReadMod8;
    b0 &= 0xFF;
    
    if (debugOut) {
      printf("b0 %s\n", get_code_bits_as_string(b0, 16).c_str());
    }

    b0 = b0 << 8;
    
    if (debugOut) {
      printf("b0 %s\n", get_code_bits_as_string(b0, 16).c_str());
    }
    
    inputBitPattern = b0;
    
    if (debugOut) {
      printf("inputBitPattern (b0) %s : binary length %d\n", get_code_bits_as_string(inputBitPattern, 16).c_str(), 16);
    }
    
    // Left shift the 8 bits in b1 then OR into inputBitPattern
    
    if (debugOut) {
      printf("b1 %s\n", get_code_bits_as_string(b1, 16).c_str());
    }
    
    b1 <<= numBitsReadMod8;
    
    if (debugOut) {
      printf("b1 %s\n", get_code_bits_as_string(b1, 16).c_str());
    }
    
#if defined(DEBUG)
    assert((inputBitPattern & b1) == 0);
#endif // DEBUG
    
    inputBitPattern |= b1;
    
    if (debugOut) {
      printf("inputBitPattern (b1) %s : binary length %d\n", get_code_bits_as_string(inputBitPattern, 16).c_str(), 16);
    }
    
    if (debugOut) {
      printf("b2 %s\n", get_code_bits_as_string(b2, 16).c_str());
    }
    
    // Right shift b2 to throw out unused bits
    b2 >>= (8 - numBitsReadMod8);
    
    if (debugOut) {
      printf("b2 %s\n", get_code_bits_as_string(b2, 16).c_str());
    }
    
#if defined(DEBUG)
    assert((inputBitPattern & b2) == 0);
#endif // DEBUG
    
    inputBitPattern |= b2;
    
    if (debugOut) {
      printf("inputBitPattern (b2) %s : binary length %d\n", get_code_bits_as_string(inputBitPattern, 16).c_str(), 16);
    }
    
    if (debugOut) {
      printf("input bit pattern %s : binary length %d\n", get_code_bits_as_string(inputBitPattern, 16).c_str(), 16);
    }
    
    // Lookup shortest matching bit pattern
    HuffLookupSymbol hls = huffSymbolTable[inputBitPattern];
#if defined(DEBUG)
    assert(hls.bitWidth != 0);
#endif // DEBUG
    
    numBitsRead += hls.bitWidth;
    
    if (debugOut) {
      printf("consume symbol bits %d\n", hls.bitWidth);
    }
    
    char symbol = hls.symbol;
    
    outBuffer[outOffseti++] = symbol;
    
    if (debugOut) {
      printf("write symbol %d\n", symbol & 0xFF);
    }
    
    if (debugOutShowEmittedSymbols) {
      printf("out[%5d] = %3d (aka 0x%02X) : bits %2d : total num bits %5d\n", outOffseti-1, symbol&0xFF, symbol, hls.bitWidth, numBitsRead-hls.bitWidth);
    }
    
    if (bitOffsetTable != NULL) {
      bitOffsetTable[symboli] = bufferBitOffset;
      bufferBitOffset += hls.bitWidth;
    }
  }
  
  return;
}

// Given an input buffer, huffman encode the input values and generate
// output that corresponds to

void
EliasgUtil::encodeBits(
                           uint8_t* inBytes,
                           int inNumBytes,
                           vector<uint8_t> & outFileHeader,
                           vector<uint8_t> & outCanonHeader,
                           vector<uint8_t> & outHuffCodes,
                           vector<uint32_t> & outBlockBitOffsets,
                           int width,
                           int height,
                           int blockDim)
{
  HuffmanEncoder enc;
  
  vector<uint8_t> bytes;
  bytes.reserve(inNumBytes);
  
  for (int i = 0; i < inNumBytes; i++) {
    int c = inBytes[i];
    bytes.push_back(c);
  }
  
  vector<uint8_t> headerBytes;
  vector<uint8_t> canonicalTableBytes;
  vector<uint8_t> huffmanCodeBytes;
  
  bool worked = enc.encode(bytes,
                           headerBytes,
                           canonicalTableBytes,
                           huffmanCodeBytes);
  assert(worked);
  
  // Copy canon table of 256 bytes back to caller
  
  assert(canonicalTableBytes.size() == 256);
  outCanonHeader = canonicalTableBytes;
  
  // Copy generated huffman codes back to caller
  
  //[outHuffCodes setLength:huffmanCodeBytes.size()];
  //uint8_t *outHuffCodesPtr = (uint8_t *) outHuffCodes.mutableBytes;

//  uint8_t *codesPtr = (uint8_t *) huffmanCodeBytes.data();
//  int codesN = (int) huffmanCodeBytes.size();
//
//  for ( int i = 0 ; i < codesN; i++) {
//    uint8_t code = codesPtr[i];
//    outHuffCodesPtr[i] = code;
//  }

  outHuffCodes = std::move(huffmanCodeBytes);
  
  // Process the input data in terms of NxN blocks, so that a given width x height
  // combination is split into blocks. Then determine the positions of each block
  // starting point and pass these indexes into the encode module so that the bit
  // offset at each position can be determined.
  
  vector<uint32_t> bufferOffsetsToQuery;
  
  int numBlocks = (int)bytes.size() / (blockDim * blockDim);
  
  for ( int i = 0; i < numBlocks; i += 1) {
    int offset = i * (blockDim * blockDim);
    bufferOffsetsToQuery.push_back(offset);
  }
  
  vector<uint32_t> blockBitOffsetBytes = enc.lookupBufferBitOffsets(bufferOffsetsToQuery);
  
//  [outBlockBitOffsets setLength:bufferOffsetsToQuery.size()*sizeof(uint32_t)];
//
//  uint32_t *outBlockBitOffsetsPtr = (uint32_t *) outBlockBitOffsets.bytes;
//  int outBlockBitOffsetsi = 0;
//
//  for ( uint32_t offset : blockBitOffsetBytes ) {
//    outBlockBitOffsetsPtr[outBlockBitOffsetsi++] = offset;
//  }

  outBlockBitOffsets = blockBitOffsetBytes;
  
  return;
}

vector<int8_t>
EliasgUtil::encodeSignedByteDeltas(
                           const vector<int8_t> & bytes)
{
  return encodeDelta(bytes);
}

vector<int8_t>
EliasgUtil::decodeSignedByteDeltas(
                                    const vector<int8_t> & deltas)
{
  return decodeDelta(deltas);
}

*/

