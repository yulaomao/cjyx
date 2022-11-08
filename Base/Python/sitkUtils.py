import SimpleITK as sitk

import cjyx

__sitk__DMMLIDImageIO_Registered__ = False


def PushVolumeToCjyx(sitkimage, targetNode=None, name=None, className='vtkDMMLScalarVolumeNode'):
    """ Given a SimpleITK image, push it back to cjyx for viewing

    :param targetNode: Target node that will store the image. If None then a new node will be created.
    :param className: if a new target node is created then this parameter determines node class. For label volumes, set it to vtkDMMLLabelMapVolumeNode.
    :param name: if a new target node is created then this parameter will be used as basis of node name.
      If an existing node is specified as targetNode then this value will not be used.
    """

    EnsureRegistration()

    # Create new node if needed
    if not targetNode:
        targetNode = cjyx.dmmlScene.AddNewNodeByClass(className, cjyx.dmmlScene.GetUniqueNameByString(name))
        targetNode.CreateDefaultDisplayNodes()

    myNodeFullITKAddress = GetCjyxITKReadWriteAddress(targetNode)
    sitk.WriteImage(sitkimage, myNodeFullITKAddress)

    return targetNode


def PullVolumeFromCjyx(nodeObjectOrName):
    """ Given a cjyx DMML image node or name, return the SimpleITK
        image object.
    """
    EnsureRegistration()
    myNodeFullITKAddress = GetCjyxITKReadWriteAddress(nodeObjectOrName)
    sitkimage = sitk.ReadImage(myNodeFullITKAddress)
    return sitkimage


def GetCjyxITKReadWriteAddress(nodeObjectOrName):
    """ This function will return the ITK FileIO formatted text address
            so that the image can be read directly from the DMML scene
    """
    myNode = nodeObjectOrName if isinstance(nodeObjectOrName, cjyx.vtkDMMLNode) else cjyx.util.getNode(nodeObjectOrName)
    myNodeSceneAddress = myNode.GetScene().GetAddressAsString("").replace('Addr=', '')
    myNodeSceneID = myNode.GetID()
    myNodeFullITKAddress = 'cjyx:' + myNodeSceneAddress + '#' + myNodeSceneID
    return myNodeFullITKAddress


def EnsureRegistration():
    """Make sure DMMLIDImageIO reader is registered.
    """
    if 'DMMLIDImageIO' in sitk.ImageFileReader().GetRegisteredImageIOs():
        # already registered
        return

    # Probably this hack is not needed anymore, but it would require some work to verify this,
    # so for now just leave this here:
    # This is a complete hack, but attempting to read a dummy file with AddArchetypeVolume
    # has a side effect of registering the DMMLIDImageIO file reader.
    global __sitk__DMMLIDImageIO_Registered__
    if __sitk__DMMLIDImageIO_Registered__:
        return
    vl = cjyx.modules.volumes.logic()
    volumeNode = vl.AddArchetypeVolume('_DUMMY_DOES_NOT_EXIST__', 'invalidRead')
    __sitk__DMMLIDImageIO_Registered__ = True
