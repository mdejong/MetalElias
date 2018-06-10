// Objective C interface to huffman parsing functions
//  MIT Licensed

#import <Foundation/Foundation.h>

#import "VariableBitWidthSymbol.h"

// Our platform independent render class
@interface Eliasg : NSObject

// Given an input buffer, huffman encode the input values and generate
// output that corresponds to

+ (void) encodeBits:(uint8_t*)inBytes
         inNumBytes:(int)inNumBytes
           outCodes:(NSMutableData*)outCodes
 outBlockBitOffsets:(NSMutableData*)outBlockBitOffsets
              width:(int)width
             height:(int)height
           blockDim:(int)blockDim;

// Unoptimized serial decode logic. Note that this logic
// assumes that huffBuff contains +2 bytes at the end
// of the buffer to account for read ahead.

+ (void) decodeBits:(int)numSymbolsToDecode
            bitBuff:(uint8_t*)bitBuff
           bitBuffN:(int)bitBuffN
          outBuffer:(uint8_t*)outBuffer
     bitOffsetTable:(uint32_t*)bitOffsetTable;

// Encode symbols by calculating signed byte deltas
// and then converting to zerod deltas which can
// be represented as positive integer values.

+ (NSData*) encodeSignedByteDeltas:(NSData*)data;

// Decode symbols by reversing zerod mapping and then applying
// signed 8 bit deltas to recover the original symbols as uint8_t.

+ (NSData*) decodeSignedByteDeltas:(NSData*)deltas;

@end
