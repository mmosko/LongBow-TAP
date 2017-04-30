/*
 * Copyright (c) 2017 Palo Alto Research Center.
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Based on the TextPlain implementation.
 *
 * Conforms to the TestAnythingProtocol display format.
 *
 * Based on the TAP version 13 specification for a Producer
 * http://testanything.org/tap-version-13-specification.html
 */
#include <config.h>

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <LongBow/Reporting/longBowReport_Testing.h>
#include <LongBow/private/longBow_String.h>

// We need to track how may unit tests are executed
static unsigned _currentTestNumber = 0;

static const char *TAP_OK = "ok";
static const char *TAP_NOTOK = "not ok";
static const char *TAP_SKIP = "# skip ";
//static const char *TAP_NOTSKIP = "-";
static const char *TAP_NOTSKIP = "";
static const char *TAP_BAIL = "Bail out!";

// The original stdout FILE*
static FILE *real_stdout = NULL;

// A read file to the freopen'd stdout FIFO
static FILE *read_stdout = NULL;

// A write file to the new stdout FIFO
static FILE *write_stdout = NULL;

// A unique name to a mknod FIFO
static char *fifo_name = NULL;


void
_printTapHeader(void)
{
	fprintf(real_stdout, "TAP version 13\n");
}

void
_printTapPlan(size_t count)
{
	fprintf(real_stdout, "1..%u\n", (unsigned) count);
}

static const LongBowTestRunner *
_testRunnerSilent(const LongBowTestRunner *testRunner)
{
    LongBowStatus status = longBowTestRunner_GetStatus(testRunner);

    printf("# %s %s\n", longBowTestRunner_GetName(testRunner), longBowStatus_ToString(status));
    return testRunner;
}

static const LongBowTestRunner *
_testRunnerDetail(const LongBowTestRunner *testRunner)
{
    size_t nFixtures = longBowTestRunner_GetFixtureCount(testRunner);

    fprintf(real_stdout, "\n#\n");
    fprintf(real_stdout, "# %s: %zd fixture%s\n", longBowTestRunner_GetName(testRunner), nFixtures, (nFixtures == 1 ? "" : "s"));

    for (size_t i = 0; i < nFixtures; i++) {
        LongBowTestFixture *fixture = longBowTestRunner_GetFixture(testRunner, i);
        longBowReportTesting_TestFixture(fixture);
    }

    return testRunner;
}

void
longBowReportTesting_Initialize(void)
{
	// Capture stdout so we can display it in the YAML block of each test case

	// Hide away a reference to the actual console output
	int original_stdout = dup(STDOUT_FILENO);
	real_stdout = fdopen(original_stdout, "w");

	// Now point stdout at something new
	fifo_name = tempnam(NULL, "longbow_");

	mknod(fifo_name, S_IFIFO|0666, 0);

	// The fifo would block, so open it for reading non-blocking
	int readfd = open(fifo_name, O_RDONLY | O_NONBLOCK);
	if (readfd == -1) {
		perror("Error opening fifo for reading ");
		exit(EXIT_FAILURE);
	}

	read_stdout = fdopen(readfd, "r");
	if (read_stdout == NULL) {
		perror("Error fdopen() on fifo ");
		exit(EXIT_FAILURE);
	}

	write_stdout = freopen(fifo_name, "w", stdout);
	if (write_stdout == NULL) {
		perror("Error freopen() on stdout ");
		exit(EXIT_FAILURE);
	}

	read_stdout = fopen(fifo_name, "r");

	_printTapHeader();
}

void
longBowReportTesting_Finalize(void)
{
	_printTapPlan(_currentTestNumber);

	fclose(write_stdout);
	pclose(read_stdout);

	// Nothing to do for this reporter
	if (fifo_name != NULL) {
		unlink(fifo_name);
		free(fifo_name);
		fifo_name = NULL;
	}
}

const LongBowTestRunner *
longBowReportTesting_TestRunner(const LongBowTestRunner *testRunner)
{
    if (longBowConfig_GetBoolean(longBowTestRunner_GetConfiguration(testRunner), false, "silent")) {
        return _testRunnerSilent(testRunner);
    } else {
        return _testRunnerDetail(testRunner);
    }
}

static unsigned int
_totalSucceeded(const LongBowTestFixtureSummary *summary)
{
    return summary->totalSucceeded + summary->totalWarned + summary->totalTearDownWarned;
}

