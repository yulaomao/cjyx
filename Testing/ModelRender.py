import Cjyx
import time
import random


def newSphere(name=''):
    if name == "":
        name = "sphere-%g" % time.time()

    sphere = Cjyx.cjyx.vtkSphereSource()
    sphere.SetCenter(-100 + 200 * random.random(), -100 + 200 * random.random(), -100 + 200 * random.random())
    sphere.SetRadius(10 + 20 * random.random())
    sphere.GetOutput().Update()
    modelDisplayNode = Cjyx.cjyx.vtkDMMLModelDisplayNode()
    modelDisplayNode.SetColor(random.random(), random.random(), random.random())
    Cjyx.cjyx.DMMLScene.AddNode(modelDisplayNode)
    modelNode = Cjyx.cjyx.vtkDMMLModelNode()
# VTK6 TODO
    modelNode.SetAndObservePolyData(sphere.GetOutput())
    modelNode.SetAndObserveDisplayNodeID(modelDisplayNode.GetID())
    modelNode.SetName(name)
    Cjyx.cjyx.DMMLScene.AddNode(modelNode)


def sphereMovie(dir="."):

    for i in range(20):
        newSphere()
        Cjyx.TkCall("update")
        Cjyx.TkCall("CjyxSaveLargeImage %s/spheres-%d.png 3" % (dir, i))
