//
//  elias.hpp
//
//  Created by Mo DeJong on 6/3/18.
//  Copyright © 2018 helpurock. All rights reserved.
//
//  The elias coder provides elias gamma encoding for
//  residual values that are non-zero and near zero.

#ifndef elias_hpp
#define elias_hpp

#include <stdio.h>
#include <assert.h>

#include <cinttypes>
#include <vector>
#include <bitset>

using namespace std;

class EliasGammaEncoder
{
public:
    bitset<8> bits;
    unsigned int bitOffset;
    vector<uint8_t> bytes;
    unsigned int numEncodedBits;
    
    EliasGammaEncoder()
    : bitOffset(0), numEncodedBits(0) {
        bytes.reserve(1024);
    }
    
    void reset() {
        bits.reset();
        bitOffset = 0;
        bytes.clear();
        numEncodedBits = 0;
    }
    
    void encodeBit(bool bit) {
        const bool debug = false;
        
        bits.set(bitOffset++, bit);
        
        if (bitOffset == 8) {
            uint8_t byteVal = 0;
            
            // Flush 8 bits to some external output
            
            for ( int i = 0; i < 8; i++ ) {
                unsigned int v = (bits.test(i) ? 0x1 : 0x0);
                byteVal |= (v << i);
            }
            
            bits.reset();
            bitOffset = 0;
            
            if (debug) {
                printf("encodeBit() emit byte 0x%02X\n", byteVal);
            }
            
            bytes.push_back(byteVal);
            numEncodedBits += 8;
        }
    }
    
    // Find the highest bit position that is on, return -1 when no on bit is found.
    // Note that this method cannot process the value zero and it supports the
    // range (1, 256) which corresponds to bit positions (0, 8) or 9 bits max,
    
    int highBitPosition(uint32_t number) {
        int highBitValue = -1;
        
        // The maximum value that is acceptable is 256 or 2^8
        // which falls outside the first byte.
        
        // FIXME: count leading zeros on 16 bit value ?
        // int __builtin_clz
        // Returns the number of leading 0-bits in x,
        // starting at the most significant bit position.
        // If x is 0, the result is undefined.
        
        for (int i = 0; i < 9; i++) {
            if ((number >> i) & 0x1) {
                highBitValue = i;
            }
        }
        
#if defined(DEBUG)
        // In DEBUG mode, bits contains bits for this specific symbol.
        assert(highBitValue != -1);
#endif // DEBUG

        return highBitValue;
    }
    
    // Encode unsigned byte range number (0, 255) with an
    // elias gamma encoding that implicitly adds 1 before encoding.
    
    void encode(uint8_t inByteNumber)
    {
        const bool debug = false;
        
#if defined(DEBUG)
        // In DEBUG mode, bits contains bits for this specific symbol.
        vector<bool> bitsThisSymbol;
#endif // DEBUG
        
        // The input value range is (0, 255) corresponding to (1, 256)
        // but since 0 is unused and 256 cannot be represented as uint8_t
        // always implicitly add 1 before encoding.
        
        uint32_t number = inByteNumber;
        number += 1;
        
        // highBitValue is set to highest POT (bit that is on) in unsigned number n
        // Encode highBitValue in unary; that is, as N zeroes followed by a one.
        
        int highBitValue = highBitPosition(number);
        
        if (debug) {
            printf("for n %3d : high bit value is 0x%02X aka %d\n", number, highBitValue, (0x1 << highBitValue));
        }
        
        // Emit highBitValue number of zero bits (unary)
        
        for (int i = 0; i < highBitValue; i++) {
          encodeBit(false);
#if defined(DEBUG)
          bitsThisSymbol.push_back(false);
#endif // DEBUG
        }
        
        encodeBit(true);
#if defined(DEBUG)
        bitsThisSymbol.push_back(true);
#endif // DEBUG
        
        // Emit the remaninig bits of the number n
        
        for (int i = highBitValue - 1; i >= 0; i--) {
          bool bit = (((number >> i) & 0x1) != 0);
          encodeBit(bit);
#if defined(DEBUG)
          bitsThisSymbol.push_back(bit);
#endif // DEBUG
        }
        
        if (debug) {
#if defined(DEBUG)
            // Print bits that were emitted for this symbol,
            // note the order from least to most significant
            printf("bits for symbol (least -> most): ");
            
            for ( bool bit : bitsThisSymbol ) {
                printf("%d", bit ? 1 : 0);
            }
            printf("\n");
#endif // DEBUG
        }        
    }
    
    // If any bits still need to be emitted, emit final byte.
    