static unsigned int
_totalWarned(const LongBowTestFixtureSummary *summary)
{
    return summary->totalWarned + summary->totalTearDownWarned;
}

static unsigned int
_totalFailed(const LongBowTestFixtureSummary *summary)
{
    return summary->totalFailed + summary->totalSignalled + summary->totalStopped + summary->totalTearDownFailed;
}

static unsigned int
_totalIncomplete(const LongBowTestFixtureSummary *summary)
{
    return summary->totalSetupFailed + summary->totalSkipped + summary->totalUnimplemented;
}

static void
_reportSummary(const LongBowTestFixture *testFixture)
{
    const LongBowTestFixtureSummary *summary = longBowTestFixture_GetSummary(testFixture);

    char *fixtureString = longBowTestFixture_ToString(testFixture);

    fprintf(real_stdout, "# %s: Ran %u test case%s.", fixtureString, summary->totalTested, summary->totalTested == 1 ? "" : "s");
    free(fixtureString);

    if (summary->totalTested > 0) {
        fprintf(real_stdout, " %d%% (%d) succeeded", _totalSucceeded(summary) * 100 / summary->totalTested, _totalSucceeded(summary));

        if (_totalWarned(summary) > 0) {
            fprintf(real_stdout, " %d%% (%d) with warnings", _totalWarned(summary) * 100 / _totalSucceeded(summary), _totalWarned(summary));
        }
        if (_totalFailed(summary) != 0) {
            fprintf(real_stdout, ", %d%% (%d) failed", _totalFailed(summary) * 100 / summary->totalTested, _totalFailed(summary));
        }
        if (_totalIncomplete(summary) > 0) {
            fprintf(real_stdout, ", %d%% (%d) incomplete", _totalIncomplete(summary) * 100 / summary->totalTested, _totalIncomplete(summary));
        }
    }
    fprintf(real_stdout, "\n");
}

const LongBowTestFixture *
longBowReportTesting_TestFixture(const LongBowTestFixture *testFixture)
{
    size_t nTestCases = longBowTestFixture_GetTestCaseCount(testFixture);

    _reportSummary(testFixture);

//    for (size_t i = 0; i < nTestCases; i++) {
//        LongBowTestCase *testCase = longBowTestFixture_GetTestCase(testFixture, i);
//        longBowReportTesting_TestCase(testCase);
//    }
    return testFixture;
}

const LongBowTestCase *
longBowReportTesting_TestCase(const LongBowTestCase *testCase)
{
    LongBowRuntimeResult *testCaseResult = longBowTestCase_GetActualResult(testCase);

    char *rusageString = longBowReportRuntime_RUsageToString(longBowRuntimeResult_GetRUsage(testCaseResult));

    char *elapsedTimeString = longBowReportRuntime_TimevalToString(longBowRuntimeResult_GetElapsedTime(testCaseResult));

    char *statusString = longBowStatus_ToString(longBowTestCase_GetActualResult(testCase)->status);
    char *testCaseString = longBowTestCase_ToString(testCase);

    LongBowString *string = longBowString_CreateFormat("# %10s %s %s %zd %s\n",
                                                       testCaseString,
                                                       elapsedTimeString,
                                                       rusageString,
                                                       longBowRuntimeResult_GetEventEvaluationCount(longBowTestCase_GetActualResult(testCase)),
                                                       statusString);
    longBowString_Write(string, real_stdout);
    longBowString_Destroy(&string);

    free(testCaseString);
    free(statusString);
    free(elapsedTimeString);
    free(rusageString);

    return testCase;
}

static LongBowString *
_createStatusString(const char *tap_ok, unsigned testNumber, const char *skipString,
		const char *testCaseString,
		const char *elapsedTimeString, const char *rusageString, size_t evaluationCount,
		const char *statusString)
{
    LongBowString *string = longBowString_CreateFormat("%s %u %s%10s %s %s %zd %s\n",
    							tap_ok,
								testNumber,
								skipString,
								testCaseString,
								elapsedTimeString,
								rusageString,
								evaluationCount,
								statusString);
    return string;
}

static void
_startYaml(void)
{
	fprintf(real_stdout, "  ---\n");
	fprintf(real_stdout, "  stdout:\n");
}

static void
_stopYaml(void)
{
	fprintf(real_stdout, "  ...\n");
}

/**
 * Flush stdout, then read it to end writing the results to
 * the original stdout.
 */
