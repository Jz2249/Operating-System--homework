#!/usr/bin/python

import ui
import testing

class MustContainTest(BaseTest):

    # Output must contain "mustcontain" field.
    def score(self, student_ex, context):
        if self.mustcontain in student_ex.output:
            return results.Correct(short="Submission output contains \'" + self.mustcontain + "\'", matched_output=student_ex.output, score=self.totalpts)
        elif "resource temporarily unavailable" in student_ex.output:
            print ui.red("ERROR: resource temporarily unavailable")
            return results.ResourceTemporarilyUnavailable()
        else:
            if context == testing.FOR_SANITY:
                print ui.red("ERROR: test file output reported error(s)")
            return results.MismatchOutput(score=0, output=student_ex.output, correct_output=self.mustcontain)
