import cjyx


def testDMMLCreateNodeByClassWithoutSetReferenceCount():

    # Always run this test as if CTest ran it.
    # It is necessary because without this the test on Windows
    # would report leaks in a popup window that has to be closed manually
    # (or wait a long time for timeout) when run from VisualStudio by
    # building the RUN_TESTS project.
    cjyx.app.setEnvironmentVariable("DASHBOARD_TEST_FROM_CTEST", "1")

    n = cjyx.dmmlScene.CreateNodeByClass('vtkDMMLViewNode')
    cjyx.dmmlScene.AddNode(n)
    # This is expected to leak memory because CreateNodeByClass increments the reference count by one
    # and nothing decrements it.


if __name__ == '__main__':
    testDMMLCreateNodeByClassWithoutSetReferenceCount()
