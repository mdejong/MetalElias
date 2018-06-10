// Objective C interface to huffman parsing functions
//  MIT Licensed

#import "Eliasg.h"

#import "EliasgUtil.hpp"

#include <assert.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "EliasgEncoder.hpp"

#import "elias.hpp"

using namespace std;

// Invoke huffman util module functions

static inline
vector<uint8_t> encode(const uint8_t * bytes,
            const int numBytes)
{
    EliasGammaEncoder encoder;
    
    encoder.emitPaddingZeros = true;
    encoder.emitMSB = true;
    encoder.encode(bytes, numBytes);
    
    return std::move(encoder.bytes);
}

static inline
bool decode(const uint8_t *encodedBitsPtr,
            const unsigned int numSymbols,
            vector<uint8_t> & symbols)
{
    EliasGammaDecoderOpt16 decoder;
    
    decoder.decode(encodedBitsPtr,
                   numSymbols,
                   symbols);
    
    return true;
}

// Generate a table of bit width offsets for N symbols, this is
// the symbol width added to a running counter of the offset
// into a buffer.

// FIXME: would be more optimal to generate a table of just the block
// offsets instead of all of the symbols in a table.

static inline
vector<uint32_t> generateBitOffsets(const uint8_t * symbols, int numSymbols)
{
    vector<uint32_t> bitOffsets;
    bitOffsets.reserve(numSymbols);
    
    unsigned int offset = 0;
    
    EliasGammaEncoder encoder;
    
    for ( int i = 0; i < numSymbols; i++ ) {
        bitOffsets.push_back(offset);
        uint8_t symbol = symbols[i];
        uint32_t bitWidth = encoder.numBits(symbol);
        offset += bitWidth;
    }
    
    return bitOffsets;
}

// Generate signed delta, note that this method supports repeated value that delta to zero

template <typename T>
vector<T>
encodeDelta(const vector<T> & orderVec)
{
    T prev;
    vector<T> deltas;
    deltas.reserve(orderVec.size());
    
    // The first value is always a delta from zero, so handle it before
    // the loop logic.
    
    {
        T val = orderVec[0];
        deltas.push_back(val);
        prev = val;
    }
    
    int maxi = (int) orderVec.size();
    for (int i = 1; i < maxi; i++) {
        T val = orderVec[i];
        T delta = val - prev;
        deltas.push_back(delta);
        prev = val;
    }
    
    return std::move(deltas);
}

template <typename T>
vector<T>
decodePlusDelta(const vector<T> &deltas, const bool minusOne = false)
{
    T prev;
    vector<T> values;
    values.reserve(deltas.size());
    
    // The first value is always a delta from zero, so handle it before
    // the loop logic.
    
    {
        T val = deltas[0];
        values.push_back(val);
        prev = val;
    }
    
    int maxi = (int) deltas.size();
    for (int i = 1; i < maxi; i++) {
        T delta = deltas[i];
        if (minusOne) {
            delta += 1;
        }
        T val = prev + delta;
        values.push_back(val);
        prev = val;
    }
    
    return std::move(values);
}

template <typename T>
vector<T>
decodeDelta(const vector<T> &deltas)
{
    return decodePlusDelta(deltas, false);
}

// zerod representation

// 0 = 0, -1 = 1, 1 = 2, -2 = 3, 2 = 4, -3 = 5, 3 = 6

uint32_t
pixelpack_num_neg_to_offset(int32_t value) {
    if (value == 0) {
        return value;
    } else if (value < 0) {
        return (value * -2) - 1;
    } else {
        return value * 2;
    }
}

int32_t
pixelpack_offset_to_num_neg(uint32_t value) {
    if (value == 0) {
        return value;
    } else if ((value & 0x1) != 0) {
        // odd numbers are negative values
        return ((int)value + 1) / -2;
    } else {
        return value / 2;
    }
}

int8_t
pixelpack_offset_uint8_to_int8(uint8_t value)
{
    int offset = (int) value;
    int iVal = pixelpack_offset_to_num_neg(offset);
    assert(iVal >= -128);
    assert(iVal <= 127);
    int8_t sVal = (int8_t) iVal;
    return sVal;
}

uint8_t
pixelpack_int8_to_offset_uint8(int8_t value)
{
    int iVal = (int) value;
    int offset = pixelpack_num_neg_to_offset(iVal);
    assert(offset >= 0);
    assert(offset <= 255);
    uint8_t offset8 = offset;
#if defined(DEBUG)
    {
        // Validate reverse operation, it must regenerate value
        int8_t decoded = pixelpack_offset_uint8_to_int8(offset8);
        assert(decoded == value);
    }
#endif // DEBUG
    return offset8;
}

// Main class performing the rendering

@implementation Eliasg

// Given an input buffer, huffman encode the input values and generate
// output that corresponds to

