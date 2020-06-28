#!/usr/bin/env python3
import struct

"""
This program creates an isam index file for hexagrams, producing 64 index pairs like:
(value_offset, id_offset)
where value_offset gives the position of the hexagram record whose lines (from bottom to top)
add up to a binary value for lookup. id_offset is the offset to the hexagram of the
corresponding index record number plus one. (ID is radix 1, value is radix 0.)

A file named hexagrams.idx is created or overwritten, and four bytes are written per offset,
(little-endian) for a total of 64 X 2 X 4 bytes (two offsets per hexagram, 4 bytes each) = 512 bytes.

Run this program whenever the isam file is modified to update the index.

The hexagram records in hexagrams.isam are each 13 tab-separated strings:
0       value       Sum of binary values of lines (1 for solid, 0 for broken, bottom = lsb)
1       id          Hexagram number in the Book of Changes
2       name        English translation of hexagram name
3       title       Romanized Chinese name of hexagram
4       judgement   The Judgement, with encoded newlines
5       image       The Image, with embedded newlines
6 - 11  lines       Text for changing lines, from bottom up, with embedded newlines
12      all         For hexagrams 1 and 2, when all lines are changing; else empty
"""

by_bin  = {}
by_id   = {}

# Read the isam file and fill the two dictionaries with offsets
# Also, keep track of longest record
data = open("hexagrams.isam", "r")
line_no       = 0
longest       = 0
while True:
  position    = data.tell()       # get the current position
  line        = data.readline()
  length      = len(line)
  if 0       == length: break
  if length > longest: longest = length
  line_no    += 1
  fields      = line.split("\t")
  if 13 != len(fields): print("line %d has %d fields" % (line_no, len(fields)))
  bin         = int(fields[0])
  id          = int(fields[1])
  by_bin[bin] = position
  by_id[id]   = position
data.close()
print("Longest data record: %d" % (longest))

# Create the idx file and write offsets from the two dictionaries
index = open("hexagrams.idx", "w+b")
for i in range(0, 64):
  index.write(struct.pack('i', by_bin[i]))    # offset to binary value hexagram
  index.write(struct.pack('i', by_id[i+1]))   # offset to hexagram number
index.close()
print("hexagrams.idx written")
