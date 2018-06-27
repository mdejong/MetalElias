# MetalElias

A GPU based Elias gamma decoder for iOS on top of Metal, adapted from Basic Texturing example provided by Apple. This decoder is known to work on iOS and should work on other Metal capable hardware. 

## Overview

This project is adapted from a Metal huffman implementation. See [MetalHuffman] https://github.com/mdejong/MetalHuffman

## Status

This encoder/decoder implementation GPU based Elias gamma decoding is a test of parellel block based decoding speed.

## Decoding Speed

Please note that current results indicate decoding on the CPU is significantly faster than decoding on the GPU since each decoding step has to wait until the previous one has completed.

## Implementation

See AAPLRenderer.m and AAPLShaders.metal for the core GPU rendering logic. An inlined and branch free elias gamma decoder is included.

