#! /usr/bin/env python
# Copyright (c) 2017 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#

import sys
import os
import pprint
import subprocess
import difflib
import csv
import argparse
sys.path.append("@INSTALL_PYTHON_DIR@")
sys.path.append("@DEPENDENCY_PYTHON_DIR@")
import LongBow

def concatenateContinuationLines(lines):
    '''
    Parse doxygen log lines.
    Lines that are indented by a space are continutations of the previous line.
    '''
    result = list()
    accumulator = ""
    for line in lines:
        line = line.rstrip()
        if line.startswith(" ") == False and line.startswith(" ") == False:
            if len(accumulator) > 0:
                result.append(accumulator)
            accumulator = line
        else:
            accumulator = accumulator + " " + line.lstrip()

    result.append(accumulator)

    return result

def parseLine(line):
    result = None
    if not line.startswith("<"):
        fields = line.split(":")
        if len(fields) >= 4:
            result = { "fileName" : fields[0].strip(),
                    "lineNumber" : int(fields[1].strip()),
                    "type" : "documentation",
                    "severity" : fields[2].strip(),
                    "message" : " ".join(fields[3:]).strip()}
        elif line.startswith("error"):
	          print line
        elif len(line) > 0:
          print "Consider using doxygen -s:", line

    return result

def canonicalize(lines):
    lines = concatenateContinuationLines(lines)
    parsedLines = map(lambda line: parseLine(line), lines)
    parsedLines = filter(lambda line: line != None, parsedLines)
    return parsedLines

def organize(entries):
    result = dict()

    for entry in entries:
        if not entry["fileName"] in result:
            result[entry["fileName"]] = dict()

        entryByFile = result[entry["fileName"]]

        if not str(entry["lineNumber"]) in entryByFile:
            entryByFile[str(entry["lineNumber"])] = list()
        if not entry in entryByFile[str(entry["lineNumber"])]:
            entryByFile[str(entry["lineNumber"])].append(entry)

    return result

def textualSummary(distribution, documentation):
    maxWidth = 0
    for entry in documentation:
        if len(entry) > maxWidth:
            maxWidth = len(entry)

    formatString ="%-" + str(maxWidth) + "s %8d %8d   %.2f%%"
    for entry in documentation:
        badLines = len(documentation[entry])
        totalLines =  LongBow.countLines(entry)
        score = float(totalLines - badLines) / float(totalLines) * 100.0
        LongBow.scorePrinter(distribution, score, formatString % (entry, totalLines, badLines, score))
    return

def textualAverage(distribution, documentation, format):
    sum = 0.0

    for entry in documentation:
        badLines = len(documentation[entry])
        totalLines =  LongBow.countLines(entry)
        score = float(totalLines - badLines) / float(totalLines) * 100.0
        sum = sum + score

    if len(documentation) == 0:
        averageScore = 100.0
    else:
        averageScore = sum / float(len(documentation))

    LongBow.scorePrinter(distribution, averageScore, format % averageScore)

def csvSummary(distribution, documentation):
        formatString ="documentation,%s,%d,%d,%.2f%%"
        for entry in documentation:
            badLines = len(documentation[entry])
            totalLines =  LongBow.countLines(entry)
            score = float(totalLines - badLines) / float(totalLines) * 100.0
            LongBow.scorePrinter(distribution, score, formatString % (entry, totalLines, badLines, score))
        return

def main(argv):
    parser = argparse.ArgumentParser(prog='longbow-doxygen-report', formatter_class=argparse.RawDescriptionHelpFormatter, description="")
    parser.add_argument('-l', '--doxygenlog', default=False, action="store", required=True, type=str, help="The doxygen output log to use.")
    parser.add_argument('-s', '--summary', default=False, action="store_true", required=False, help="Produce the score for each file")
    parser.add_argument('-a', '--average', default=False, action="store_true", required=False, help="Produce the simple average of all scores.")
    parser.add_argument('-d', '--distribution', default="[100, 95]", action="store", required=False, type=str, help="A list containing the score distributions for pretty-printing")
    parser.add_argument('-o', '--output', default="text", action="store", required=False, type=str, help="The required output format. text, csv")

    args = parser.parse_args()

    if not args.summary and not args.average:
        args.summary = True

    with open(args.doxygenlog, 'r') as f:
        lines = f.readlines()

    lines = canonicalize(lines)

    result = organize(lines)

    pp = pprint.PrettyPrinter(indent=4)
    #pp.pprint(result)

    distribution = eval(args.distribution)
    if args.summary:
        if args.output == "text":
            textualSummary(distribution, result)
        else:
            csvSummary(distribution, result)

    if args.average:
        textualAverage(distribution, result, "%.2f")


if __name__ == '__main__':
    '''
@(#) longbow-doxygen-report @VERSION@ @DATE@
@(#)   All Rights Reserved. Use is subject to license terms.
    '''
    main(sys.argv)