+ (void) encodeBits:(uint8_t*)inBytes
         inNumBytes:(int)inNumBytes
           outCodes:(NSMutableData*)outCodes
 outBlockBitOffsets:(NSMutableData*)outBlockBitOffsets
              width:(int)width
             height:(int)height
           blockDim:(int)blockDim
{
  vector<uint8_t> outBytesVec = encode(inBytes, inNumBytes);
    
  {
      // Copy from outBytesVec to outCodes
      NSMutableData *mData = outCodes;
      int numBytes = (int)(outBytesVec.size() * sizeof(uint8_t));
      [mData setLength:numBytes];
      memcpy(mData.mutableBytes, outBytesVec.data(), numBytes);
  }
    
  // Generate bit width lookup table from original input symbols
  vector<uint32_t> offsetsVec = generateBitOffsets(inBytes, inNumBytes);

  // The outBlockBitOffsets output contains bit offsets of the start
  // of each block, so skip over (blockDim * blockDim) offsets on
  // each lookup.

  const int maxOffset = (width * height);
  const int blockN = (blockDim * blockDim);
    
  vector<uint32_t> blockStartOffsetsVec;
  blockStartOffsetsVec.reserve(maxOffset / blockN);

  for (int offset = 0; offset < maxOffset; offset += blockN ) {
      int blockStartBitOffset = offsetsVec[offset];
      blockStartOffsetsVec.push_back(blockStartBitOffset);
  }

  {
      int numBytes = (int) (blockStartOffsetsVec.size() * sizeof(uint32_t));
      if ((int)outBlockBitOffsets.length != numBytes) {
          [outBlockBitOffsets setLength:numBytes];
      }
      memcpy(outBlockBitOffsets.mutableBytes, blockStartOffsetsVec.data(), numBytes);
  }
  
  return;
}

// Unoptimized serial decode logic. Note that this logic
// assumes that huffBuff contains +2 bytes at the end
// of the buffer to account for read ahead.

+ (void) decodeBits:(int)numSymbolsToDecode
           bitBuff:(uint8_t*)bitBuff
          bitBuffN:(int)bitBuffN
          outBuffer:(uint8_t*)outBuffer
     bitOffsetTable:(uint32_t*)bitOffsetTable
{
    vector<uint8_t> outVec;
    outVec.reserve(numSymbolsToDecode);
    
    decode(bitBuff, numSymbolsToDecode, outVec);
    // FIXME: how should decode method return the result data?
    // Since size of buffer is know, this module can assume
    // that allocated buffer is large enough to handle known
    // number of symbols.
    memcpy(outBuffer, outVec.data(), numSymbolsToDecode);
}

// Encode symbols by calculating signed byte deltas
// and then converting to zerod deltas which can
// be represented as positive integer values.

+ (NSData*) encodeSignedByteDeltas:(NSData*)data
{
  vector<int8_t> inBytes;
  inBytes.resize(data.length);
  memcpy(inBytes.data(), data.bytes, data.length);
  
  vector<int8_t> outSignedDeltaBytes = encodeDelta(inBytes);
    
  NSMutableData *outZerodDeltaBytes = [NSMutableData data];
  [outZerodDeltaBytes setLength:outSignedDeltaBytes.size()];
  uint8_t *outZerodDeltaPtr = (uint8_t *) outZerodDeltaBytes.mutableBytes;
    
  // Convert signed delta to zerod (unsigned) deltas
  const int maxNumBytes = (int) outSignedDeltaBytes.size();

  for (int i = 0; i < maxNumBytes; i++) {
      int8_t sVal = outSignedDeltaBytes[i];
      uint8_t zerodVal = pixelpack_int8_to_offset_uint8(sVal);
      *outZerodDeltaPtr++ = zerodVal;
  }

  return [NSData dataWithData:outZerodDeltaBytes];
}

// Decode symbols by reversing zerod mapping and then applying
// signed 8 bit deltas to recover the original symbols as uint8_t.

+ (NSData*) decodeSignedByteDeltas:(NSData*)deltas
{
  const int maxNumBytes = (int) deltas.length;

  vector<uint8_t> signedDeltaBytes;
  signedDeltaBytes.resize(maxNumBytes);
  const uint8_t *zerodDeltasPtr = (uint8_t *) deltas.bytes;
  
  for (int i = 0; i < maxNumBytes; i++) {
    uint8_t zerodVal = zerodDeltasPtr[i];
    int8_t sVal = pixelpack_offset_uint8_to_int8(zerodVal);
    signedDeltaBytes[i] = (uint8_t) sVal;
  }

  // Apply signed deltas
  vector<uint8_t> outSymbols = decodeDelta(signedDeltaBytes);
    
  NSMutableData *mData = [NSMutableData data];
  [mData setLength:maxNumBytes];
  memcpy((void*)mData.mutableBytes, (void*)outSymbols.data(), maxNumBytes);
    
  return [NSData dataWithData:mData];
}

@end