static void
_redirectStdout(void)
{
	const size_t bufferSize = 4096;
	char buffer[bufferSize];
	const char *marker = "# $^%&@ You should never see this\n";

	bool needStartYaml = true;
	bool needStopYaml = false;

	// Write a marker to stdout and read until we get the marker back
	printf("%s", marker);
	fflush(stdout);

	bool finished = false;
	while ( !feof(read_stdout) && !finished) {
		if (fgets(buffer, bufferSize, read_stdout) != NULL) {
			if (strcmp(marker, buffer) != 0) {
				if (needStartYaml) {
					_startYaml();
					needStartYaml = false;
					needStopYaml = true;
				}

				fprintf(real_stdout, "    %s", buffer);
			} else {
				finished = true;
			}
		} else if (ferror(read_stdout)) {
			perror("Error from fgets() on FIFO ");
		} else {
			// an EOF
		}
	}

	if (needStopYaml) {
		_stopYaml();
	}
}

void
longBowReportTesting_DisplayTestCaseResult(const LongBowTestCase *testCase)
{
	_currentTestNumber++;

    LongBowRuntimeResult *testCaseResult = longBowTestCase_GetActualResult(testCase);
    char *rusageString = longBowReportRuntime_RUsageToString(longBowRuntimeResult_GetRUsage(testCaseResult));

    char *elapsedTimeString = longBowReportRuntime_TimevalToString(longBowRuntimeResult_GetElapsedTime(testCaseResult));

    char *statusString = longBowStatus_ToString(longBowTestCase_GetActualResult(testCase)->status);
    char *testCaseString = longBowTestCase_ToString(testCase);

    size_t evaluationCount = longBowRuntimeResult_GetEventEvaluationCount(longBowTestCase_GetActualResult(testCase));
    LongBowString *string;

    switch (testCaseResult->status) {
    	case LongBowStatus_UNIMPLEMENTED:
    		// fall through
    	case LongBowStatus_IMPOTENT:
    		// fall through
    	case LongBowStatus_WARNED:
    		// fall through
        case LongBowStatus_TEARDOWN_WARNED:
    		// fall through
    	case LONGBOW_STATUS_SUCCEEDED:
    		// fall through
        case LongBowStatus_UNTESTED:
            string = _createStatusString(
						TAP_OK,
						_currentTestNumber,
						TAP_NOTSKIP,
						testCaseString,
						elapsedTimeString,
						rusageString,
						evaluationCount,
						statusString);
        	break;

        case LONGBOW_STATUS_SKIPPED:
            string = _createStatusString(
						TAP_OK,
						_currentTestNumber,
						TAP_SKIP,
						testCaseString,
						elapsedTimeString,
						rusageString,
						evaluationCount,
						statusString);
            break;

        case LONGBOW_STATUS_FAILED:
    		// fall through
        case LONGBOW_STATUS_SETUP_FAILED:
    		// fall through
        case LONGBOW_STATUS_TEARDOWN_FAILED:
            string = _createStatusString(
						TAP_NOTOK,
						_currentTestNumber,
						TAP_NOTSKIP,
						testCaseString,
						elapsedTimeString,
						rusageString,
						evaluationCount,
						statusString);
        	break;

        case LongBowStatus_STOPPED:
            string = _createStatusString(
						TAP_BAIL,
						_currentTestNumber,
						TAP_NOTSKIP,
						testCaseString,
						elapsedTimeString,
						rusageString,
						evaluationCount,
						statusString);
            break;

        default:
            if (testCaseResult->status >= LongBowStatus_SIGNALLED) {
                string = _createStatusString(
    						TAP_BAIL,
    						_currentTestNumber,
    						"- SIGNALED",
    						testCaseString,
    						elapsedTimeString,
    						rusageString,
    						evaluationCount,
    						statusString);
            } else {
                string = _createStatusString(
    						TAP_BAIL,
    						_currentTestNumber,
    						"- UNKNOWN",
    						testCaseString,
    						elapsedTimeString,
    						rusageString,
    						evaluationCount,
    						statusString);
            }
    }

    longBowString_Write(string, real_stdout);
    longBowString_Destroy(&string);

    _redirectStdout();

    fprintf(real_stdout,"\n");
    fflush(real_stdout);

    free(testCaseString);
    free(statusString);
    free(elapsedTimeString);
    free(rusageString);
}

void
longBowReportTesting_Trace(const char *restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *message;
    if (vasprintf(&message, format, ap) == -1) {
        return;
    }
    va_end(ap);

    fprintf(real_stdout, "# %s\n", message);
    free(message);
}
