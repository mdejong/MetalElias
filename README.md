# MetalElias

A GPU Elias gamma decoder for iOS on top of Metal, adapted from Basic Texturing example provided by Apple. This decoder is known to work on iOS and should work on other Metal capable hardware.

## Overview

This project is adapted from a Metal huffman implementation. See [MetalHuffman] https://github.com/mdejong/MetalHuffman

## Status

Not completed currently.

## Decoding Speed

## Implementation

See AAPLRenderer.m and AAPLShaders.metal for the core GPU rendering logic. A table based huffman encoder and decoder are also included.

