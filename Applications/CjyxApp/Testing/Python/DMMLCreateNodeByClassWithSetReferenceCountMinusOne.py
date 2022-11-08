import cjyx


def testDMMLCreateNodeByClassWithSetReferenceCountMinusOne():
    n = cjyx.dmmlScene.CreateNodeByClass('vtkDMMLViewNode')
    n.UnRegister(None)  # the node object is now owned by n Python variable therefore we can release the reference that CreateNodeByClass added
    cjyx.dmmlScene.AddNode(n)


if __name__ == '__main__':
    testDMMLCreateNodeByClassWithSetReferenceCountMinusOne()