    void finish() {
        const bool debug = false;
        
        if (bitOffset > 0) {
            // Flush 1-8 bits to some external output.
            // Note that all remaining bits must
            // be flushed as true so that multiple
            // symbols are not encoded at the end
            // of the buffer.
            
            numEncodedBits += bitOffset;
            
            // Emit zeros up until the end of a byte, so
            // the decoding logic will skip zeros until
            // the end of the stream and exit loop.
            
            while (bitOffset < 8) {
                bits.set(bitOffset++, false);
            }
            
            uint8_t byteVal = 0;
            
            for ( int i = 0; i < 8; i++ ) {
                unsigned int v = (bits.test(i) ? 0x1 : 0x0);
                byteVal |= (v << i);
                
                if (debug) {
                    printf("finish() bit %d : byteVal 0x%02X\n", i, byteVal);
                }
            }
            
            if (debug) {
                printf("finish() emit byte 0x%02X\n", byteVal);
            }
            
            bits.reset();
            bitOffset = 0;
            bytes.push_back(byteVal);
        }
    }
    
    // Encode N symbols and emit any leftover bits
    
    void encode(const uint8_t * byteVals, int numByteVals) {
        for (int i = 0; i < numByteVals; i++) {
            uint8_t byteVal = byteVals[i];
            encode(byteVal);
        }
        finish();
    }
    
    // Query number of bits needed to store symbol
    // with the given k parameter. Note that this
    // size query logic does not need to actually copy
    // encoded bytes so it is much faster than encoding.
    
    int numBits(uint8_t inByteNumber) {
        uint32_t number = inByteNumber;
        number += 1;
#if defined(DEBUG)
        assert(number >= 1 && number <= 256);
#endif // DEBUG
        int highBitValue = highBitPosition(number);
        int lowBits = highBitValue;
        int totalBits = highBitValue + 1 + lowBits;
        return totalBits;
    }
    
    // Query the number of bits needed to store these symbols
    
    int numBits(const uint8_t * byteVals, int numByteVals) {
        int numBitsTotal = 0;
        for (int i = 0; i < numByteVals; i++) {
            uint8_t byteVal = byteVals[i];
            numBitsTotal += numBits(byteVal);
        }
        return numBitsTotal;
    }
    
};

class EliasGammaDecoder
{
public:
    bitset<8> bits;
    unsigned int bitOffset;
    vector<uint8_t> bytes;
    unsigned int byteOffset;
    unsigned int numDecodedBits;
    bool isFinishedReading;
    
    EliasGammaDecoder()
    {
        reset();
    }
    
    void reset() {
        numDecodedBits = 0;
        byteOffset = 0;
        bitOffset = 8;
        isFinishedReading = false;
    }
    
    bool decodeBit() {
        const bool debug = false;
        
        if (debug) {
            printf("decodeBit() bitOffset %d\n", bitOffset);
        }
        
        if (bitOffset == 8) {
            if (byteOffset == bytes.size()) {
                // All bytes read and all bits read
                isFinishedReading = true;
                return true;
            }
            
            bits.reset();
            
            uint8_t byteVal = bytes[byteOffset++];
            for ( int i = 0; i < 8; i++ ) {
                bool bit = ((byteVal >> i) & 0x1) ? true : false;
                bits.set(i, bit);
            }
            
            bitOffset = 0;
        }
        
        bool bit = bits.test(bitOffset++);
        
        if (debug) {
            printf("decodeBit() returning %d\n", bit);
        }
        
        return bit;
    }
    
    // Decode symbols from a buffer of encoded bytes and
    // return the results as a vector of decoded bytes.
    
    vector<uint8_t> decode(const vector<uint8_t> & inBytesVec) {
        reset();
        
        const bool debug = false;
        
        vector<uint8_t> decodedBytes;
        
        assert(inBytesVec.size() > 0);
        bytes.resize(inBytesVec.size());
        memcpy(bytes.data(), inBytesVec.data(), inBytesVec.size());

        for ( ; 1 ; ) {
            unsigned int countOfZeros = 0;
            
            while (decodeBit() == false) {
                countOfZeros++;
            }
            unsigned int symbol = (0x1 << countOfZeros);
            
            if (debug) {
                printf("symbol base : 2 ^ %d : %d\n", countOfZeros, symbol);
            }
            
            for ( int i = countOfZeros - 1; i >= 0; i-- ) {
                bool b = decodeBit();
                symbol |= ((b ? 1 : 0) << i);
            }

            if (isFinishedReading) {
                break;
            }
            
            if (debug) {
                printf("append decoded symbol = %d\n", symbol);
            }
            
#if defined(DEBUG)
            assert(symbol >= 1 && symbol <= 256);
#endif // DEBUG
            
            decodedBytes.push_back(symbol - 1);
            
            int highBitValue = countOfZeros;
            int lowBits = highBitValue;
            int totalBits = highBitValue + 1 + lowBits;
            numDecodedBits += totalBits;
        }
        
        return decodedBytes;
    }
    
};

#endif // elias_hpp
