#!/usr/bin/python

import re

# Filter for inode check
def extract_filesys_inodes(str):
    canonical = []
    for line in str.split('\n'):
        match = re.match('Inode (?P<inode>[0-9]*) ([^ ]* ){5}(?P<chksum>[^ ]*)', line)
        if match:
            canonical.append(match)
    sorted_pairs = sorted(canonical, key=lambda line: int(line.group('inode')))
    return '\n'.join([pair.expand('Inode \g<inode> checksum \g<chksum>') for pair in sorted_pairs])

# Filter for path check
def extract_filesys_paths(str):
    canonical = []
    for line in str.split('\n'):
        match = re.match('Path [^ ]* (?P<path>[^ ]*) ([^ ]* ){5}(?P<chksum>[^ ]*)', line)
        if match:
            canonical.append(match.expand('Path \g<path> checksum \g<chksum>'))
    return '\n'.join(sorted(canonical))

class LongOutputDiffSoln(OutputDiffSoln):

    # If mismatched output is extremely long, abbreviate it
    def score(self, student, soln, context):
        original_result = OutputDiffSoln.score(self, student, soln, context)
        if isinstance(original_result, results.Correct):
            return original_result
        return results.LongMismatchOutput(score=original_result.score, output=original_result.output,
            correct_output=original_result.correct_output)


import ui
import testing

class MustContainTest(BaseTest):

    # Output must contain "mustcontain" field.
    def score(self, student_ex, context):
        if self.mustcontain in student_ex.output:
            return results.Correct(short="Submission output contains \'" + self.mustcontain + "\'", score=self.totalpts)
        else:
            if context == testing.FOR_SANITY:
                print ui.red("ERROR: test file output reported error(s)")
            return results.MismatchOutput(score=0, output=student_ex.output, correct_output=self.mustcontain)
