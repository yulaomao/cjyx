import cjyx
import os


def testDMMLSceneImportAndExport():
    tempDir = cjyx.app.temporaryPath
    scenePath = tempDir + '/temp_scene.dmml'
    cjyx.dmmlScene.SetURL(scenePath)
    if not cjyx.dmmlScene.Commit(scenePath):
        raise Exception('Saving a DMML scene failed !')

    success = cjyx.dmmlScene.Import()
    os.remove(scenePath)
    if not success:
        raise Exception('Importing back a DMML scene failed !')


if __name__ == '__main__':
    testDMMLSceneImportAndExport()
