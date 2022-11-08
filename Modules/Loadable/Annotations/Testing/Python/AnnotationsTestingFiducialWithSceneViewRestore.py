# Test that setting a fiducial coordinate is consistent across scene view saves and restores

import cjyx


fid = cjyx.vtkDMMLAnnotationFiducialNode()
fid.SetScene(cjyx.dmmlScene)
fid.CreateAnnotationTextDisplayNode()
fid.CreateAnnotationPointDisplayNode()

startCoords = [1.0, 2.0, 3.0]
fid.AddControlPoint(startCoords, 0, 1)

cjyx.dmmlScene.AddNode(fid)
fid.GetFiducialCoordinates(startCoords)
print("Starting fiducial coordinates = ", startCoords)

sv = cjyx.dmmlScene.AddNode(cjyx.vtkDMMLSceneViewNode())

sv.StoreScene()

fid.SetFiducialCoordinates(11.1, 22.2, 33.3)
afterStoreSceneCoords = [0, 0, 0]
fid.GetFiducialCoordinates(afterStoreSceneCoords)
print("After storing the scene, set fiducial coords to ", afterStoreSceneCoords)

sv.RestoreScene()

fidAfterRestore = cjyx.dmmlScene.GetNodeByID("vtkDMMLAnnotationFiducialNode1")

coords = [0, 0, 0]
fidAfterRestore.GetFiducialCoordinates(coords)
print("After restoring the scene, fiducial coordinates = ", coords)

xdiff = coords[0] - startCoords[0]
ydiff = coords[1] - startCoords[1]
zdiff = coords[2] - startCoords[2]

print("Difference between coordinates after restored the scene and value from just before stored the scene: ", xdiff, ydiff, zdiff)

diffTotal = xdiff + ydiff + zdiff

if diffTotal > 0.1:
    exceptionMessage = "Difference between coordinate values total = " + str(diffTotal)
    raise Exception(exceptionMessage)
